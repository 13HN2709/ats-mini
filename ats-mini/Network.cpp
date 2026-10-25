#include "Common.h"
#include "Storage.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Memories.h"
#include "Draw.h"
#include "Splash.h"
#include "TcpMode.h"
#include "Ota.h"
#include "Patches.h"
#include "PageTemplate.h"
#include "PageCommon.h"
#include "PageStatus.h"
#include "PageMemory.h"
#include "PageConfig.h"
#include "PageUpdate.h"
#include "PagePatches.h"
#include <new>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <time.h>
#include <esp_heap_caps.h>
#include <esp_system.h>

#define CONNECT_TIME  3000  // Time of inactivity to start connecting WiFi
#define WIFI_MULTI_TOTAL_TIMEOUT  30000
#define SPLASH_MAX_FILE_SIZE (512U * 1024U)

#ifndef WIFI_POWER_LEVEL
#define WIFI_POWER_LEVEL WIFI_POWER_17dBm
#endif

WiFiMulti wifiMulti;

//
// Access Point (AP) mode settings
//
static const char *apSSID    = RECEIVER_NAME;
static const char *apPWD     = 0;       // No password
static const int   apChannel = 10;      // WiFi channel number (1..13)
static const bool  apHideMe  = false;   // TRUE: disable SSID broadcast
static const int   apClients = 3;       // Maximum simultaneous connected clients

static bool itIsTimeToWiFi = false; // TRUE: Need to connect to WiFi
static uint32_t connectTime = 0;

// Settings
String loginUsername = "";
String loginPassword = "";
static bool wifiScanHidden = false;

// AsyncWebServer object on port 80
AsyncWebServer server(80);

// NTP Client to get time
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "pool.ntp.org");

static bool wifiInitAP();
static bool wifiConnect();
static void webInit();
static void wifiRegisterPowerLevelCallback();
static void wifiPowerLevelOnEvent(WiFiEvent_t event);

static void webSetConfig(AsyncWebServerRequest *request);
static void webSetMemories(AsyncWebServerRequest *request);
static void webUploadSplash(AsyncWebServerRequest *request, const String &filename,
                            size_t index, uint8_t *data, size_t len, bool final);
static bool webIsAuthenticated(AsyncWebServerRequest *request);
static volatile bool webSpectrumRequested = false;
static volatile bool webSpectrumReady = false;
static volatile bool webSpectrumBusy = false;
static volatile int32_t webTuneFrequencyHz = 0;
static volatile int8_t webTuneRequestedMode = -1;
static volatile int8_t webSpectrumResultMode = -1;
static volatile int16_t webRequestedVolume = -1;
static volatile int8_t webTuneStep = 0;
static volatile uint16_t webSpectrumStartFreq = 0;
static volatile uint16_t webSpectrumEndFreq = 0;
static volatile bool webSpectrumCustomRange = false;
static volatile int8_t webSpectrumRequestedMode = -1;
static volatile uint16_t webSpectrumStep = 1;
static volatile uint32_t webTuneRequestId = 0;
static volatile uint32_t webTuneCompletedId = 0;
static volatile uint32_t webTuneTargetHz = 0;
static volatile int8_t webTuneTargetMode = -1;
static volatile int8_t webTuneResult = 0; // 0 pending, 1 confirmed, 2 frequency mismatch
static volatile uint32_t webTuneActualHz = 0;
static volatile int8_t webTuneActualMode = -1;
volatile uint8_t webTunePhase = 0; // Tune diagnostics: 0 idle, 1 queued, 2 processing, 3 switching band, 4 configuring receiver, 5 reading receiver, 6 identifying station, 7 complete; 8-12 selectBand stages
static void webUploadFirmwareComplete(AsyncWebServerRequest *request);
static void webUpdatePage(AsyncWebServerRequest *request, const OtaStatus &status = otaStatus(), int code = 0);
static void webUploadFirmware(AsyncWebServerRequest *request, const String &filename,
                              size_t index, uint8_t *data, size_t len, bool final);
static void webPatchesPage(AsyncWebServerRequest *request);
static void webPatchesComplete(AsyncWebServerRequest *request);
static void webUploadPatch(AsyncWebServerRequest *request, const String &filename,
                           size_t index, uint8_t *data, size_t len, bool final);
static bool webParseUTCDateTime(const String &text, uint32_t *epoch);

static const char *WEB_DIAG_PATH = "/web-diagnostics.log";
void webDiagLog(const char *event)
{
  if(!event || !webDiagEnabled) return;
  if(LittleFS.exists(WEB_DIAG_PATH)) { File oldFile = LittleFS.open(WEB_DIAG_PATH, "r"); size_t oldSize = oldFile ? oldFile.size() : 0; if(oldFile) oldFile.close(); if(oldSize > 49152) LittleFS.remove(WEB_DIAG_PATH); }
  File file = LittleFS.open(WEB_DIAG_PATH, "a");
  if(!file) return;
  file.printf("%lu ms | %s | phase=%u freq=%u mode=%u band=%d heap=%u\n", (unsigned long)millis(), event, (unsigned)webTunePhase, (unsigned)currentFrequency, (unsigned)currentMode, bandIdx, (unsigned)ESP.getFreeHeap());
  file.flush(); file.close();
}


static String webPage(const char *body, const char *title, std::initializer_list<PageValue> values,
                      std::initializer_list<PageFragment> fragments = {});
static const String webRadioPage();
static const String webMemoryPage();
static const String webConfigPage();

struct SplashUploadState
{
  bool incomplete;
  bool tooLarge;
};

static bool webIsAuthenticated(AsyncWebServerRequest *request)
{
  return(loginUsername == "" || loginPassword == "" ||
         request->authenticate(loginUsername.c_str(), loginPassword.c_str()));
}

//
// Delayed WiFi connection
//
void netRequestConnect()
{
  connectTime = millis();
  itIsTimeToWiFi = true;
}

void netTickTime()
{
  otaTick();

  // Connect to WiFi if requested
  if(itIsTimeToWiFi && ((millis() - connectTime) > CONNECT_TIME))
  {
    netInit(wifiModeIdx);
    itIsTimeToWiFi = false;
  }
}

//
// Get current connection status
// (-1 - not connected, 0 - disabled, 1 - connected, 2 - connected to network)
//
int8_t getWiFiStatus()
{
  wifi_mode_t mode = WiFi.getMode();

  switch(mode)
  {
    case WIFI_MODE_NULL:
      return(0);
    case WIFI_AP:
      return(WiFi.softAPgetStationNum()? 1 : -1);
    case WIFI_STA:
      return(WiFi.status()==WL_CONNECTED? 2 : -1);
    case WIFI_AP_STA:
      return((WiFi.status()==WL_CONNECTED)? 2 : WiFi.softAPgetStationNum()? 1 : -1);
    default:
      return(-1);
  }
}

char *getWiFiIPAddress()
{
  static char ip[16];
  return strcpy(ip, WiFi.status()==WL_CONNECTED ? WiFi.localIP().toString().c_str() : "");
}

//
// Stop WiFi hardware
//
void netStop()
{
  tcpStop();
  wifi_mode_t mode = WiFi.getMode();

  MDNS.end();

  // If network connection up, shut it down
  if((mode==WIFI_STA) || (mode==WIFI_AP_STA))
    WiFi.disconnect(true);

  // If access point up, shut it down
  if((mode==WIFI_AP) || (mode==WIFI_AP_STA))
    WiFi.softAPdisconnect(true);

  WiFi.mode(WIFI_MODE_NULL);
}

//
// Initialize WiFi network and services
//
void netInit(uint8_t netMode)
{
  // Always disable WiFi first
  netStop();
  wifiRegisterPowerLevelCallback();

  switch(netMode)
  {
    case NET_OFF:
      // Do not initialize WiFi if disabled
      return;
    case NET_AP_ONLY:
      // Start WiFi access point if requested
      WiFi.mode(WIFI_AP);
      wifiInitAP();
      break;
    case NET_AP_CONNECT:
      // Start WiFi access point if requested
      WiFi.mode(WIFI_AP_STA);
      wifiInitAP();
      break;
    default:
      // No access point
      WiFi.mode(WIFI_STA);
      break;
  }

  // Initialize WiFi and try connecting to a network
  if(netMode>NET_AP_ONLY && wifiConnect())
  {
    // NTP time updates will happen every 5 minutes
    ntpClient.setUpdateInterval(5*60*1000);

    // Get NTP time from the network
    clockReset();
    for(int j=0 ; j<10 ; j++)
      if(ntpSyncTime()) break; else delay(500);

    // Start the result timeout after the blocking time synchronization.
    if(netMode!=NET_SYNC)
      statusShow(
        ("Connected to WiFi network (" + WiFi.SSID() + ")").c_str(),
        ("IP : " + WiFi.localIP().toString() + " or atsmini.local").c_str()
      );
    else
      statusShow(nullptr);
  }
  else if(netMode==NET_AP_ONLY || netMode==NET_AP_CONNECT)
  {
    // Show the access point details when it is the available connection.
    statusShow(
      ("Use Access Point " + String(apSSID)).c_str(),
      ("IP : " + WiFi.softAPIP().toString() + " or atsmini.local").c_str()
    );
  }
  else
    statusShow("Connecting to WiFi network...", "No WiFi connection");

  // If only connected to sync...
  if(netMode==NET_SYNC)
  {
    // Drop network connection
    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_NULL);
  }
  else
  {
    // Initialize web server for remote configuration
    webInit();

    // Initialize mDNS
    MDNS.begin("atsmini"); // Set the hostname to "atsmini.local"
    MDNS.addService("http", "tcp", 80);
  }
}

//
// Returns TRUE if NTP time is available
//
bool ntpIsAvailable()
{
  return(ntpClient.isTimeSet());
}

//
// Update NTP time and synchronize clock with NTP time
//
bool ntpSyncTime()
{
  if(WiFi.status()==WL_CONNECTED)
  {
    ntpClient.update();

    if(ntpClient.isTimeSet())
      return(clockSetEpoch(ntpClient.getEpochTime()));
  }
  return(false);
}

static void wifiRegisterPowerLevelCallback()
{
  static bool registered = false;

  if(registered) return;

  WiFi.onEvent(wifiPowerLevelOnEvent, ARDUINO_EVENT_WIFI_AP_START);
  WiFi.onEvent(wifiPowerLevelOnEvent, ARDUINO_EVENT_WIFI_STA_START);
  registered = true;
}

static void wifiPowerLevelOnEvent(WiFiEvent_t event)
{
  (void)event;
  WiFi.setTxPower(WIFI_POWER_LEVEL);
}

//
// Initialize WiFi access point (AP)
//
static bool wifiInitAP()
{
  // These are our own access point (AP) addresses
  IPAddress ip(10, 1, 1, 1);
  IPAddress gateway(10, 1, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  // Start as access point (AP)
  WiFi.softAP(apSSID, apPWD, apChannel, apHideMe, apClients);
  WiFi.softAPConfig(ip, gateway, subnet);

  return(true);
}

//
// Connect to a WiFi network
//
static bool wifiConnect()
{
  // Clean credentials
  wifiMulti.APlistClean();

  // Get the preferences
  prefs.begin("network", true, STORAGE_PARTITION);
  loginUsername = prefs.getString("loginusername", "");
  loginPassword = prefs.getString("loginpassword", "");
  wifiScanHidden = prefs.getBool("wifiscanhidden", false);

  // Try connecting to known WiFi networks
  for(int j=0 ; (j<3) ; j++)
  {
    char nameSSID[16], namePASS[16];
    sprintf(nameSSID, "wifissid%d", j+1);
    sprintf(namePASS, "wifipass%d", j+1);

    String ssid = prefs.getString(nameSSID, "");
    String password = prefs.getString(namePASS, "");

    if(ssid != "")
      wifiMulti.addAP(ssid.c_str(), password.c_str());
  }

  // Done with preferences
  prefs.end();

  statusShow("Connecting to WiFi network...", nullptr, 0);
  drawScreen();

  consumeAbortPending();
  wl_status_t wifiStatus = WL_NO_SSID_AVAIL;
  uint32_t start = millis();
  while(((millis() - start)<WIFI_MULTI_TOTAL_TIMEOUT) && (wifiStatus!=WL_CONNECTED))
  {
    wifiStatus = (wl_status_t)wifiMulti.run(5000, wifiScanHidden);

    if(consumeAbortPending())
    {
      WiFi.disconnect();
      break;
    }

    if((wifiStatus!=WL_CONNECTED) && ((millis() - start)<WIFI_MULTI_TOTAL_TIMEOUT))
      delay(1000);
  }

  return(wifiStatus == WL_CONNECTED);
}

//
// Initialize internal web server
//
static void webInit()
{
  server.on("/", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/html", webRadioPage());
  });

  server.on("/rds.json", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    auto jsonEscape = [](const char *input) -> String {
      String output;
      if(!input) return output;
      while(*input)
      {
        const unsigned char c = (unsigned char)*input++;
        if(c == '"' || c == '\\') { output += '\\'; output += (char)c; }
        else if(c == '\n') output += "\\n";
        else if(c == '\r') output += "\\r";
        else if(c == '\t') output += "\\t";
        else if(c >= 0x20) output += (char)c;
      }
      return output;
    };
    String station, radioText, programType;
    if(currentMode == FM)
    {
      // getProgramInfo() is the RDS program type (e.g. "Pop Music"),
      // not the station's Program Service (PS) name. Use getStationName()
      // for the actual RDS station name, with its built-in memory fallback.
      const char *ps = getStationName();
      if(ps && ps[0]) station = ps;
      const char *rt = getRadioText();
      if(rt && rt[0]) radioText = rt;
      const char *pty = getProgramInfo();
      if(pty && pty[0]) programType = pty;
    }
    if(station.length() == 0)
    {
      uint32_t freqHz = currentMode == FM
        ? (uint32_t)currentFrequency * 10000UL
        : (uint32_t)currentFrequency * 1000UL + currentBFO;
      char memoryName[10] = {};
      if(findMemoryName(freqHz, currentMode, 255, 255, memoryName) && memoryName[0])
        station = memoryName;
    }
    String json = "{\"station\":\"" + jsonEscape(station.c_str()) + "\",\"text\":\"" + jsonEscape(radioText.c_str()) + "\",\"pty\":\"" + jsonEscape(programType.c_str()) + "\"}";
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
  });

  server.on("/memories.json", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    auto jsonEscape = [](const char *input) -> String {
      String output;
      if(!input) return output;
      while(*input)
      {
        const unsigned char c = (unsigned char)*input++;
        if(c == '"' || c == '\\') { output += '\\'; output += (char)c; }
        else if(c == '\n') output += "\\n";
        else if(c == '\r') output += "\\r";
        else if(c == '\t') output += "\\t";
        else if(c >= 0x20) output += (char)c;
      }
      return output;
    };
    String json = "{\"memories\":[";
    bool first = true;
    for(int i = 0; i < getTotalMemories(); i++)
    {
      const Memory memory = getMemory(i);
      if(!memory.freq || !memory.name[0] || memory.mode >= getTotalModes()) continue;
      if(!first) json += ",";
      first = false;
      json += "{\"slot\":" + String(i) +
              ",\"name\":\"" + jsonEscape(memory.name) +
              "\",\"freq\":" + String(memory.freq) +
              ",\"mode\":\"" + jsonEscape(bandModeDesc[memory.mode]) +
              "\",\"displayFrequency\":\"" +
              String(memory.mode == FM ? memory.freq / 1000000.0f : memory.freq / 1000.0f, memory.mode == FM ? 2 : 1) +
              (memory.mode == FM ? " MHz\"}" : " kHz\"}");
    }
    json += "]}";
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
  });

  server.on("/radio/diagnostics.log", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    if(!LittleFS.exists(WEB_DIAG_PATH)) return request->send(404, "text/plain", "No diagnostic log yet.");
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, WEB_DIAG_PATH, "text/plain", true);
    response->addHeader("Cache-Control", "no-store"); response->addHeader("Content-Disposition", "attachment; filename=ats-mini-diagnostics.log"); request->send(response);
  });

  server.on("/radio/tune-status.json", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    String json = "{\"request_id\":" + String((uint32_t)webTuneRequestId) +
      ",\"completed_id\":" + String((uint32_t)webTuneCompletedId) +
      ",\"result\":" + String((int)webTuneResult) +
      ",\"target_hz\":" + String((uint32_t)webTuneTargetHz) +
      ",\"actual_hz\":" + String((uint32_t)webTuneActualHz) +
      ",\"target_mode\":" + String((int)webTuneTargetMode) +
      ",\"actual_mode\":" + String((int)webTuneActualMode) +
      ",\"phase\":" + String((int)webTunePhase) + "}";
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
  });

  server.on("/radio/tune", HTTP_POST, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    if(webSpectrumBusy || webSpectrumRequested) return request->send(409, "application/json", "{\"message\":\"Radio tuning is unavailable during a spectrum sweep.\"}");
    if(currentCmd == CMD_DUAL_RX) return request->send(409, "application/json", "{\"message\":\"Tuning is unavailable during Dual RX.\"}");
    if(!request->hasParam("freq", true)) return request->send(400, "application/json", "{\"message\":\"Missing frequency.\"}");
    String value = request->getParam("freq", true)->value();
    char *end = nullptr;
    long freqHz = strtol(value.c_str(), &end, 10);
    if(!end || *end || freqHz < 100000 || freqHz > 300000000) return request->send(400, "application/json", "{\"message\":\"Invalid frequency.\"}");
    int8_t targetMode = currentMode;
    bool modeProvided = request->hasParam("mode", true);
    if(modeProvided)
    {
      String modeValue = request->getParam("mode", true)->value();
      if(modeValue == "FM") targetMode = FM;
      else if(modeValue == "AM") targetMode = AM;
      else if(modeValue == "USB") targetMode = USB;
      else if(modeValue == "LSB") targetMode = LSB;
      else return request->send(400, "application/json", "{\"message\":\"Unsupported modulation.\"}");
    }
    uint32_t minHz = targetMode == FM ? 64000000UL : 150000UL;
    uint32_t maxHz = targetMode == FM ? 108000000UL : 30000000UL;
    if(freqHz < minHz || freqHz > maxHz)
      return request->send(422, "application/json", "{\"message\":\"Invalid frequency for the selected modulation. Receiver frequency was not changed.\"}");

    // Without an explicit modulation switch, reject frequencies outside the
    // active band's limits before queueing any change. Explicit mode requests
    // can switch from VHF/FM to ALL in the main-loop handler.
    if(!modeProvided)
    {
      Band *activeBand = getCurrentBand();
      uint16_t targetFreq = freqFromHz((uint32_t)freqHz, targetMode);
      if(!activeBand || !isFreqInBand(activeBand, targetFreq) ||
         (isSSB() && targetFreq == activeBand->maximumFreq && (freqHz % 1000UL)))
        return request->send(422, "application/json", "{\"message\":\"Frequency is outside the current band's limits. Receiver frequency was not changed.\"}");
    }
    uint32_t requestId = (uint32_t)webTuneRequestId + 1;
    if(requestId == 0) requestId = 1;
    webTuneRequestId = requestId;
    webTuneTargetHz = (uint32_t)freqHz;
    webTuneTargetMode = targetMode;
    webTuneResult = 0;
    webTunePhase = 1;
    webTuneRequestedMode = modeProvided ? targetMode : -1;
    webTuneFrequencyHz = freqHz;
    String response = "{\"message\":\"Tune command queued.\",\"request_id\":" + String(requestId) + "}";
    return request->send(202, "application/json", response);
  });
  server.on("/radio/step", HTTP_POST, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    if(webSpectrumBusy || webSpectrumRequested) return request->send(409, "application/json", "{\"message\":\"Radio tuning is unavailable during a spectrum sweep.\"}");
    if(currentCmd == CMD_DUAL_RX) return request->send(409, "application/json", "{\"message\":\"Tuning is unavailable during Dual RX.\"}");
    if(!request->hasParam("direction", true)) return request->send(400, "application/json", "{\"message\":\"Missing direction.\"}");
    String value = request->getParam("direction", true)->value();
    if(value != "up" && value != "down") return request->send(400, "application/json", "{\"message\":\"Direction must be up or down.\"}");
    webTuneStep = value == "up" ? 1 : -1;
    return request->send(202, "application/json", "{\"message\":\"Step tune queued.\"}");
  });
  server.on("/radio/volume", HTTP_POST, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    if(webSpectrumBusy || webSpectrumRequested) return request->send(409, "application/json", "{\"message\":\"Volume changes are unavailable during a spectrum sweep.\"}");
    if(!request->hasParam("volume", true)) return request->send(400, "application/json", "{\"message\":\"Missing volume.\"}");
    String value = request->getParam("volume", true)->value();
    char *end = nullptr;
    long requested = strtol(value.c_str(), &end, 10);
    if(!end || *end || requested < 0 || requested > 63) return request->send(400, "application/json", "{\"message\":\"Volume must be 0–63.\"}");
    webRequestedVolume = requested;
    return request->send(202, "application/json", "{\"message\":\"Volume change queued.\"}");
  });
  server.on("/spectrum/start", HTTP_POST, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    if(currentCmd == CMD_DUAL_RX)
      return request->send(409, "application/json", "{\"ready\":false,\"error\":\"Stop Dual RX before scanning.\"}");
    if(webSpectrumBusy || webSpectrumRequested)
      return request->send(409, "application/json", "{\"ready\":false,\"error\":\"A spectrum scan is already running.\"}");

    bool hasStart = request->hasParam("start_hz", true);
    bool hasEnd = request->hasParam("end_hz", true);
    uint16_t requestedStart = 0, requestedEnd = 0;
    if(hasStart || hasEnd)
    {
      if(!hasStart || !hasEnd)
        return request->send(400, "application/json", "{\"ready\":false,\"error\":\"Provide both start and end frequencies.\"}");
      String modeValue = request->hasParam("mode", true) ? request->getParam("mode", true)->value() : String("FM");
      if(modeValue != "FM")
        return request->send(422, "application/json", "{\"ready\":false,\"error\":\"Spectrum scanning is FM-only. Select a 64–108 MHz range.\"}");
      const int8_t requestedMode = FM;
      String startValue = request->getParam("start_hz", true)->value();
      String endValue = request->getParam("end_hz", true)->value();
      char *startEnd = nullptr, *endEnd = nullptr;
      long startHz = strtol(startValue.c_str(), &startEnd, 10);
      long endHz = strtol(endValue.c_str(), &endEnd, 10);
      if(!startEnd || *startEnd || !endEnd || *endEnd || startHz < 64000000L || endHz > 108000000L || endHz <= startHz)
        return request->send(400, "application/json", "{\"ready\":false,\"error\":\"FM range must be between 64 and 108 MHz.\"}");
      requestedStart = freqFromHz((uint32_t)startHz, FM);
      requestedEnd = freqFromHz((uint32_t)endHz, FM);
      uint32_t stepHz = 100000UL;
      if(request->hasParam("step_hz", true))
      {
        String stepValue = request->getParam("step_hz", true)->value();
        char *stepEnd = nullptr;
        long parsedStep = strtol(stepValue.c_str(), &stepEnd, 10);
        if(!stepEnd || *stepEnd || parsedStep != 100000L)
          return request->send(422, "application/json", "{\"ready\":false,\"error\":\"FM spectrum scan uses a fixed 100 kHz step.\"}");
        stepHz = (uint32_t)parsedStep;
      }
      uint16_t step = 10;
      uint32_t points = ((uint32_t)requestedEnd - requestedStart) / step + 1;
      if(points < 2 || points > 256)
        return request->send(422, "application/json", "{\"ready\":false,\"error\":\"Range must contain 2–256 samples at the selected resolution.\"}");
      webSpectrumRequestedMode = requestedMode;
      webSpectrumStep = step;
    }

    webSpectrumStartFreq = requestedStart;
    webSpectrumEndFreq = requestedEnd;
    webSpectrumCustomRange = hasStart && hasEnd;
    webSpectrumReady = false;
    webSpectrumBusy = true;
    webSpectrumRequested = true;
    request->send(202, "application/json", "{\"ready\":false}");
  });

  server.on("/spectrum.json", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    if(!webSpectrumReady)
      return request->send(200, "application/json", "{\"ready\":false}");
    String json;
    json.reserve(7200);
    int8_t resultMode = webSpectrumResultMode >= 0 ? webSpectrumResultMode : currentMode;
    String modeName = resultMode == FM ? "FM" : resultMode == AM ? "AM" : resultMode == USB ? "USB" : "LSB";
    json = "{\"ready\":true,\"fm\":" + String(resultMode == FM ? "true" : "false") + ",\"mode\":\"" + modeName + "\",\"samples\":[";
    uint16_t count = scanGetCount();
    for(uint16_t i = 0; i < count; i++)
    {
      uint16_t freq = 0;
      uint8_t signal = 0, noise = 0;
      if(!scanGetPoint(i, &freq, &signal, &noise)) break;
      if(i) json += ',';
      json += "{\"f\":" + String(freq) + ",\"r\":" + String(signal) + ",\"s\":" + String(noise) + "}";
    }
    json += "]}";
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
  });

  server.on("/log", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    String html = R"HTML(<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta name="theme-color" content="#080d12"><title>ATS Debug Log</title><style>
:root{color-scheme:dark;--bg:#020611;--panel:#030d27;--line:#123a78;--text:#e7f1ff;--muted:#7192c6;--blue:#167bff}*{box-sizing:border-box}body{margin:0;background:linear-gradient(180deg,#020611,#000);color:var(--text);font:16px/1.45 system-ui,sans-serif}.APP{max-width:1040px;margin:auto;padding:16px 12px 28px}.STAT{padding:16px;border:1px solid #1454ad;background:linear-gradient(180deg,#030d25,#020712)}.EYEBROW{color:var(--blue);font-size:.74rem;font-weight:750;letter-spacing:.16em;text-transform:uppercase}.SUB,.LABEL{color:var(--muted)}button{padding:10px 14px;border:1px solid #16458f;background:#061d4b;color:#dce5ff;font:inherit;margin:4px 4px 8px 0}textarea{width:100%;min-height:55vh;background:#010713;color:#dce5ff;border:1px solid #16458f;padding:10px;font:12px/1.45 monospace;white-space:pre;overflow:auto}.NAV{display:grid;grid-template-columns:repeat(6,minmax(0,1fr));margin:0 0 16px;border:1px solid #10356e}.NAV a{padding:12px 4px;text-align:center;color:#dce5ff;text-decoration:none;border-right:1px solid #10356e;font-size:.85rem}.NAV .ACTIVE{color:#80a0ff;background:#1a2946;font-weight:800}@media(max-width:500px){.NAV a{font-size:.7rem;padding:10px 2px}}</style></head><body><main class="APP">
<nav class="NAV"><a href="/">Radio</a><a href="/memory">Memory</a><a href="/config">Config</a><a href="/update">Update</a><a href="/patches">Patches</a><span class="ACTIVE" style="padding:12px 4px;text-align:center">Log</span></nav>
<section class="STAT"><div class="EYEBROW">Diagnostics</div><h2>WEB CONTROL LOG</h2><p class="SUB">Actions and errors observed by this browser. The log is stored locally in this browser, not on the receiver.</p>
<button id="refresh">REFRESH</button><button id="export">EXPORT / COPY LOG</button><button id="clear">CLEAR LOG</button>
<textarea id="output" readonly></textarea><p id="status" class="LABEL">Ready</p></section></main><script>
(()=>{const key='atsControlDebugLog',out=document.getElementById('output'),status=document.getElementById('status');function read(){try{return JSON.parse(localStorage.getItem(key)||'[]')}catch(_){return []}}function render(){out.value=read().map(e=>e.time+' | '+e.type+' | '+e.message).join('\n');out.scrollTop=out.scrollHeight;status.textContent=read().length+' entries. Log survives page reloads in this browser.'}document.getElementById('refresh').onclick=render;document.getElementById('export').onclick=()=>{out.value=JSON.stringify({exportedAt:new Date().toISOString(),userAgent:navigator.userAgent,entries:read()},null,2);out.focus();out.select();let ok=false;try{ok=document.execCommand('copy')}catch(_){}status.textContent=ok?'Log copied. Paste it into the support chat.':'Log selected; copy it and paste into the support chat.'};document.getElementById('clear').onclick=()=>{if(confirm('Delete all browser log entries?')){localStorage.removeItem(key);render()}};render()})();
</script></body></html>)HTML";
    request->send(200, "text/html; charset=utf-8", html);
  });

  server.on("/memory", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    request->send(200, "text/html", webMemoryPage());
  });
  server.on("/memory", HTTP_POST, webSetMemories);

  server.on("/config", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    request->send(200, "text/html", webConfigPage());
  });

  server.on("/splash.png", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(!LittleFS.exists(SPLASH_PATH))
      return request->send(404, "text/plain", "Not found");
    request->send(LittleFS, SPLASH_PATH, "image/png");
  });

  server.onNotFound([] (AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

  // This method saves configuration form contents
  server.on("/setconfig", HTTP_POST, webSetConfig, webUploadSplash);

  // Register subpaths first: the server also matches /update to /update/... .
  server.on("/update/upload", HTTP_POST, webUploadFirmwareComplete, webUploadFirmware);
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    webUpdatePage(request, otaStatus(), 200);
  });
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(!webIsAuthenticated(request)) return request->requestAuthentication();
    if(!otaRequestLatest(request->hasParam("action", true) && request->getParam("action", true)->value() == "install"))
      return webUpdatePage(request, {OTA_FAILED, "An update is already in progress."}, 409);
    request->redirect("/update");
  });

  server.on("/patches", HTTP_GET, webPatchesPage);
  server.on("/patches", HTTP_POST, webPatchesComplete, webUploadPatch);

  // Start web server
  server.begin();
}

static void webUploadFirmware(AsyncWebServerRequest *request, const String &filename,
                              size_t index, uint8_t *data, size_t len, bool)
{
  if(!webIsAuthenticated(request) || request->getResponse()) return;

  if(!index)
  {
    if(!filename.endsWith(".bin"))
      return webUpdatePage(request, {OTA_FAILED, "Select a firmware .bin file."});
    const auto *param = request->getParam("size", true);
    if(!param) return webUpdatePage(request, {OTA_FAILED, "Invalid firmware size."});
    size_t imageSize = strtoul(param->value().c_str(), nullptr, 10);
    // Round-trip the number to reject signs, whitespace, suffixes, and overflow.
    if(!imageSize || imageSize > request->contentLength() || String(imageSize) != param->value())
      return webUpdatePage(request, {OTA_FAILED, "Invalid firmware size."});
    if(!otaBegin(imageSize))
      return webUpdatePage(request, {OTA_FAILED, "An update is already in progress."}, 409);
    request->client()->setRxTimeout(15);
    request->onDisconnect([]() { otaEndUpload(); });
  }

  if(!otaWrite(data, len)) return webUpdatePage(request);
}

static void webUploadFirmwareComplete(AsyncWebServerRequest *request)
{
  if(!webIsAuthenticated(request)) return request->requestAuthentication();
  if(request->getResponse()) return; // Preserve an upload error queued above.
  if(!request->hasParam("firmware", true, true))
    return webUpdatePage(request, {OTA_FAILED, "No complete firmware uploaded."});
  otaFinish();
  webUpdatePage(request);
}

static void webUploadSplash(AsyncWebServerRequest *request, const String &filename,
                            size_t index, uint8_t *data, size_t len, bool final)
{
  if(!webIsAuthenticated(request)) return;

  if(index == 0)
  {
    LittleFS.remove(SPLASH_TEMP_PATH);

    SplashUploadState *state = static_cast<SplashUploadState *>(calloc(1, sizeof(SplashUploadState)));
    if(!state) return;

    request->_tempObject = state;
    request->onDisconnect([request, state]() {
      if(state->incomplete)
        request->_tempFile.close();

      // Discard an upload that was not installed by webSetConfig().
      LittleFS.remove(SPLASH_TEMP_PATH);
    });

    // The browser filter is only advisory, so enforce the extension here too.
    if(filename.endsWith(".png"))
    {
      request->_tempFile = LittleFS.open(SPLASH_TEMP_PATH, "w");
      state->incomplete = request->_tempFile;
    }
  }

  SplashUploadState *state = static_cast<SplashUploadState *>(request->_tempObject);

  if(request->_tempFile && len)
  {
    if((index + len) > SPLASH_MAX_FILE_SIZE)
    {
      state->tooLarge = true;
      state->incomplete = false;
      request->_tempFile.close();
      LittleFS.remove(SPLASH_TEMP_PATH);
    }
    else if(request->_tempFile.write(data, len) != len)
    {
      state->incomplete = false;
      request->_tempFile.close();
      LittleFS.remove(SPLASH_TEMP_PATH);
    }
  }

  if(final && request->_tempFile)
    request->_tempFile.close();

  if(final && state)
    state->incomplete = false;
}

void webSetConfig(AsyncWebServerRequest *request)
{
  if(!webIsAuthenticated(request)) return request->requestAuthentication();

  uint32_t prefsSave = 0;
  uint32_t epoch;
  bool setClock = false;

  if(request->hasParam("datetime", true))
  {
    String dateTime = request->getParam("datetime", true)->value();
    if(dateTime != "")
    {
      if(!webParseUTCDateTime(dateTime, &epoch))
        return request->send(400, "text/plain", "Date/time must use the YYYY-mm-dd HH:MM:SS format and contain a valid UTC date and time.");
      setClock = true;
    }
  }

  if(request->hasParam("deletesplash", true))
  {
    LittleFS.remove(SPLASH_TEMP_PATH);
    LittleFS.remove(SPLASH_PATH);
  }
  else if(request->hasParam("splash", true, true))
  {
    String filename = request->getParam("splash", true, true)->value();

    if(filename != "")
    {
      SplashUploadState *state = static_cast<SplashUploadState *>(request->_tempObject);
      if(state && state->tooLarge)
        return request->send(413, "text/plain", "The splash image must not exceed 512 KB.");

      if(!filename.endsWith(".png"))
      {
        LittleFS.remove(SPLASH_TEMP_PATH);
        return request->send(400, "text/plain", "The splash image filename must end in .png.");
      }

      if(!LittleFS.exists(SPLASH_TEMP_PATH))
        return request->send(500, "text/plain", "The splash image could not be stored.");

      String error = splashValidate();
      if(error != "")
      {
        LittleFS.remove(SPLASH_TEMP_PATH);
        return request->send(400, "text/plain", error);
      }

      if(!LittleFS.rename(SPLASH_TEMP_PATH, SPLASH_PATH))
      {
        LittleFS.remove(SPLASH_TEMP_PATH);
        return request->send(500, "text/plain", "The splash image could not be installed.");
      }
    }
  }

  // Start modifying preferences
  prefs.begin("network", false, STORAGE_PARTITION);

  // Save user name and password
  if(request->hasParam("username", true) && request->hasParam("password", true))
  {
    loginUsername = request->getParam("username", true)->value();
    loginPassword = request->getParam("password", true)->value();

    prefs.putString("loginusername", loginUsername);
    prefs.putString("loginpassword", loginPassword);
  }

  // Save SSIDs and their passwords
  bool haveSSID = false;
  for(int j=0 ; j<3 ; j++)
  {
    char nameSSID[16], namePASS[16];

    sprintf(nameSSID, "wifissid%d", j+1);
    sprintf(namePASS, "wifipass%d", j+1);

    if(request->hasParam(nameSSID, true) && request->hasParam(namePASS, true))
    {
      String ssid = request->getParam(nameSSID, true)->value();
      String pass = request->getParam(namePASS, true)->value();
      prefs.putString(nameSSID, ssid);
      prefs.putString(namePASS, pass);
      haveSSID |= ssid != "" && pass != "";
    }
  }

  // Save hidden SSID scanning preference
  wifiScanHidden = request->hasParam("wifiscanhidden", true);
  prefs.putBool("wifiscanhidden", wifiScanHidden);

  // Save time zone
  if(request->hasParam("utcoffset", true))
  {
    int idx = request->getParam("utcoffset", true)->value().toInt();
    if(idx >= 0 && idx < getTotalUTCOffsets())
    {
      utcOffsetIdx = idx;
      prefsSave |= SAVE_SETTINGS;
    }
  }

  // Save theme
  if(request->hasParam("theme", true))
  {
    String theme = request->getParam("theme", true)->value();
    themeIdx = theme.toInt();
    prefsSave |= SAVE_SETTINGS;
  }

  // Save scroll direction and menu zoom
  scrollDirection = request->hasParam("scroll", true)? -1 : 1;
  zoomMenu        = request->hasParam("zoom", true);
  setEncoderHalfStep(request->hasParam("encoderhalfstep", true));
  prefsSave |= SAVE_SETTINGS;

  // Done with the preferences
  prefs.end();

  // Save preferences immediately
  prefsRequestSave(prefsSave, true);

  if(setClock) clockSetEpoch(epoch);

  // Show config page again
  request->redirect("/config");

  // If we are currently in AP mode, and infrastructure mode requested,
  // and there is at least one SSID / PASS pair, request network connection
  if(haveSSID && (wifiModeIdx>NET_AP_ONLY) && (WiFi.status()!=WL_CONNECTED))
    netRequestConnect();
}

static bool webParseUTCDateTime(const String &text, uint32_t *epoch)
{
  int year, month, day, hour, minute, second;
  return(epoch && text.length() == 19 &&
         sscanf(text.c_str(), "%4d-%2d-%2d %2d:%2d:%2d",
                &year, &month, &day, &hour, &minute, &second) == 6 &&
         clockUTCDateTimeToEpoch(year, month, day, hour, minute, second, epoch));
}

static String webPage(const char *body, const char *title, std::initializer_list<PageValue> values,
                      std::initializer_list<PageFragment> fragments)
{
  String page;
  pageReserve(page, sizeof(pageStart) + strlen(title) + strlen(body) + sizeof(pageEnd) + 64);
  pageAppend(page, pageStart, {{"title", title}});
  pageAppend(page, body, values, fragments);
  pageReserve(page, sizeof(pageEnd) + 64);
  // Snapshot after allocating the page, before the HTTP response copies it.
  multi_heap_info_t heap, psram;
  heap_caps_get_info(&heap, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  heap_caps_get_info(&psram, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  pageAppend(page, pageEnd, {
    {"heap_used", String(heap.total_allocated_bytes / 1024.0, 1)},
    {"heap_free", String(heap.total_free_bytes / 1024.0, 1)},
    {"psram_used", String(psram.total_allocated_bytes / 1024.0, 1)},
    {"psram_free", String(psram.total_free_bytes / 1024.0, 1)}
  });
  return page;
}

static String webLedFrequency(const String &text)
{
  String out;
  out.reserve(text.length() * 620);
  const char *segments[10] = {
    "abcdef", "bc", "abged", "abgcd", "fgbc", "afgcd", "afgcde", "abc",
    "abcdefg", "abfgcd"
  };
  for(size_t i = 0; i < text.length(); i++)
  {
    char ch = text[i];
    if(ch >= '0' && ch <= '9')
    {
      out += F("<svg class=\"LED-DIGIT\" viewBox=\"0 0 24 37\" aria-hidden=\"true\">");
      const char *on = segments[ch - '0'];
      const char *names = "abcdefg";
      const int x[7] = {4,20,20,4,1,1,4};
      const int y[7] = {2,5,20,32,20,5,17};
      const int w[7] = {16,3,3,16,3,3,16};
      const int h[7] = {3,12,12,3,12,12,3};
      for(uint8_t s = 0; s < 7; s++)
      {
        bool lit = strchr(on, names[s]) != nullptr;
        out += F("<rect x=\"");
        out += String(x[s]); out += F("\" y=\"");
        out += String(y[s]); out += F("\" width=\"");
        out += String(w[s]); out += F("\" height=\"");
        out += String(h[s]); out += F("\" fill=\"");
        out += lit ? F("#167bff") : F("#061630");
        out += F("\"/>");
      }
      out += F("</svg>");
    }
    else if(ch == '.')
      out += F("<svg class=\"LED-DOT\" viewBox=\"0 0 8 37\" aria-hidden=\"true\"><rect x=\"1\" y=\"31\" width=\"6\" height=\"6\" fill=\"#167bff\"/></svg>");
  }
  out += currentMode == FM ? F("<span class=\"LED-UNIT\">MHz</span>") : F("<span class=\"LED-UNIT\">kHz</span>");
  return out;
}

static const String webRadioPage()
{
  String ip = "";
  String ssid = "";
  String receiverTime = "Not synchronized";
  int offsetMinutes = getCurrentUTCOffset() * 15;
  int offsetMagnitude = abs(offsetMinutes);
  char utcOffset[10];
  snprintf(utcOffset, sizeof(utcOffset), "UTC%c%02d:%02d",
           offsetMinutes < 0? '-' : '+', offsetMagnitude / 60, offsetMagnitude % 60);
  String freq = currentMode == FM?
    String(currentFrequency / 100.0) + "MHz "
  : String(currentFrequency + currentBFO / 1000.0) + "kHz ";

  if(clockAvailable())
  {
    time_t localTime = time(NULL) + offsetMinutes * 60;
    struct tm fields;
    gmtime_r(&localTime, &fields);
    char text[20];

    strftime(text, sizeof(text),
             clockGetDate(NULL, NULL, NULL, NULL)? "%Y-%m-%d %H:%M:%S" : "%H:%M:%S",
             &fields);

    receiverTime = text;
  }

  receiverTime += " (" + String(utcOffset) + ")";

  if(WiFi.status()==WL_CONNECTED)
  {
    ip = WiFi.localIP().toString();
    ssid = WiFi.SSID();
  }
  else
  {
    ip = WiFi.softAPIP().toString();
    ssid = String(apSSID);
  }

  String connection = WiFi.status() == WL_CONNECTED ? "Wi-Fi connected" :
                      WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA ?
                      "Local access point" : "Radio online";

  return webPage(pageStatus, pageStatusTitle, {
    {"title", pageStatusTitle},
    {"connection", connection},
    {"ip", ip}, {"ssid", ssid},
    {"mac", String(getMACAddress())}, {"version", String(getVersion(true))},
    {"time", receiverTime}, {"band", getCurrentBand()->bandName},
    {"frequency", freq}, {"frequency_hz", String((uint32_t)currentFrequency * (currentMode == FM ? 10000UL : 1000UL))}, {"frequency_step_hz", currentMode == FM ? "10000" : "1000"}, {"volume", String(volume)}, {"mode", bandModeDesc[currentMode]},
    {"stereo", currentMode == FM && fmStereoIdx != FM_STEREO_MONO && rx.getCurrentPilot()? String("<SPAN CLASS=\"STEREO\">STEREO</SPAN>") : String("")},
    {"rssi", String(rssi)}, {"snr", String(snr)}, {"battery", String(batteryMonitor())}
  }, {
    {"navigation", [](String &out) { webNavigation(out, "/"); }},
    {"led_frequency", [freq](String &out) { out += webLedFrequency(freq); }}
  });
}

static void webSetMemories(AsyncWebServerRequest *request)
{
  if(!webIsAuthenticated(request)) return request->requestAuthentication();

  // Validate a complete replacement before modifying any live slots.
  Memory *pending = static_cast<Memory *>(ps_calloc(getTotalMemories(), sizeof(Memory)));
  if(!pending) return request->send(503, "text/plain", "Not enough memory to save the slots.");
  const char *error = nullptr;
  int slot = 0;
  for(; slot<getTotalMemories() ; slot++)
  {
    String suffix(slot);
    const auto *name = request->getParam("name" + suffix, true);
    const auto *band = request->getParam("band" + suffix, true);
    const auto *freq = request->getParam("freq" + suffix, true);
    const auto *mode = request->getParam("mode" + suffix, true);
    if(!name || !band || !freq || !mode)
    {
      error = "Missing slot fields.";
      break;
    }

    Memory &mem = pending[slot];
    const String &label = name->value();
    if(label.length() >= sizeof(mem.name)) error = "Names must be at most 9 characters.";
    for(size_t i=0 ; !error && i<label.length() ; i++)
      if((uint8_t)label[i]<0x20 || (uint8_t)label[i]>0x7e)
        error = "Names must contain printable ASCII characters.";
    if(error) break;
    memcpy(mem.name, label.c_str(), label.length());

    const String &number = freq->value();
    if(!number.length() || number.length()>10) error = "Invalid frequency in Hz.";
    uint64_t hz = 0;
    for(size_t i=0 ; !error && i<number.length() ; i++)
    {
      if(number[i]<'0' || number[i]>'9') error = "Invalid frequency in Hz.";
      else hz = hz * 10 + number[i] - '0';
    }
    if(hz>UINT32_MAX) error = "Frequency is out of range.";
    if(error) break;
    mem.freq = hz;

    mem.mode = AM;
    if(mode->value().length() || mem.freq)
    {
      mem.mode = 0xff;
      for(int i=0 ; i<getTotalModes() ; i++)
        if(mode->value() == bandModeDesc[i]) mem.mode = i;
      if(mem.mode == 0xff)
      {
        error = "Unknown mode.";
        break;
      }
    }

    mem.band = 0xff;
    for(int i=0 ; i<getTotalBands() ; i++)
    {
      if(band->value() != bands[i].bandName) continue;
      // Resolve duplicate band names using the frequency and mode. Check Hz
      // before converting to the receiver's narrower frequency representation.
      uint32_t unit = mem.mode == FM? 10000 : 1000;
      if(mem.freq && (hz < (uint32_t)bands[i].minimumFreq * unit ||
                     hz > (uint32_t)bands[i].maximumFreq * unit ||
                     !isMemoryInBand(&bands[i], &mem))) continue;
      mem.band = i;
      break;
    }
    if(mem.band == 0xff)
    {
      if(!mem.freq && !band->value().length()) mem.band = 0;
      else
      {
        error = "Frequency or mode does not match the band.";
        break;
      }
    }
  }

  if(error)
  {
    free(pending);
    return request->send(400, "text/html", webPage(pageMemoryError, pageMemoryTitle, {
      {"title", pageMemoryTitle},
      {"slot", String(slot + 1)}, {"error", error}
    }));
  }

  setMemories(pending);
  free(pending);
  prefsRequestSave(SAVE_MEMORIES, true);
  request->redirect("/memory");
}

static const String webMemoryPage()
{
  return webPage(pageMemory, pageMemoryTitle, {
    {"title", pageMemoryTitle}, {"toolbar", pageMemoryToolbar}
  }, {
    {"navigation", [](String &out) { webNavigation(out, "/memory"); }},
    {"rows", [](String &out) {
      for(int j=0 ; j<getTotalMemories() ; j++)
      {
        const Memory mem = getMemory(j);
        const char *band = mem.freq && mem.band<getTotalBands()? bands[mem.band].bandName : "";
        char slot[16];
        snprintf(slot, sizeof(slot), "%02d", j+1);
        pageAppend(out, pageMemoryRow, {
          {"index", String(j)}, {"slot", slot}, {"name", mem.name}, {"frequency", String(mem.freq)},
          {"up_disabled", j==0? "DISABLED" : ""},
          {"down_disabled", j==getTotalMemories()-1? "DISABLED" : ""}
        }, {
          {"bands", [&](String &out) {
            for(int i=0 ; i<getTotalBands() ; i++)
            {
              bool duplicate = false;
              for(int k=0 ; k<i ; k++)
                if(!strcmp(bands[k].bandName, bands[i].bandName)) duplicate = true;
              if(duplicate) continue;
              pageAppend(out, pageMemoryOption, {
                {"value", bands[i].bandName}, {"selected", !strcmp(band, bands[i].bandName)? "SELECTED" : ""}
              });
            }
          }},
          {"modes", [&](String &out) {
            for(int i=0 ; i<getTotalModes() ; i++)
              pageAppend(out, pageMemoryOption, {
                {"value", bandModeDesc[i]}, {"selected", mem.freq && mem.mode==i? "SELECTED" : ""}
              });
          }}
        });
      }
    }}
  });
}

const String webConfigPage()
{
  prefs.begin("network", true, STORAGE_PARTITION);
  String ssid1 = prefs.getString("wifissid1", "");
  String pass1 = prefs.getString("wifipass1", "");
  String ssid2 = prefs.getString("wifissid2", "");
  String pass2 = prefs.getString("wifipass2", "");
  String ssid3 = prefs.getString("wifissid3", "");
  String pass3 = prefs.getString("wifipass3", "");
  bool scanHidden = prefs.getBool("wifiscanhidden", false);
  prefs.end();

  String splashResolution = String(spr.width()) + "x" + String(spr.height());

  return webPage(pageConfig, pageConfigTitle, {
    {"title", pageConfigTitle},
    {"ssid1", ssid1}, {"pass1", pass1}, {"ssid2", ssid2}, {"pass2", pass2},
    {"ssid3", ssid3}, {"pass3", pass3}, {"username", loginUsername}, {"password", loginPassword},
    {"scan_hidden", scanHidden? "CHECKED" : ""},
    {"scroll", scrollDirection<0? "CHECKED" : ""},
    {"half_step", encoderHalfStep? "CHECKED" : ""}, {"zoom", zoomMenu? "CHECKED" : ""},
    {"resolution", splashResolution}
  }, {
    {"navigation", [](String &out) { webNavigation(out, "/config"); }},
    {"utc_options", [](String &out) {
      for(int i=0 ; i<getTotalUTCOffsets(); i++)
        pageAppend(out, pageConfigUtcOption, {
          {"value", String(i)}, {"minutes", String(utcOffsets[i].offset * 15)},
          {"selected", utcOffsetIdx==i? "SELECTED" : ""}, {"label", utcOffsets[i].desc}
        });
    }},
    {"theme_options", [](String &out) {
      for(int i=0 ; i<getTotalThemes(); i++)
        pageAppend(out, pageConfigThemeOption, {
          {"value", String(i)}, {"selected", themeIdx==i? "SELECTED" : ""}, {"label", theme[i].name}
        });
    }},
    {"splash", [](String &out) {
      if(LittleFS.exists(SPLASH_PATH)) pageAppend(out, pageSplash, {{"version", String(millis())}});
      else out += "Not installed";
    }}
  });
}

// Explicit request errors leave the active operation's status unchanged.
static void webUpdatePage(AsyncWebServerRequest *request, const OtaStatus &status, int code)
{
  const bool busy = status.phase == OTA_CHECK_QUEUED || status.phase == OTA_QUEUED ||
                    status.phase == OTA_CONNECTING || status.phase == OTA_WRITING;
  const bool complete = status.phase == OTA_COMPLETE || status.phase == OTA_REBOOT_PENDING;
  const bool available = status.phase == OTA_AVAILABLE;
  const char *refresh = complete? pageUpdateComplete : busy? pageUpdateBusy : "";
  const String page = webPage(pageUpdate, pageUpdateTitle, {
    {"title", pageUpdateTitle},
    {"message", status.message},
    {"action", available? "install" : "check"}, {"disabled", busy || complete? "DISABLED" : ""},
    {"button", available? "Update" : "Check for updates"}, {"refresh", refresh}
  }, {
    {"navigation", [](String &out) { webNavigation(out, "/update"); }}
  });
  if(!code) code = status.phase == OTA_FAILED? 400 : 200;
  AsyncWebServerResponse *response = request->beginResponse(code, "text/html", page);
  response->addHeader("Cache-Control", "no-store");
  response->addHeader("Connection", "close");
  request->send(response);
}

static PatchUpload *webPatchBegin(AsyncWebServerRequest *request)
{
  if(request->_tempObject) return static_cast<PatchUpload *>(request->_tempObject);
  PatchUpload *state = new(std::nothrow) PatchUpload();
  if(!state) return nullptr;
  request->_tempObject = state;
  request->onDisconnect([request, state]() {
    // Destroy the C++ upload context before the server frees _tempObject.
    request->_tempObject = nullptr;
    delete state;
  });
  if(!request->hasParam("slot", true) || !request->hasParam("action", true))
  {
    state->error = "Missing patch slot or action.";
    return state;
  }
  String slot = request->getParam("slot", true)->value();
  String action = request->getParam("action", true)->value();
  if(slot.length() != 1 || slot[0] < '0' || slot[0] > '3' ||
     action != "upload")
  {
    state->error = "Invalid patch slot or action.";
    return state;
  }
  uint8_t mode = PATCH_MODE_COUNT;
  if(request->hasParam("mode", true))
    for(uint8_t i = 0; i < PATCH_MODE_COUNT; i++)
      if(request->getParam("mode", true)->value() == patchModeNames[i]) mode = i;
  state->begin(slot[0] - '0', static_cast<PatchMode>(mode));
  return state;
}

static void webUploadPatch(AsyncWebServerRequest *request, const String &filename,
                           size_t index, uint8_t *data, size_t len, bool final)
{
  if(!webIsAuthenticated(request) || request->getResponse()) return;
  if(filename.isEmpty() && index == 0 && len == 0 && final) return;
  PatchUpload *state = webPatchBegin(request);
  if(!state) return request->send(503, "text/plain", "Not enough memory to process the patch set.");
  if(state->error) return;
  if(index == 0)
  {
    String extension = filename;
    extension.toLowerCase();
    if(!extension.endsWith(".bin"))
    {
      state->error = "Upload exactly one .bin patch file.";
      return;
    }
  }

  state->write(index, data, len, final);
}

static void webPatchesComplete(AsyncWebServerRequest *request)
{
  if(!webIsAuthenticated(request)) return request->requestAuthentication();
  if(request->getResponse()) return;
  if(request->hasParam("delete", true))
  {
    PatchUpload *state = static_cast<PatchUpload *>(request->_tempObject);
    if(state)
    {
      // Discard any uploaded file and release its reservation before deletion.
      state->error = "Upload discarded.";
      state->finish();
    }
    const char *error = nullptr;
    String slot = request->hasParam("slot", true)? request->getParam("slot", true)->value() : "";
    if(slot.length() != 1 || slot[0] < '1' || slot[0] > '3') error = "Invalid patch slot.";
    else patchesDelete(slot[0] - '0', error);
    if(error) return request->send(400, "text/plain", error);
    return request->redirect("/patches");
  }
  if(!request->_tempObject)
  {
    if(!request->hasParam("slot", true) || !request->hasParam("action", true))
      return request->send(400, "text/plain", "Missing patch slot or action.");
    String slot = request->getParam("slot", true)->value();
    String action = request->getParam("action", true)->value();
    const char *error = nullptr;
    if(slot.length() != 1 || slot[0] < '0' || slot[0] > '3') error = "Invalid patch slot.";
    else if(action == "select") patchesRequestSelection(slot[0] - '0', error);
    else error = "Upload one complete .bin patch.";
    if(error) return request->send(400, "text/plain", error);
    return request->redirect("/patches");
  }
  PatchUpload *state = static_cast<PatchUpload *>(request->_tempObject);
  state->finish();
  if(state->error)
    return request->send(400, "text/plain", state->error);
  request->redirect("/patches");
}

static void webPatchesPage(AsyncWebServerRequest *request)
{
  if(!webIsAuthenticated(request)) return request->requestAuthentication();
  bool busy;
  uint8_t selected;
  patchesSnapshot(selected, busy);
  const String page = webPage(pagePatches, pagePatchesTitle, {
    {"title", pagePatchesTitle}, {"busy", busy? pagePatchesBusy : ""},
    {"disabled", busy? "DISABLED" : ""}
  }, {
    {"navigation", [](String &out) { webNavigation(out, "/patches"); }},
    {"options", [&](String &out) {
      for(uint8_t slot = 0; slot <= PATCH_SET_COUNT; slot++)
      {
        if(slot && !patchesModes(slot)) continue;
        pageAppend(out, pagePatchOption, {
          {"value", String(slot)}, {"selected", slot == selected? "SELECTED" : ""}, {"label", patchSetNames[slot]}
        });
      }
    }},
    {"sets", [&](String &out) {
      for(uint8_t slot = 1; slot <= PATCH_SET_COUNT; slot++)
      {
        const char *disabled = busy || selected == slot? "DISABLED" : "";
        uint8_t modes = patchesModes(slot);
        String installed;
        for(uint8_t mode = 0; mode < PATCH_MODE_COUNT; mode++)
        {
          if(mode) installed += " · ";
          installed += patchModeNames[mode];
          installed += (modes & (1U << mode))? ": uploaded" : ": Default";
        }
        pageAppend(out, pagePatchSet, {
          {"slot", String(slot)}, {"name", patchSetNames[slot]}, {"installed", installed}, {"disabled", disabled}
        }, {
          {"options", [](String &out) {
            for(uint8_t mode = 0; mode < PATCH_MODE_COUNT; mode++)
              pageAppend(out, pagePatchOption, {
                {"value", patchModeNames[mode]}, {"selected", ""}, {"label", patchModeNames[mode]}
              });
          }},
          {"delete", [&](String &out) {
            if(modes) pageAppend(out, pagePatchDelete, {{"slot", String(slot)}, {"disabled", disabled}});
          }}
        });
      }
    }}
  });
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", page);
  response->addHeader("Cache-Control", "no-store");
  if(busy) response->addHeader("Refresh", "2; url=/patches");
  request->send(response);
}

// Run receiver commands and sweeps only from the main loop, never inside the async web task.
void webSpectrumTick()
{
  // Do not let commands accepted just before Dual RX starts execute later,
  // after Dual RX has stopped. Dual RX owns the receiver while active.
  if(currentCmd == CMD_DUAL_RX)
  {
    webTuneStep = 0;
    webRequestedVolume = -1;
    webTuneFrequencyHz = 0;
    webTuneRequestedMode = -1;
    webSpectrumRequested = false;
    webSpectrumCustomRange = false;
    webSpectrumRequestedMode = -1;
    webSpectrumStep = 1;
    webSpectrumBusy = false;
    return;
  }

  int8_t requestedStep = webTuneStep;
  if(requestedStep)
  {
    webTuneStep = 0;
    int step = getCurrentStep()->step;
    if(isSSB()) updateBFO(currentBFO + requestedStep * step, true);
    else updateFrequency(currentFrequency + requestedStep * step, true);
    clearStationInfo();
    identifyFrequency(currentFrequency + currentBFO / 1000);
  }
  int16_t requestedVolume = webRequestedVolume;
  if(requestedVolume >= 0)
  {
    webRequestedVolume = -1;
    volume = (uint8_t)requestedVolume;
    rx.setVolume(volume);
  }
  int32_t requestedHz = webTuneFrequencyHz;
  if(requestedHz > 0)
  {
    webTuneFrequencyHz = 0;
    int8_t requestedMode = webTuneRequestedMode;
    webTuneRequestedMode = -1;
    webTunePhase = 2;
    webDiagLog("TUNE START");
    if(requestedMode >= 0 && requestedMode != currentMode)
    {
      // Reconfigure the receiver only when the requested modulation actually
      // changes. Reapplying setSSB()/setSSBBfo() on an already active USB/LSB
      // mode can shift the displayed tuning by the current BFO offset.
      // If switching from the VHF/FM band to HF, move the band state to ALL
      // before configuring AM/SSB so band limits and calibration are correct.
      if(requestedMode != FM && bandIdx == 0)
      {
        webTunePhase = 3;
        { webDiagLog("TUNE before selectBand ALL"); selectBand(1, false); webDiagLog("TUNE after selectBand ALL"); }
      }

      webTunePhase = 4;
      webDiagLog("TUNE before receiver configuration");
      currentMode = requestedMode;
      uint16_t targetFreq = freqFromHz((uint32_t)requestedHz, currentMode);
      if(currentMode == FM)
        rx.setFM(6400, 10800, constrain(targetFreq, 6400, 10800), 10);
      else if(currentMode == AM)
        rx.setAM(100, 30000, constrain(targetFreq, 100, 30000), 1);
      else
      {
        rx.setSSB(100, 30000, constrain(targetFreq, 100, 30000), 0, currentMode);
        rx.setSSBConfig(getCurrentBandwidth()->idx, 1, 0, 1, 0, 1);
        Band *activeBand = getCurrentBand();
        if(currentMode == USB) rx.setSSBBfo(-(currentBFO + (activeBand ? activeBand->usbCal : 0)));
        else rx.setSSBBfo(-(currentBFO + (activeBand ? activeBand->lsbCal : 0)));
      }
      webDiagLog("TUNE receiver configuration returned");
      webTunePhase = 5;
      currentFrequency = freqFromHz((uint32_t)requestedHz, currentMode);
      Band *activeBand = getCurrentBand();
      if(activeBand) activeBand->currentFreq = currentFrequency;
    }
    else if(requestedMode >= 0)
    {
      // The requested mode is already active: tune only the frequency.
      // Avoid resetting the SSB BFO/calibration for a simple guide selection.
      uint16_t targetFreq = freqFromHz((uint32_t)requestedHz, currentMode);
      rx.setFrequency(targetFreq);
      currentFrequency = targetFreq;
      Band *activeBand = getCurrentBand();
      if(activeBand) activeBand->currentFreq = currentFrequency;
    }
    else
    {
      uint16_t targetFreq = freqFromHz((uint32_t)requestedHz, currentMode);
      uint32_t minFreq = currentMode == FM ? 6400UL : 150UL;
      uint32_t maxFreq = currentMode == FM ? 10800UL : 30000UL;
      if(targetFreq >= minFreq && targetFreq <= maxFreq) rx.setFrequency(targetFreq);
    }
    // After an explicit web mode configuration (FM/AM/SSB), the SI4735
    // getFrequency() call can stall when returning from FM to SSB. The
    // requested frequency was already supplied to setFM/setAM/setSSB above,
    // so retain that known target instead of querying the receiver here.
    // For frequency-only tunes, keep reading back the actual receiver value.
    if(requestedMode < 0)
    {
      webDiagLog("TUNE before final getFrequency");
      currentFrequency = rx.getFrequency();
      webDiagLog("TUNE after final getFrequency");
    }
    else
    {
      currentFrequency = freqFromHz((uint32_t)requestedHz, currentMode);
      webDiagLog("TUNE mode configured; using requested frequency");
    }
    Band *band = getCurrentBand();
    if(band) band->currentFreq = currentFrequency;
    webTunePhase = 6;
    webDiagLog("TUNE before station identification");
    clearStationInfo();
    identifyFrequency(currentFrequency + currentBFO / 1000);
    webDiagLog("TUNE after station identification");
    webTuneActualHz = (uint32_t)currentFrequency * (currentMode == FM ? 10000UL : 1000UL);
    webTuneActualMode = currentMode;
    uint32_t expectedHz = (uint32_t)freqFromHz((uint32_t)requestedHz, currentMode) *
                          (currentMode == FM ? 10000UL : 1000UL);
    webTuneResult = (webTuneActualMode == webTuneTargetMode &&
                     (webTuneActualHz > expectedHz ? webTuneActualHz - expectedHz : expectedHz - webTuneActualHz) <=
                     (currentMode == FM ? 10000UL : 1000UL)) ? 1 : 2;
    webTunePhase = 7;
    webTuneCompletedId = webTuneRequestId;
    webDiagLog("TUNE COMPLETE");
  }
  if(!webSpectrumRequested) return;
  webDiagLog("SPECTRUM request received");
  webSpectrumRequested = false;
  bool customRange = webSpectrumCustomRange;
  uint16_t rangeStart = webSpectrumStartFreq;
  uint16_t rangeEnd = webSpectrumEndFreq;
  int8_t requestedMode = webSpectrumRequestedMode;
  uint16_t customStep = webSpectrumStep;
  webSpectrumCustomRange = false;
  webSpectrumRequestedMode = -1;
  webSpectrumStep = 1;
  const uint16_t savedFrequency = currentFrequency;
  const int8_t savedMode = currentMode;
  const int16_t savedBFO = currentBFO;
  const uint8_t savedBandIdx = bandIdx;
  const int8_t scanMode = (customRange && requestedMode >= 0) ? requestedMode : currentMode;
  uint16_t center = currentFrequency;
  uint16_t step = currentMode == FM ? 10 : 1;

  webDiagLog("SPECTRUM preparation start");
  if(customRange && requestedMode >= 0)
  {
    webDiagLog("SPECTRUM before mode/band setup");
    // Enter ALL band before a non-FM sweep when the receiver was on VHF.
    // Changing currentMode alone leaves the band's limits and calibration on
    // VHF, which can break subsequent scans and tuning commands.
    if(requestedMode != FM && bandIdx == 0)
      selectBand(1, false);
    currentMode = requestedMode;
    if(currentMode == FM)
    {
      center = constrain(center, 6400, 10800);
      rx.setFM(6400, 10800, center, 10);
    }
    else if(currentMode == AM)
    {
      center = constrain(center, 150, 30000);
      rx.setAM(150, 30000, center, 1);
    }
    else
    {
      center = constrain(center, 150, 30000);
      rx.setSSB(150, 30000, center, 0, currentMode);
      rx.setSSBConfig(getCurrentBandwidth()->idx, 1, 0, 1, 0, 1);
      if(currentMode == USB) rx.setSSBBfo(-(currentBFO + getCurrentBand()->usbCal));
      else rx.setSSBBfo(-(currentBFO + getCurrentBand()->lsbCal));
    }
    // The receiver's blocking getFrequency() read can hang after an SSB
    // mode setup. setFM/setAM/setSSB already received this exact center
    // frequency, so use the configured target instead of querying the chip.
    currentFrequency = center;
    Band *setupBand = getCurrentBand();
    if(setupBand) setupBand->currentFreq = currentFrequency;
    webDiagLog("SPECTRUM setup frequency recorded; skipped getFrequency");
  }

  webDiagLog("SPECTRUM scan call start");
  if(customRange)
  {
    step = customStep;
    webDiagLog("SPECTRUM scanRunRange start");
    scanRunRange(rangeStart, rangeEnd, step);
    webDiagLog("SPECTRUM scanRunRange returned");
  }
  else {
    scanRun(center, currentMode == FM ? 10 : 5);
    webDiagLog("SPECTRUM scan returned");
  }

  webDiagLog("SPECTRUM scan result processing");
  webSpectrumResultMode = scanMode;
  webSpectrumReady = scanGetCount() > 0;

  if(customRange)
  {
    webDiagLog("SPECTRUM restore start");
    // Restore the original receiver state while retaining the scan modulation
    // separately, so selecting a peak can intentionally switch to that mode.
    // Restore the actual band as well as mode/frequency after every sweep.
    if(bandIdx != savedBandIdx)
    {
      webDiagLog("SPECTRUM restore selectBand start");
      selectBand(savedBandIdx, false);
      webDiagLog("SPECTRUM restore selectBand returned");
    }
    currentMode = savedMode;
    currentBFO = savedBFO;
    if(savedMode == FM)
      rx.setFM(6400, 10800, constrain(savedFrequency, 6400, 10800), 10);
    else if(savedMode == AM)
      rx.setAM(150, 30000, constrain(savedFrequency, 150, 30000), 1);
    else
    {
      rx.setSSB(150, 30000, constrain(savedFrequency, 150, 30000), 0, savedMode);
      rx.setSSBConfig(getCurrentBandwidth()->idx, 1, 0, 1, 0, 1);
      if(savedMode == USB) rx.setSSBBfo(-(currentBFO + getCurrentBand()->usbCal));
      else rx.setSSBBfo(-(currentBFO + getCurrentBand()->lsbCal));
    }
    // The restore calls above already commanded savedFrequency. Avoid a
    // blocking SI4735 getFrequency() read here as well, especially after SSB.
    currentFrequency = savedFrequency;
    Band *band = getCurrentBand();
    if(band) band->currentFreq = currentFrequency;
    webDiagLog("SPECTRUM restore frequency recorded; skipped getFrequency");
    clearStationInfo();
    webDiagLog("SPECTRUM restore identify start");
    identifyFrequency(currentFrequency + currentBFO / 1000);
    webDiagLog("SPECTRUM restore identify returned");
  }
  webSpectrumBusy = false;
  webDiagLog("SPECTRUM COMPLETE");
}
