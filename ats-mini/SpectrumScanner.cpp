#include "Common.h"
#include "Menu.h"
#include "Themes.h"
#include "Draw.h"
#include "SpectrumScanner.h"
#include <stdlib.h>
#include <string.h>

// Low-level scan engine shared with web Spectrum; scanner UI/input state is isolated.
void scanRunRange(uint16_t startFreq, uint16_t endFreq, uint16_t step);
uint16_t scanGetCount();
bool scanGetPoint(uint16_t index, uint16_t *freq, uint8_t *rssi, uint8_t *snr);

static constexpr uint16_t VHF_MIN = 8800;   // 88.00 MHz, 10 kHz units
static constexpr uint16_t VHF_MAX = 10800;  // 108.00 MHz
static constexpr uint16_t VHF_STEP = 10;    // 100 kHz
static uint16_t selectedPoint = 0;
static uint16_t scanStart = VHF_MIN;
static uint16_t scanEnd = VHF_MAX;
static bool spectrumHasData = false;

// Scanner-owned keypad state. FreqKeypad.cpp and Dual RX are not used or modified.
static const char *const spectrumKeys[16] = {
  "1", "2", "3", "BS", "4", "5", "6", "0",
  "7", "8", "9", ".", "OK", "MHz", "", "CANCEL"
};
static char spectrumInput[2][12] = {"", ""};
static uint8_t spectrumInputStage = 0;
static uint8_t spectrumKeyIndex = 5;
static bool spectrumInputError = false;
static bool spectrumScanning = false;

void spectrumScannerStart()
{
  spectrumInput[0][0] = spectrumInput[1][0] = '\0';
  spectrumInputStage = 0;
  spectrumKeyIndex = 5;
  spectrumInputError = false;
  spectrumHasData = false;
  spectrumScanning = false;
  currentCmd = CMD_SPECTRUM_INPUT;
}

bool spectrumScannerInputRotate(int16_t delta)
{
  if(!delta) return false;
  int next = (int)spectrumKeyIndex + (delta > 0 ? 1 : -1);
  do {
    if(next < 0) next = 15;
    if(next > 15) next = 0;
  } while(next == 14);
  spectrumKeyIndex = (uint8_t)next;
  return true;
}

static bool spectrumParseFrequency(uint16_t *out)
{
  const char *src = spectrumInput[spectrumInputStage];
  if(!src[0]) return false;
  char normalized[12];
  size_t n = strlen(src);
  if(n >= sizeof(normalized)) return false;
  for(size_t i=0;i<=n;i++) normalized[i] = src[i] == ',' ? '.' : src[i];
  char *end = nullptr;
  float mhz = strtof(normalized, &end);
  if(end == normalized || *end != '\0' || mhz < 88.0f || mhz > 108.0f) return false;
  long units = (long)(mhz * 100.0f + 0.5f);
  if(units < VHF_MIN || units > VHF_MAX) return false;
  *out = (uint16_t)units;
  return true;
}

void spectrumScannerInputCancel()
{
  spectrumInputError = false;
  spectrumScanning = false;
  currentCmd = CMD_SPECTRUM_MENU;
}

bool spectrumScannerInputClick(bool shortPress)
{
  if(!shortPress) { spectrumScannerInputCancel(); return true; }
  const char *key = spectrumKeys[spectrumKeyIndex];
  char *value = spectrumInput[spectrumInputStage];
  size_t len = strlen(value);
  spectrumInputError = false;

  if(!strcmp(key, "CANCEL")) { spectrumScannerInputCancel(); return true; }
  if(!strcmp(key, "BS")) {
    if(len) value[len-1] = '\0';
    return true;
  }
  if(!strcmp(key, "OK")) {
    uint16_t parsed = 0;
    if(!spectrumParseFrequency(&parsed)) { spectrumInputError = true; return true; }
    if(spectrumInputStage == 0) {
      scanStart = parsed;
      spectrumInputStage = 1;
      spectrumKeyIndex = 5;
      return true;
    }
    scanEnd = parsed;
    if(scanEnd <= scanStart) { spectrumInputError = true; return true; }

    // Inputs are validated before invoking the shared sweep engine.
    currentCmd = CMD_SPECTRUM;
    spectrumScanning = true;
    spectrumHasData = false;
    selectedPoint = 0;
    drawSpectrumScannerInput();
    spr.pushSprite(0, 0);
    scanRunRange(scanStart, scanEnd, VHF_STEP);
    spectrumHasData = scanGetCount() > 0;
    spectrumScanning = false;
    if(spectrumHasData) selectedPoint = scanGetCount() / 2;
    return true;
  }
  if(!strcmp(key, "MHz")) return true; // fixed unit for VHF input
  if(!strcmp(key, ".") && strchr(value, '.')) return true;
  if(len < sizeof(spectrumInput[0])-1 && (key[0] >= '0' && key[0] <= '9' || !strcmp(key, "."))) {
    value[len] = key[0];
    value[len+1] = '\0';
  }
  return true;
}

void drawSpectrumScannerInput()
{
  const uint16_t bg=0x0208, panel=0x030D, blue=0x167F, cyan=0x07FF, green=0x07E0, red=0xF800;
  spr.fillSprite(bg);
  spr.fillRoundRect(3,3,314,164,5,panel);
  spr.drawRoundRect(3,3,314,164,5,blue);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(cyan);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString(spectrumScanning ? "SCANNING SPECTRUM" : (spectrumInputStage == 0 ? "VHF START FREQUENCY" : "VHF END FREQUENCY"),160,6);
  spr.setTextDatum(MC_DATUM);
  char display[32];
  snprintf(display,sizeof(display),"%s MHz",spectrumInput[spectrumInputStage][0] ? spectrumInput[spectrumInputStage] : "--.--");
  spr.setTextColor(spectrumInputError ? red : green);
  spr.drawRoundRect(22,25,276,27,3,spectrumInputError ? red : blue);
  spr.drawString(display,160,38);

  for(int i=0;i<16;i++) {
    if(i==14) continue;
    int x=12+(i%4)*76, y=58+(i/4)*24;
    bool sel=(i==spectrumKeyIndex);
    if(sel) spr.fillRoundRect(x,y,70,21,3,cyan);
    else spr.fillRoundRect(x,y,70,21,3,0x0841);
    spr.drawRoundRect(x,y,70,21,3,blue);
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(sel?0x0000:cyan);
    spr.setFont(FONT_DEFAULT);
    spr.drawString(spectrumKeys[i],x+35,y+10);
  }
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(spectrumInputError ? red : cyan);
  spr.setFont(FONT_SMALL);
  spr.drawString(spectrumInputError ? "Invalid frequency / end must be higher" : "Turn: select   Click: enter   Long click: cancel",160,158);
}

void spectrumScannerRotate(int16_t delta)
{
  const uint16_t count = scanGetCount();
  if(!spectrumHasData || !count || !delta) return;
  int32_t next = (int32_t)selectedPoint + delta;
  while(next < 0) next += count;
  selectedPoint = (uint16_t)(next % count);
}

void spectrumScannerSelect()
{
  if(spectrumHasData) {
    uint16_t freq=0;
    if(scanGetPoint(selectedPoint,&freq,nullptr,nullptr)) {
      // Only the accepted point tunes the radio; browsing the graph does not.
      if(bandIdx != 0) selectBand(0,false);
      currentMode = FM;
      updateFrequency(freq,false);
    }
  }
  currentCmd = CMD_NONE;
}

void drawSpectrumScanner()
{
  const uint16_t bg = 0x0208;
  const uint16_t panel = 0x030D;
  const uint16_t grid = 0x123A;
  const uint16_t cyan = 0x07FF;
  const uint16_t blue = 0x167F;
  const uint16_t green = 0x07E0;
  const uint16_t white = 0xFFFF;
  const uint16_t muted = 0x7192;

  spr.fillSprite(bg);
  spr.fillRoundRect(3, 3, 314, 164, 5, panel);
  spr.drawRoundRect(3, 3, 314, 164, 5, blue);

  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(cyan, panel);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString("VHF SPECTRUM", 160, 7);

  uint16_t selectedFreq = scanStart;
  if(spectrumHasData)
    scanGetPoint(selectedPoint, &selectedFreq, nullptr, nullptr);

  char freqLabel[24];
  snprintf(freqLabel, sizeof(freqLabel), "%u.%02u MHz",
           (unsigned)(selectedFreq / 100), (unsigned)(selectedFreq % 100));
  spr.setTextColor(green, panel);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString(freqLabel, 160, 25);

  const int gx = 12, gy = 48, gw = 296, gh = 91;
  spr.drawRect(gx, gy, gw, gh, blue);
  for(int i = 1; i < 5; ++i)
  {
    int y = gy + (gh * i) / 5;
    for(int x = gx + 2; x < gx + gw; x += 5) spr.drawPixel(x, y, grid);
  }
  for(int i = 1; i < 10; ++i)
  {
    int x = gx + (gw * i) / 10;
    for(int y = gy + 2; y < gy + gh; y += 5) spr.drawPixel(x, y, grid);
  }

  const uint16_t count = scanGetCount();
  if(spectrumHasData && count > 1)
  {
    uint8_t minR = 255, maxR = 0;
    for(uint16_t i = 0; i < count; ++i)
    {
      uint8_t r = 0;
      if(scanGetPoint(i, nullptr, &r, nullptr))
      {
        if(r < minR) minR = r;
        if(r > maxR) maxR = r;
      }
    }
    if(maxR <= minR) maxR = minR + 1;
    int lastX = gx, lastY = gy + gh - 2;
    for(uint16_t i = 0; i < count; ++i)
    {
      uint8_t r = 0;
      if(!scanGetPoint(i, nullptr, &r, nullptr)) continue;
      int x = gx + 1 + (int)((uint32_t)i * (gw - 3) / (count - 1));
      int y = gy + gh - 2 - (int)((uint32_t)(r - minR) * (gh - 5) / (maxR - minR));
      if(i) spr.drawLine(lastX, lastY, x, y, cyan);
      lastX = x;
      lastY = y;
    }
    int sx = gx + 1 + (int)((uint32_t)selectedPoint * (gw - 3) / (count - 1));
    spr.drawLine(sx, gy + 1, sx, gy + gh - 1, green);
    spr.fillTriangle(sx - 4, gy - 1, sx + 4, gy - 1, sx, gy + 5, green);
  }
  else
  {
    spr.setTextColor(muted, panel);
    spr.setFont(FONT_SMALL);
    spr.drawString("No scan data", 160, 83);
  }

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(white, panel);
  spr.setFont(FONT_DEFAULT);
  char startLabel[16]; snprintf(startLabel, sizeof(startLabel), "%u.%02u", (unsigned)(scanStart/100), (unsigned)(scanStart%100));\n  spr.drawString(startLabel, gx, 143);
  spr.setTextDatum(TR_DATUM);
  char endLabel[20]; snprintf(endLabel, sizeof(endLabel), "%u.%02u MHz", (unsigned)(scanEnd/100), (unsigned)(scanEnd%100));\n  spr.drawString(endLabel, gx + gw, 143);

  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(muted, panel);
  spr.setFont(FONT_DEFAULT);
  char footer[64];
  snprintf(footer, sizeof(footer), "%u points  |  Turn: select  |  Click: tune & exit",
           (unsigned)count);
  spr.drawString(footer, 160, 162);
}
