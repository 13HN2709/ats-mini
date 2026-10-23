// =================================
// INCLUDE FILES
// =================================

#include "Common.h"
#include <Wire.h>
#include "Rotary.h"
#include "Button.h"
#include "Menu.h"
#include "Memories.h"
#include "Patches.h"
#include "Draw.h"
#include "Storage.h"
#include "Themes.h"
#include "Utils.h"
#include "EIBI.h"
#include "Remote.h"
#include "BleMode.h"
#include "TcpMode.h"
#include "Splash.h"
#include <stdlib.h>
#include <time.h>
#include "FreqKeypad.h"


// SI473/5 and UI
#define MIN_ELAPSED_TIME         5  // 300
#define MIN_ELAPSED_RSSI_TIME  200  // RSSI check uses IN_ELAPSED_RSSI_TIME * 6 = 1.2s
#define ELAPSED_COMMAND      10000  // time to turn off the last command controlled by encoder. Time to goes back to the VFO control // G8PTN: Increased time and corrected comment
#define DEFAULT_VOLUME          35  // change it for your favorite sound volume
#define DEFAULT_SLEEP            0  // Default sleep interval, range = 0 (off) to 255 in steps of 5
#define RDS_CHECK_TIME         250  // Increased from 90
#define SEEK_TIMEOUT        600000  // Max seek timeout (ms)
#define NTP_CHECK_TIME       60000  // NTP time refresh period (ms)
#define SCHEDULE_CHECK_TIME   2000  // How often to identify the same frequency (ms)
#define BACKGROUND_REFRESH_TIME 5000    // Background screen refresh time. Covers the situation where there are no other events causing a refresh

// =================================
// CONSTANTS AND VARIABLES
// =================================

int8_t agcIdx = 0;
uint8_t disableAgc = 0;
int8_t agcNdx = 0;
int8_t softMuteMaxAttIdx = 4;

volatile bool seekStop = false; // G8PTN: Added flag to abort seeking on rotary encoder detection
bool pushAndRotate = false;   // Push and rotate is active, ignore the long press

long elapsedRSSI = millis();
long elapsedButton = millis();

long lastStrengthCheck = millis();
long lastRDSCheck = millis();
long lastNTPCheck = millis();
long lastScheduleCheck = millis();

long elapsedCommand = millis();
volatile int16_t encoderCount = 0;
volatile int16_t encoderCountAccel = 0;
uint16_t currentFrequency;

// AGC/ATTN index per mode (FM/AM/SSB)
int8_t FmAgcIdx = 0;                    // Default FM  AGGON  : Range = 0 to 37, 0 = AGCON, 1 - 27 = ATTN 0 to 26
int8_t AmAgcIdx = 0;                    // Default AM  AGCON  : Range = 0 to 37, 0 = AGCON, 1 - 37 = ATTN 0 to 36
int8_t SsbAgcIdx = 0;                   // Default SSB AGCON  : Range = 0 to 1,  0 = AGCON,      1 = ATTN 0

// AVC index per mode (AM/SSB)
int8_t AmAvcIdx = 48;                   // Default AM  = 48 (as per AN332), range = 12 to 90 in steps of 2
int8_t SsbAvcIdx = 48;                  // Default SSB = 48, range = 12 to 90 in steps of 2
bool ssbAvcHold = false;                // Temporary; cleared when the band/mode is reinitialized

// SoftMute index per mode (AM/SSB)
int8_t AmSoftMuteIdx = 4;               // Default AM  = 4, range = 0 to 32
int8_t SsbSoftMuteIdx = 4;              // Default SSB = 4, range = 0 to 32

// Menu options
uint8_t volume = DEFAULT_VOLUME;        // Volume, range = 0 (muted) - 63
uint8_t currentSquelch[4] = {0};        // Squelch per mode: lower 7 bits = threshold, high bit selects SNR (1) vs RSSI (0)
uint8_t FmRegionIdx = 0;                // FM Region

uint16_t currentBrt = 130;              // Display brightness, range = 10 to 255 in steps of 5
uint16_t currentSleep = DEFAULT_SLEEP;  // Display sleep timeout, range = 0 to 255 in steps of 5
long elapsedSleep = millis();           // Display sleep timer
bool zoomMenu = false;                  // Display zoomed menu item
int8_t scrollDirection = 1;             // Menu scroll direction
#if defined(LILYGO_SI473X)
bool encoderHalfStep = true;             // T-Embed defaults to half-step decoding
#else
bool encoderHalfStep = false;            // Standard hardware defaults to full-step decoding
#endif

// Background screen refresh
uint32_t background_timer = millis();   // Background screen refresh timer.

//
// Current parameters
//
uint16_t currentCmd  = CMD_NONE;
uint8_t  currentMode = FM;
int16_t  currentBFO  = 0;

uint8_t  rssi = 0;
uint8_t  snr  = 0;

//
// Devices
//
Rotary encoder  = Rotary(ENCODER_PIN_B, ENCODER_PIN_A, encoderHalfStep);
ButtonTracker pb1 = ButtonTracker();
LGFX tft;
LGFX_Sprite spr(&tft);
SI4735_fixed rx;

// ============================================================================
// SKIZZEN-WASSERFALL (ECO-MODE) CONFIG & HELPERS (ORIGINAL BEHALTEN)
// ============================================================================
#define WATERFALL_HEIGHT  35
#define WATERFALL_WIDTH   (DISPLAY_WIDTH - 20) 
#define X_MID_START      100 
#define X_MID_WIDTH      60  

uint16_t getSketchedColor(uint8_t rssiVal) {
  if (rssiVal < 5)   return TFT_BLACK; 
  if (rssiVal < 20)  return 0xF800; // Rot   (TFT_RED)
  if (rssiVal < 30)  return 0xFFE0; // Gelb  (TFT_YELLOW)
  return 0x07E0;                    // Grün  (TFT_GREEN)
}

void drawEcoWaterfallStep(LGFX_Sprite &targetSprite, uint8_t currentRssi) {
    int wfX = 10; 
    int wfY = DISPLAY_HEIGHT - WATERFALL_HEIGHT - 5; 
    int xStart = wfX + X_MID_START;

    targetSprite.fillRect(xStart, wfY, X_MID_WIDTH, WATERFALL_HEIGHT, 0x0000); 
    
    uint16_t sigColor = getSketchedColor(currentRssi);
    int barH = map(currentRssi, 0, 45, 0, WATERFALL_HEIGHT);
    barH = constrain(barH, 0, WATERFALL_HEIGHT);
    
    targetSprite.fillRect(xStart + 10, wfY + WATERFALL_HEIGHT - barH, X_MID_WIDTH - 20, barH, sigColor);
}

void drawEcoWaterfallStatic(LGFX_Sprite &targetSprite) {
    int wfX = 10;
    int wfY = DISPLAY_HEIGHT - WATERFALL_HEIGHT - 5;

    targetSprite.drawRect(wfX, wfY, WATERFALL_WIDTH, WATERFALL_HEIGHT, 0xFFFF);
    targetSprite.fillRect(wfX, wfY, X_MID_START, WATERFALL_HEIGHT, 0x001F);
    targetSprite.fillRect(wfX + X_MID_START + X_MID_WIDTH, wfY, WATERFALL_WIDTH - (X_MID_START + X_MID_WIDTH), WATERFALL_HEIGHT, 0x001F);
}

//
// Hardware initialization and setup
//
void setup()
{
  setenv("TZ", "UTC0", 1);
  tzset();

  Serial.begin(115200);

  pinMode(ENCODER_PUSH_BUTTON, INPUT_PULLUP);
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  if(PIN_AMP_EN >= 0)
  {
    pinMode(PIN_AMP_EN, OUTPUT);
    digitalWrite(PIN_AMP_EN, LOW);
  }

  pinMode(AUDIO_MUTE, OUTPUT);
  digitalWrite(AUDIO_MUTE, HIGH);

  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  delay(100);

  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);

  ledcAttach(PIN_LCD_BL, 16000, 8);
  ledcWrite(PIN_LCD_BL, 0);

  tft.init();
  tft.setRotation(3);

  tft.fillScreen(TH.bg);
  spr.setPsram(true);
  spr.setColorDepth(16);
  spr.createSprite(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  spr.setTextDatum(MC_DATUM);
  spr.setFont(&lgfx::fonts::Orbitron_Light_24);
  spr.setTextColor(TH.text, TH.bg);

  if(digitalRead(ENCODER_PUSH_BUTTON)==LOW)
  {
    nvsErase();
    diskInit(true);

    ledcWrite(PIN_LCD_BL, 255);
    tft.setTextSize(2);
    tft.setTextColor(TH.text_warn, TH.bg);
    tft.println(getVersion(true));
    tft.println();
    tft.setTextColor(TH.text_warn, TH.bg);
    tft.print("Resetting Preferences");
    while(digitalRead(ENCODER_PUSH_BUTTON) == LOW) delay(100);
  }

  diskInit();

  if(!ESP.getPsramSize() || !memoriesInit()) {
    ledcWrite(PIN_LCD_BL, 255);
    tft.setTextSize(2);
    tft.setTextColor(TH.text_warn, TH.bg);
    tft.println("PSRAM not detected");
#ifdef CONFIG_SPIRAM_MODE_OCT
    tft.println("(try the QSPI f/w version)");
#else
    tft.println("(try the OSPI f/w version)");
#endif
  while(1);
  }

  rx.setI2CFastModeCustom(800000UL);

  int16_t si4735Addr = rx.getDeviceI2CAddress(RESET_PIN);
  if(!si4735Addr)
  {
    ledcWrite(PIN_LCD_BL, 255);
    tft.setTextSize(2);
    tft.setTextColor(TH.text_warn, TH.bg);
    tft.println("Si4732 not detected");
    while(1);
  }

  rx.setup(RESET_PIN, MW_BAND_TYPE);

  if(!prefsLoad(SAVE_SETTINGS|SAVE_VERIFY))
  {
    prefsSave(SAVE_SETTINGS);
    spr.fillSprite(TH.bg);
    ledcWrite(PIN_LCD_BL, currentBrt);
    drawAboutHelp(0);
    while(digitalRead(ENCODER_PUSH_BUTTON)!=LOW) delay(100);
    while(digitalRead(ENCODER_PUSH_BUTTON)==LOW) delay(100);
  }

  if(!prefsLoad(SAVE_MEMORIES|SAVE_VERIFY)) prefsSave(SAVE_MEMORIES);
  if(!prefsLoad(SAVE_BANDS|SAVE_VERIFY)) prefsSave(SAVE_BANDS);

  if(PIN_AMP_EN >= 0) digitalWrite(PIN_AMP_EN, HIGH);

  patchesInit();

  initVfo();
  delay(50);
  rx.setVolume(volume);
  rx.setMaxSeekTime(SEEK_TIMEOUT);

  if(splashDraw())
  {
    ledcWrite(PIN_LCD_BL, currentBrt);
    delay(3000);
  }
  drawScreen();
  ledcWrite(PIN_LCD_BL, currentBrt);

  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  netInit(wifiModeIdx);
  bleInit(bleModeIdx);
}


int16_t accelerateEncoder(int8_t dir)
{
  const uint32_t speedThresholds[] = {350, 60, 45, 35, 25};
  const uint16_t accelFactors[] =      {1,  2,  4,  8, 16};
  static uint32_t lastEncoderTime = 0;
  static uint32_t lastSpeed = speedThresholds[0];
  static uint16_t lastAccelFactor = accelFactors[0];
  static int8_t lastEncoderDir = 0;

  uint32_t currentTime = millis();
  lastSpeed = ((currentTime - lastEncoderTime) * 7 + lastSpeed * 3) / 10;

  if (lastSpeed > speedThresholds[0] || lastEncoderDir != dir) {
    lastSpeed = speedThresholds[0];
    lastAccelFactor = accelFactors[0];
  } else {
    for (int8_t i = LAST_ITEM(speedThresholds); i >= 0; i--) {
      if (lastSpeed <= speedThresholds[i] && lastAccelFactor < accelFactors[i]) {
        lastAccelFactor = accelFactors[i];
        break;
      }
    }
  }
  lastEncoderTime = currentTime;
  lastEncoderDir = dir;

  return(dir * lastAccelFactor);
}

ICACHE_RAM_ATTR void rotaryEncoder()
{
  uint8_t encoderStatus = encoder.process();
  if(encoderStatus)
  {
    int8_t delta = encoderStatus==DIR_CW? 1 : -1;
    int16_t accelDelta = accelerateEncoder(delta);

    if(abs(encoderCount) < 5)
    {
      encoderCount += delta;
      encoderCountAccel += accelDelta;
    }

    seekStop = true;
  }
}

void setEncoderHalfStep(bool enabled)
{
  noInterrupts();
  encoder.setHalfStep(enabled);
  encoderHalfStep = enabled;
  encoderCount = 0;
  encoderCountAccel = 0;
  interrupts();
}

uint32_t consumeEncoderCounts()
{
  int16_t encCount, encCountAccel;
  noInterrupts();
  encCount = encoderCount;
  encCountAccel = encoderCountAccel;
  encoderCount = 0;
  encoderCountAccel = 0;
  interrupts();
  return ((uint32_t)encCountAccel << 16) | ((uint16_t)encCount & 0xFFFF);
}

//
// Switch radio to given band (Original)
//
void useBand(const Band *band)
{
  currentFrequency = band->currentFreq;
  currentMode = band->bandMode;
  currentBFO = 0;
  ssbAvcHold = false;

  if(band->bandMode==FM)
  {
    rx.setFM(band->minimumFreq, band->maximumFreq, band->currentFreq, getCurrentStep()->step);
    rx.setTuneFrequencyAntennaCapacitor(1); 
    rx.setSeekFmLimits(band->minimumFreq, band->maximumFreq);
    rx.setSeekFmRssiThreshold(0);
    rx.setSeekFmSNRThreshold(1);
    rx.setFMDeEmphasis(fmRegions[FmRegionIdx].value);
    applyFmStereo();
    rx.RdsInit();
    rx.setRdsConfig(1, 2, 2, 2, 2);
    rx.setGpioCtl(1, 0, 0);
    rx.setGpio(0, 0, 0);
  }
  else
  {
    if(band->bandMode==AM)
    {
      rx.setAM(band->minimumFreq, band->maximumFreq, band->currentFreq, getCurrentStep()->step);
      rx.setSeekAmRssiThreshold(10);
      rx.setSeekAmSNRThreshold(3);
    }
    else
    {
      rx.setSSB(band->minimumFreq, band->maximumFreq, band->currentFreq, 0, currentMode);
      rx.setSSBConfig(getCurrentBandwidth()->idx, 1, 0, 1, 0, 1);
      if (currentMode == USB)
        rx.setSSBBfo(-(currentBFO + band->usbCal));
      else if (currentMode == LSB)
        rx.setSSBBfo(-(currentBFO + band->lsbCal));
      else
        rx.setSSBBfo(-currentBFO);
    }

    rx.setGpioCtl(1, 0, 0);
    rx.setGpio(1, 0, 0);
    rx.setSeekAmLimits(band->minimumFreq, band->maximumFreq);
  }

  doStep(0);
  doSoftMute(0);
  doAgc(0);
  doAvc(0);
  delay(100);
  rssi = 0;
  snr  = 0;
}

//
// Tune using BFO (Original ohne Auto-Mode Bug)
//
bool updateBFO(int newBFO, bool wrap)
{
  Band *band = getCurrentBand();
  int newFreq = currentFrequency;

  if(!isSSB()) newBFO = 0;

  if(newBFO > MAX_BFO || newBFO < -MAX_BFO)
  {
    int fCorrect = (newBFO / MAX_BFO) * MAX_BFO;
    newFreq += fCorrect / 1000;
    newBFO  -= fCorrect;
  }

  int f = newFreq * 1000 + newBFO;
  if(f < band->minimumFreq * 1000)
  {
    if(!wrap) return false;
    newFreq = band->maximumFreq;
    newBFO  = 0;
  }
  else if(f > band->maximumFreq * 1000)
  {
    if(!wrap) return false;
    newFreq = band->minimumFreq;
    newBFO  = 0;
  }

  if(newFreq != currentFrequency)
  {
    rx.setFrequency(newFreq);
    doAgc(0);
    currentFrequency = rx.getFrequency();
  }

  currentBFO = newBFO;

  if (currentMode == USB)
    rx.setSSBBfo(-(currentBFO + band->usbCal));
  else if (currentMode == LSB)
    rx.setSSBBfo(-(currentBFO + band->lsbCal));
  else
    rx.setSSBBfo(-currentBFO);

  band->currentFreq = currentFrequency + currentBFO / 1000;
  return true;
}

//
// Tune to a new frequency (Original)
//
bool updateFrequency(int newFreq, bool wrap)
{
  Band *band = getCurrentBand();

  if(newFreq < band->minimumFreq)
  {
    if(!wrap) return false; else newFreq = band->maximumFreq;
  }
  else if(newFreq > band->maximumFreq)
  {
    if(!wrap) return false; else newFreq = band->minimumFreq;
  }

  rx.setFrequency(newFreq);

  if(currentBFO) updateBFO(0, true);

  currentFrequency = rx.getFrequency();
  band->currentFreq = currentFrequency + currentBFO / 1000;
  return true;
}

bool consumeAbortPending()
{
  if(seekStop)
  {
    seekStop = false;
    return true;
  }
  if(bleConsumeAbortPending(bleModeIdx)) return true;
  if(serialConsumeAbortPending(usbModeIdx)) return true;
  if(tcpConsumeAbortPending(tcpModeIdx)) return true;

  if(pb1.update(digitalRead(ENCODER_PUSH_BUTTON) == LOW, 0).isPressed)
  {
    while(pb1.update(digitalRead(ENCODER_PUSH_BUTTON) == LOW).isPressed)
      delay(100);
    return true;
  }

  return false;
}

void showFrequencySeek(uint16_t freq)
{
  currentFrequency = freq;
  drawScreen();
}

bool doSeek(int16_t enc, int16_t enca)
{
  muteOn(MUTE_TEMP, true);
  if(seekMode() == SEEK_DEFAULT)
  {
    if(isSSB())
    {
      updateBFO(currentBFO + enca * getCurrentStep()->step, true);
    }
    else
    {
      clearStationInfo();
      rssi = snr = 0;
      consumeAbortPending();
      rx.seekStationProgress(showFrequencySeek, consumeAbortPending, enc>0? 1 : 0);
      updateFrequency(rx.getFrequency(), true);
    }
  }
  else if(seekMode() == SEEK_SCHEDULE && enc)
  {
    uint8_t hour, minute;
    clockGetHM(&hour, &minute);

    size_t offset = -1;
    const StationSchedule *schedule = enc > 0 ?
      eibiNext(currentFrequency + currentBFO / 1000, hour, minute, &offset) :
      eibiPrev(currentFrequency + currentBFO / 1000, hour, minute, &offset);

    if(schedule) updateFrequency(schedule->freq, false);
  }

  clearStationInfo();
  identifyFrequency(currentFrequency + currentBFO / 1000);
  muteOn(MUTE_TEMP, false);
  return(true);
}

bool doTune(int16_t enc)
{
  if(isSSB())
  {
    uint32_t step = getCurrentStep()->step;
    uint32_t stepAdjust = (currentFrequency * 1000 + currentBFO) % step;
    step = !stepAdjust? step : enc>0? step - stepAdjust : stepAdjust;

    updateBFO(currentBFO + enc * step, true);
  }
  else
  {
    uint16_t step = getCurrentStep()->step;
    uint16_t stepAdjust = currentFrequency % step;
    stepAdjust = (currentMode==FM) && (step==20)? (stepAdjust+10) % step : stepAdjust;
    step = !stepAdjust? step : enc>0? step - stepAdjust : stepAdjust;

    updateFrequency(currentFrequency + step * enc, true);
  }

  clearStationInfo();
  identifyFrequency(currentFrequency + currentBFO / 1000);
  return(true);
}

bool doDigit(int16_t enc)
{
  bool updated = false;

  if(isSSB())
  {
    updated = updateBFO(currentBFO + enc * getFreqInputStep(), false);
  }
  else
  {
    updated = updateFrequency(currentFrequency + enc * getFreqInputStep(), false);
  }

  if (updated) {
    clearStationInfo();
    identifyFrequency(currentFrequency + currentBFO / 1000);
  }

  return(updated);
}

bool clickFreq(bool shortPress)
{
  if (shortPress) {
    bool updated = false;

     if(isSSB()) {
       updated = updateBFO(currentBFO - (currentFrequency * 1000 + currentBFO) % getFreqInputStep(), false);
     } else {
       updated = updateFrequency(currentFrequency - currentFrequency % getFreqInputStep(), false);
     }

     if (updated) {
       clearStationInfo();
       identifyFrequency(currentFrequency + currentBFO / 1000);
     }
     return true;
  }
  return false;
}

bool processRssiSnr()
{
  static uint32_t updateCounter = 0;
  bool needRedraw = false;

  rx.getCurrentReceivedSignalQuality();
  int newRSSI = rx.getCurrentRSSI();
  int newSNR = rx.getCurrentSNR();

  uint8_t squelchValue = currentSquelch[currentMode] & 0x7f;
  uint8_t squelchParam = (currentSquelch[currentMode] & 0x80)? newSNR:newRSSI;
  if(squelchValue)
  {
    if(squelchParam >= squelchValue && muteOn(MUTE_SQUELCH))
    {
      muteOn(MUTE_SQUELCH, false);
    }
    else if(squelchParam < squelchValue && !muteOn(MUTE_SQUELCH))
    {
      muteOn(MUTE_SQUELCH, true);
    }
  }
  else if(muteOn(MUTE_SQUELCH))
  {
    muteOn(MUTE_SQUELCH, false);
  }

  if(!(updateCounter++ & 7))
  {
    if(newRSSI != rssi)
    {
      rssi = newRSSI;
      needRedraw = true;
    }
    if(newSNR != snr)
    {
      snr = newSNR;
      needRedraw = true;
    }
  }
  
  // Eco-Waterfall beibehalten
  drawEcoWaterfallStep(spr, rssi);

  return needRedraw;
}

void loop()
{
  uint32_t currentTime = millis();
  // Remember the last band and frequency used for frequency-dependent layout hints.
  static uint32_t lastBandplanDisplayFrequency = UINT32_MAX;
  static const Band *lastBandplanDisplayBand = nullptr;
  bool needRedraw = false;

  uint32_t encCounts = consumeEncoderCounts();
  int16_t encCount = (int16_t)(encCounts & 0xFFFF);
  int16_t encCountAccel = (int16_t)(encCounts >> 16);

  ButtonTracker::State pb1st = pb1.update(digitalRead(ENCODER_PUSH_BUTTON) == LOW);

  if(currentCmd == CMD_DUAL_RX && dualRxTick(currentTime)) needRedraw = true;

  int ser_event = serialLoop(usbModeIdx);
  needRedraw |= !!(ser_event & REMOTE_CHANGED);
  pb1st.isPressed |= !!(ser_event & REMOTE_PRESSED);
  pb1st.wasClicked |= !!(ser_event & REMOTE_CLICK);
  pb1st.wasShortPressed |= !!(ser_event & REMOTE_SHORT_PRESS);
  int ser_direction = ser_event >> REMOTE_DIRECTION;
  encCount = ser_direction? ser_direction : encCount;
  encCountAccel = ser_direction? ser_direction : encCountAccel;
  if(ser_event & REMOTE_PREFS) prefsRequestSave(SAVE_ALL);

  int ble_event = bleLoop(bleModeIdx);
  needRedraw |= !!(ble_event & REMOTE_CHANGED);
  pb1st.isPressed |= !!(ble_event & REMOTE_PRESSED);
  pb1st.wasClicked |= !!(ble_event & REMOTE_CLICK);
  pb1st.wasShortPressed |= !!(ble_event & REMOTE_SHORT_PRESS);
  int ble_direction = ble_event >> REMOTE_DIRECTION;
  encCount = ble_direction? ble_direction : encCount;
  encCountAccel = ble_direction? ble_direction : encCountAccel;
  if(ble_event & REMOTE_PREFS) prefsRequestSave(SAVE_ALL);

  int tcp_event = tcpLoop(tcpModeIdx);
  needRedraw |= !!(tcp_event & REMOTE_CHANGED);
  pb1st.isPressed |= !!(tcp_event & REMOTE_PRESSED);
  pb1st.wasClicked |= !!(tcp_event & REMOTE_CLICK);
  pb1st.wasShortPressed |= !!(tcp_event & REMOTE_SHORT_PRESS);
  int tcp_direction = tcp_event >> REMOTE_DIRECTION;
  encCount = tcp_direction? tcp_direction : encCount;
  encCountAccel = tcp_direction? tcp_direction : encCountAccel;
  if(tcp_event & REMOTE_PREFS) prefsRequestSave(SAVE_ALL);

  if(encCount && sleepOn() && sleepModeIdx==SLEEP_LOCKED) encCount = encCountAccel = 0;

  if (encCount && pb1st.isPressed) pushAndRotate = true;

  if(!pb1st.isPressed && pushAndRotate)
  {
    pushAndRotate = false;
    needRedraw = true;
  }

  if(currentCmd == CMD_DUAL_RX)
  {
    // Handle a long press only once per physical press. ButtonTracker reports
    // isLongPressed continuously while the encoder remains held.
    static bool dualRxLongPressHandled = false;
    if(!pb1st.isPressed) dualRxLongPressHandled = false;

    if(pb1st.isLongPressed && !dualRxLongPressHandled)
    {
      dualRxLongPressHandled = true;
      // Long press only dims/wakes the display; Dual RX keeps scanning in the background.
      sleepOn(!sleepOn());
      elapsedSleep = elapsedCommand = currentTime = millis();
      pb1st.wasClicked = false;
      pb1st.wasShortPressed = false;
      needRedraw = true;
    }
    else if(pb1st.wasClicked || pb1st.wasShortPressed)
    {
      if(dualRxSetupStage == 1 || dualRxSetupStage == 2)
      {
        currentCmd = CMD_FREQ_KEYPAD;
        freqKeypadReset();
      }
      else if(dualRxSetupStage == 3) dualRxBeginScan();
      else if(dualRxSetupStage == 4)
      {
        // A short click stops the scan and wakes the display if it was dimmed.
        if(sleepOn()) sleepOn(false);
        dualRxStop();
      }
      needRedraw = true;
      elapsedCommand = millis();
      pb1st.wasClicked = pb1st.wasShortPressed = pb1st.isLongPressed = false;
    }
    encCount = encCountAccel = 0;
  }

  if(currentCmd != CMD_DUAL_RX && pushAndRotate)
  {
    if(encCount)
    {
      switch(currentCmd)
      {
        case CMD_NONE:
          currentCmd = CMD_FREQ_KEYPAD;
          freqKeypadReset();
          needRedraw = true;
          break;
        case CMD_FREQ:
          doSelectDigit(encCount);
          needRedraw = true;
          break;
        case CMD_SEEK:
          needRedraw |= doTune(encCount);
          prefsRequestSave(SAVE_CUR_BAND);
          break;
      }
    }
    elapsedSleep = elapsedCommand = currentTime;
  }
  else if(currentCmd != CMD_DUAL_RX)
  {
    if(encCount)
    {
      switch(currentCmd)
      {
        case CMD_NONE:
        case CMD_SCAN:
          needRedraw |= doTune(encCountAccel);
          prefsRequestSave(SAVE_CUR_BAND);
          break;
        case CMD_FREQ_KEYPAD:
          doFreqKeypad(encCount);
          needRedraw = true;
          break;
        case CMD_FREQ:
          needRedraw |= doDigit(encCount);
          prefsRequestSave(SAVE_CUR_BAND);
          break;
        case CMD_SEEK:
          needRedraw |= doSeek(encCount, encCountAccel);
          currentTime = millis();
          prefsRequestSave(SAVE_CUR_BAND);
          break;
        default:
          needRedraw |= doSideBar(currentCmd, encCount, encCountAccel);
          prefsRequestSave(SAVE_ALL);
          break;
      }

      elapsedSleep = elapsedCommand = currentTime;
    }
    else if(currentCmd == CMD_FREQ_KEYPAD && pb1st.isLongPressed)
    {
      // Cancelling keypad input during DUAL RX setup must also restore the
      // radio state that was active before entering the setup wizard.
      if(dualRxSetupStage == 1 || dualRxSetupStage == 2)
        dualRxStop();
      else
        currentCmd = CMD_NONE;
      needRedraw = true;
    }
    else if(pb1st.isLongPressed)
    {
      sleepOn(!sleepOn());
      elapsedSleep = elapsedCommand = currentTime = millis();
    }
    else if(currentCmd == CMD_FREQ_KEYPAD && (pb1st.wasClicked || pb1st.wasShortPressed))
    {
      clickFreqKeypad(true);
      needRedraw = true;
    }
    else if(pb1st.wasClicked || pb1st.wasShortPressed)
    {
      elapsedSleep = elapsedCommand = currentTime;

      if(sleepOn())
      {
        if(currentSleep)
        {
          sleepOn(false);
          needRedraw = true;
        }
        else if(sleepModeIdx == SLEEP_UNLOCKED)
        {
          if(pb1st.wasShortPressed && currentCmd==CMD_NONE)
            currentCmd = CMD_VOLUME;
          else if(currentCmd==CMD_VOLUME)
            clickHandler(currentCmd, pb1st.wasShortPressed);

          needRedraw = true;
        }
      }
      else if(clickHandler(currentCmd, pb1st.wasShortPressed))
      {
        needRedraw = true;
        elapsedSleep = elapsedCommand = currentTime = millis();
      }
      else if(currentCmd != CMD_NONE)
      {
        currentCmd = CMD_NONE;
        needRedraw = true;
      }
      else if(pb1st.wasShortPressed)
      {
        currentCmd = CMD_VOLUME;
        needRedraw = true;
      }
      else
      {
        currentCmd = CMD_MENU;
        needRedraw = true;
      }
    }
  }

  if((currentTime - elapsedCommand) > ELAPSED_COMMAND)
  {
    if(currentCmd != CMD_NONE && currentCmd != CMD_SEEK && currentCmd != CMD_SCAN && currentCmd != CMD_MEMORY && currentCmd != CMD_DUAL_RX)
    {
      currentCmd = CMD_NONE;
      needRedraw = true;
    }

    elapsedCommand = currentTime;
  }

  if(currentSleep && !sleepOn() && ((currentTime - elapsedSleep) > currentSleep * 1000))
  {
    sleepOn(true);
    elapsedSleep = elapsedCommand = currentTime = millis();
  }

  if((currentTime - elapsedRSSI) > MIN_ELAPSED_RSSI_TIME)
  {
    needRedraw |= processRssiSnr();
    elapsedRSSI = currentTime;
  }

  if((currentTime - lastRDSCheck) > RDS_CHECK_TIME)
  {
    needRedraw |= (currentMode == FM) && checkRds();
    lastRDSCheck = currentTime;
  }

  if((currentTime - lastScheduleCheck) > SCHEDULE_CHECK_TIME)
  {
    needRedraw |= identifyFrequency(currentFrequency + currentBFO / 1000, true);
    lastScheduleCheck = currentTime;
  }

  // Refresh frequency-dependent hints immediately after tuning changes.
  const uint32_t bandplanDisplayFrequency =
    currentFrequency + currentBFO / 1000;
  const Band *bandplanDisplayBand = getCurrentBand();
  if(bandplanDisplayFrequency != lastBandplanDisplayFrequency ||
     bandplanDisplayBand != lastBandplanDisplayBand)
  {
    lastBandplanDisplayFrequency = bandplanDisplayFrequency;
    lastBandplanDisplayBand = bandplanDisplayBand;
    needRedraw = true;
  }

  if((currentTime - lastNTPCheck) > NTP_CHECK_TIME)
  {
    needRedraw |= ntpSyncTime();
    lastNTPCheck = currentTime;
  }

  prefsTickTime();
  netTickTime();
  webSpectrumTick();
  needRedraw |= patchesTick();
  needRedraw |= clockUpdate();

  if(needRedraw) background_timer = currentTime;
  if((currentTime - background_timer) > BACKGROUND_REFRESH_TIME)
  {
    if(currentCmd == CMD_NONE) needRedraw = true;
    background_timer = currentTime;
  }

  needRedraw |= statusTick(millis());

  if(needRedraw) {
    drawScreen();
    // Statischer Rahmen und blaue Seiten des Eco-Wasserfalls beim Redraw
    drawEcoWaterfallStatic(spr);
  }

  delay(5);
}