#include "Common.h"
#include "Menu.h"
#include "Themes.h"
#include "Draw.h"
#include "Utils.h"
#include "SpectrumScanner.h"
#include <stdlib.h>
#include <string.h>

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
  "7", "8", "9", ".", "OK", "MHz", "", "CLOSE"
};
static char spectrumInput[2][12] = {"", ""};
static uint8_t spectrumInputStage = 0;
static uint8_t spectrumKeyIndex = 5;
static bool spectrumInputError = false;
static bool spectrumScanning = false;
static bool savedRadioStateValid = false;
static uint8_t savedBandIndex = 0;
static uint16_t savedFrequency = 0;
static uint8_t savedMode = FM;
static int16_t savedBFO = 0;

// Independent native-scan sample storage and sweep loop. Do not share the Web
// Spectrum buffer or state: the two scanner interfaces must not overwrite each other.
static constexpr uint16_t SPECTRUM_MAX_POINTS = 201; // 88.00..108.00 MHz in 100 kHz steps
static uint8_t spectrumRssi[SPECTRUM_MAX_POINTS] = {0};
static uint8_t spectrumSnr[SPECTRUM_MAX_POINTS] = {0};
static uint16_t spectrumPointCount = 0;

static bool spectrumScanGetPoint(uint16_t index, uint16_t *freq, uint8_t *rssi, uint8_t *snr)
{
  if(index >= spectrumPointCount) return false;
  if(freq) *freq = scanStart + VHF_STEP * index;
  if(rssi) *rssi = spectrumRssi[index];
  if(snr) *snr = spectrumSnr[index];
  return true;
}

static bool spectrumScanRunRange(uint16_t startFreq, uint16_t endFreq, uint16_t step)
{
  if(!step || endFreq < startFreq) return false;
  uint32_t points = ((uint32_t)endFreq - startFreq) / step + 1;
  if(points > SPECTRUM_MAX_POINTS) return false;

  spectrumPointCount = 0;
  rx.setMaxDelaySetFrequency(60);
  muteOn(MUTE_TEMP, true);
  seekStop = false;
  const uint16_t restoreFreq = currentFrequency;
  bool aborted = false;

  for(uint16_t i = 0; i < points; ++i)
  {
    const uint16_t freq = startFreq + step * i;
    rx.getStatus(1, 0);
    rx.setFrequency(freq);
    const uint32_t tuneStart = millis();

    while(true)
    {
      delay(10);
      rx.getStatus(0, 0);
      if(rx.getTuneCompleteTriggered() || millis() - tuneStart >= 350) break;
      if(consumeAbortPending()) { aborted = true; break; }
    }
    if(aborted) break;

    rx.getCurrentReceivedSignalQuality(0);
    spectrumRssi[i] = rx.getCurrentRSSI();
    spectrumSnr[i] = rx.getCurrentSNR();
    spectrumPointCount = i + 1;

    if(i + 1 < points && consumeAbortPending()) { aborted = true; break; }
  }

  rx.setFrequency(restoreFreq);
  muteOn(MUTE_TEMP, false);
  rx.setMaxDelaySetFrequency(30);
  return spectrumPointCount > 0;
}



static int findFmBandForRange(uint16_t startFreq, uint16_t endFreq)
{
  for(int i = 0; i < getTotalBands(); ++i)
  {
    const Band &band = bands[i];
    if(band.bandMode == FM && band.minimumFreq <= startFreq && band.maximumFreq >= endFreq)
      return i;
  }
  return -1;
}

static void restoreSavedRadioState()
{
  if(!savedRadioStateValid) return;
  selectBand(savedBandIndex, false);
  currentMode = savedMode;
  if(!updateFrequency(savedFrequency, false))
  {
    // A saved frequency should always fit its saved band; avoid leaving the
    // receiver in a transient scanner band if stored band metadata changed.
    useBand(&bands[savedBandIndex]);
  }
  if(isSSB()) updateBFO(savedBFO, true);
  savedRadioStateValid = false;
}

void spectrumScannerStart()
{
  spectrumInput[0][0] = spectrumInput[1][0] = '\0';
  spectrumInputStage = 0;
  spectrumKeyIndex = 5;
  spectrumInputError = false;
  spectrumHasData = false;
  spectrumScanning = false;
  savedRadioStateValid = false;
  currentCmd = CMD_SPECTRUM_INPUT;
}

bool spectrumScannerInputRotate(int16_t delta)
{
  if(!delta) return false;
  const int direction = (scrollDirection * delta > 0) ? 1 : -1;
  int next = (int)spectrumKeyIndex + direction;
  if(next < 0) next = 15;
  if(next > 15) next = 0;
  // Index 14 is the intentionally empty keypad cell. Skip it by moving
  // exactly one additional step; never loop on the unchanged index.
  if(next == 14) {
    next += direction;
    if(next < 0) next = 15;
    if(next > 15) next = 0;
  }
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
  const char *decimal = strchr(normalized, '.');
  if(decimal && strlen(decimal + 1) > 2) return false;
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

  if(!strcmp(key, "CANCEL") || !strcmp(key, "CLOSE")) { spectrumScannerInputCancel(); return true; }
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
      spectrumInput[1][0] = '\0';
      spectrumKeyIndex = 5;
      return true;
    }
    scanEnd = parsed;
    if(scanEnd <= scanStart) { spectrumInputError = true; return true; }

    // Validate that a configured FM band can cover the whole requested sweep
    // before changing the receiver at all.
    const int fmBand = findFmBandForRange(scanStart, scanEnd);
    if(fmBand < 0) { spectrumInputError = true; return true; }

    savedBandIndex = bandIdx;
    savedFrequency = currentFrequency;
    savedMode = currentMode;
    savedBFO = currentBFO;
    savedRadioStateValid = true;

    // The isolated native sweep tunes the receiver directly, so first configure
    // the tuner for FM, then restore the user's exact band/frequency/mode.
    selectBand((uint8_t)fmBand, false);
    currentMode = FM;
    currentBFO = 0;

    currentCmd = CMD_SPECTRUM;
    spectrumScanning = true;
    spectrumHasData = false;
    selectedPoint = 0;
    drawSpectrumScannerInput();
    spr.pushSprite(0, 0);
    spectrumScanRunRange(scanStart, scanEnd, VHF_STEP);

    uint16_t firstFreq = 0;
    spectrumHasData = spectrumPointCount > 0 &&
                      spectrumScanGetPoint(0, &firstFreq, nullptr, nullptr) &&
                      firstFreq == scanStart;
    spectrumScanning = false;
    restoreSavedRadioState();
    if(spectrumHasData) selectedPoint = spectrumPointCount / 2;
    return true;
  }
  if(!strcmp(key, "MHz")) return true; // fixed unit for VHF input
  if(!strcmp(key, ".") && strchr(value, '.')) return true;
  if(len < sizeof(spectrumInput[0])-1 && ((key[0] >= '0' && key[0] <= '9') || !strcmp(key, "."))) {
    value[len] = key[0];
    value[len+1] = '\0';
  }
  return true;
}

void drawSpectrumScannerInput()
{
  // Match the existing radio frequency keypad's geometry and styling.
  // VHF is fixed to FM and MHz: the mode/unit indicators are informational,
  // not controls, so no other modulation or kHz entry can be selected.
  const int boxX = 6;
  const int boxY = 2;
  const int boxW = 308;
  const int boxH = 166;
  const uint16_t frameBlue = 0x001F;
  const uint16_t brightBlue = 0x07FF;
  const uint16_t darkButton = 0x0841;
  const uint16_t red = 0xF800;

  spr.fillRect(boxX, boxY, boxW, boxH, 0x0000);
  spr.drawRoundRect(boxX, boxY, boxW, boxH, 4, frameBlue);
  spr.drawRoundRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, 3, frameBlue);

  if(spectrumScanning)
  {
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(brightBlue);
    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.drawString("SCANNING SPECTRUM", 160, 28);
    char range[40];
    snprintf(range, sizeof(range), "%u.%02u - %u.%02u MHz",
             (unsigned)(scanStart / 100), (unsigned)(scanStart % 100),
             (unsigned)(scanEnd / 100), (unsigned)(scanEnd % 100));
    spr.setTextColor(0x07E0);
    spr.drawString(range, 160, 62);
    spr.setTextColor(brightBlue);
    spr.setFont(FONT_DEFAULT);
    spr.drawString("Please wait...", 160, 105);
    return;
  }

  const int displayH = 23;
  const int displayX = boxX + 8;
  const int displayY = boxY + 20;
  const int displayW = boxW - 100;
  spr.fillRect(displayX, displayY, displayW, displayH, 0x0000);
  spr.drawRoundRect(displayX, displayY, displayW, displayH, 2,
                    spectrumInputError ? red : frameBlue);

  char displayBuf[24];
  const char *entered = spectrumInput[spectrumInputStage][0]
      ? spectrumInput[spectrumInputStage] : "---";
  snprintf(displayBuf, sizeof(displayBuf), "%s MHz", entered);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(spectrumInputError ? red : brightBlue);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString(displayBuf, displayX + displayW / 2, displayY + displayH / 2);
  spr.setTextDatum(TC_DATUM);
  spr.setFont(FONT_SMALL);
  spr.setTextColor(0xFFFF);
  if(spectrumInputStage == 0) {
    spr.drawString("ENTER START FREQUENCY (MHz)", 160, 6);
  } else {
    char startInfo[40];
    snprintf(startInfo, sizeof(startInfo), "START: %u.%02u MHz | ENTER END FREQUENCY",
             (unsigned)(scanStart / 100), (unsigned)(scanStart % 100));
    spr.drawString(startInfo, 160, 6);
  }

  // Fixed modulation indicator in the same position as the normal keypad's
  // mode selector. It deliberately cannot be focused or changed.
  const int modeX = boxX + boxW - 88;
  const int modeY = displayY;
  const int modeW = 80;
  spr.fillRoundRect(modeX, modeY, modeW, displayH, 2, darkButton);
  spr.drawRoundRect(modeX, modeY, modeW, displayH, 2, frameBlue);
  spr.setTextColor(brightBlue);
  spr.drawString("FM", modeX + modeW / 2, modeY + displayH / 2);

  // Same 4x4 key grid, spacing, selection colors and font as drawFreqKeypad().
  const int startX = boxX + 8;
  const int startY = boxY + 47;
  const int btnW = 68;
  const int btnH = 27;
  const int gapX = 5;
  const int gapY = 2;

  for(int i = 0; i < 16; ++i)
  {
    if(i == 14) continue;
    const int col = i % 4;
    const int row = i / 4;
    const int x = startX + col * (btnW + gapX);
    const int y = startY + row * (btnH + gapY);
    const bool selected = (i == spectrumKeyIndex);

    if(selected)
    {
      spr.fillRoundRect(x, y, btnW, btnH, 3, brightBlue);
      spr.drawRoundRect(x, y, btnW, btnH, 3, 0xFFFF);
      spr.setTextColor(0x0000);
    }
    else
    {
      spr.fillRoundRect(x, y, btnW, btnH, 3, darkButton);
      spr.drawRoundRect(x, y, btnW, btnH, 3, frameBlue);
      spr.drawRoundRect(x + 1, y + 1, btnW - 2, btnH - 2, 2, frameBlue);
      spr.setTextColor(brightBlue);
    }

    spr.setTextDatum(MC_DATUM);
    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.drawString(spectrumKeys[i], x + btnW / 2, y + btnH / 2);
  }

  // MHz is a fixed unit for this keypad, not a unit-toggle control.
  const int unitX = startX + (13 % 4) * (btnW + gapX);
  const int unitY = startY + (13 / 4) * (btnH + gapY);
  const bool unitSelected = (spectrumKeyIndex == 13);
  if(unitSelected)
  {
    spr.fillRoundRect(unitX, unitY, btnW, btnH, 3, brightBlue);
    spr.drawRoundRect(unitX, unitY, btnW, btnH, 3, 0xFFFF);
    spr.setTextColor(0x0000);
  }
  else
  {
    spr.fillRoundRect(unitX, unitY, btnW, btnH, 3, darkButton);
    spr.drawRoundRect(unitX, unitY, btnW, btnH, 3, frameBlue);
    spr.drawRoundRect(unitX + 1, unitY + 1, btnW - 2, btnH - 2, 2, frameBlue);
    spr.setTextColor(brightBlue);
  }
  spr.setTextDatum(MC_DATUM);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString("MHz", unitX + btnW / 2, unitY + btnH / 2);

  // Invalid input is indicated by the red frequency-field border/text, leaving
  // the full bottom key row unobstructed just like the native keypad.
}

void spectrumScannerRotate(int16_t delta)
{
  const uint16_t count = spectrumPointCount;
  if(!spectrumHasData || count < 3 || !delta) return;
  uint8_t minR = 255, maxR = 0;
  for(uint16_t i = 0; i < count; ++i) {
    const uint8_t value = spectrumRssi[i];
    if(value < minR) minR = value;
    if(value > maxR) maxR = value;
  }
  // Keep the threshold low enough to include ordinary FM stations, not only
  // the strongest transmitters. Every encoder detent should target another
  // local peak rather than stepping through adjacent 100 kHz samples.
  const uint8_t threshold = (uint8_t)(minR + ((uint16_t)(maxR - minR) * 5) / 100);
  const int direction = delta > 0 ? 1 : -1;
  int candidate = selectedPoint;
  for(uint16_t step = 0; step < count; ++step) {
    candidate += direction;
    if(candidate < 0) candidate = count - 1;
    if(candidate >= count) candidate = 0;
    if(candidate == 0 || candidate == count - 1) continue;
    const uint8_t here = spectrumRssi[candidate];
    if(here >= threshold && here >= spectrumRssi[candidate - 1] && here > spectrumRssi[candidate + 1]) {
      selectedPoint = (uint16_t)candidate;
      return;
    }
  }

  // If the trace is unusually flat and has no qualifying local maximum,
  // jump to the strongest sample in the requested direction instead of
  // moving only 100 kHz and appearing to ignore peak navigation.
  int best = -1;
  uint8_t bestRssi = 0;
  candidate = selectedPoint;
  for(uint16_t step = 0; step < count - 1; ++step) {
    candidate += direction;
    if(candidate < 0) candidate = count - 1;
    if(candidate >= count) candidate = 0;
    if(candidate == selectedPoint) break;
    if(candidate == 0 || candidate == count - 1) continue;
    if(best < 0 || spectrumRssi[candidate] > bestRssi) {
      best = candidate;
      bestRssi = spectrumRssi[candidate];
    }
  }
  if(best >= 0) selectedPoint = (uint16_t)best;
}

void spectrumScannerSelect()
{
  if(spectrumHasData) {
    uint16_t freq = 0;
    if(spectrumScanGetPoint(selectedPoint, &freq, nullptr, nullptr)) {
      const int fmBand = findFmBandForRange(freq, freq);
      if(fmBand >= 0) {
        selectBand((uint8_t)fmBand, false);
        currentMode = FM;
        currentBFO = 0;
        updateFrequency(freq, false);
        savedRadioStateValid = false; // Accepted selection intentionally replaces saved state.
      }
    }
  }
  currentCmd = CMD_NONE;
}

void drawSpectrumScanner()
{
  const uint16_t bg = 0x0000;
  const uint16_t panel = 0x0008;
  const uint16_t grid = 0x000F;
  const uint16_t cyan = 0x07FF;
  const uint16_t blue = 0x001F;
  const uint16_t green = 0x07E0;
  const uint16_t red = 0xF800;
  const uint16_t wave = 0x18C3;
  const uint16_t white = 0xFFFF;
  const uint16_t muted = 0x4208;

  spr.fillSprite(bg);
  spr.fillRoundRect(3, 3, 314, 164, 5, panel);
  spr.drawRoundRect(3, 3, 314, 164, 5, blue);

  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(cyan, panel);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString("VHF SPECTRUM", 160, 7);

  uint16_t selectedFreq = scanStart;
  if(spectrumHasData)
    spectrumScanGetPoint(selectedPoint, &selectedFreq, nullptr, nullptr);

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

  const uint16_t count = spectrumPointCount;
  if(spectrumHasData && count > 1)
  {
    uint8_t minR = 255, maxR = 0;
    for(uint16_t i = 0; i < count; ++i)
    {
      uint8_t r = 0;
      if(spectrumScanGetPoint(i, nullptr, &r, nullptr))
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
      if(!spectrumScanGetPoint(i, nullptr, &r, nullptr)) continue;
      int x = gx + 1 + (int)((uint32_t)i * (gw - 3) / (count - 1));
      int y = gy + gh - 2 - (int)((uint32_t)(r - minR) * (gh - 5) / (maxR - minR));
      if(i) spr.drawLine(lastX, lastY, x, y, wave);
      lastX = x;
      lastY = y;
    }
    const uint8_t peakThreshold = (uint8_t)(minR + ((uint16_t)(maxR - minR) * 12) / 100);
    for(uint16_t i = 1; i + 1 < count; ++i) {
      const uint8_t prev = spectrumRssi[i - 1];
      const uint8_t here = spectrumRssi[i];
      const uint8_t next = spectrumRssi[i + 1];
      if(here >= peakThreshold && here >= prev && here > next) {
        int px = gx + 1 + (int)((uint32_t)i * (gw - 3) / (count - 1));
        int py = gy + gh - 2 - (int)((uint32_t)(here - minR) * (gh - 5) / (maxR - minR));
        spr.drawFastVLine(px, py, max(3, gy + gh - 2 - py), red);
        spr.fillTriangle(px - 3, py - 1, px + 3, py - 1, px, py - 6, red);
      }
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
  char startLabel[16]; snprintf(startLabel, sizeof(startLabel), "%u.%02u", (unsigned)(scanStart/100), (unsigned)(scanStart%100));
  spr.drawString(startLabel, gx, 143);
  spr.setTextDatum(TR_DATUM);
  char endLabel[20]; snprintf(endLabel, sizeof(endLabel), "%u.%02u MHz", (unsigned)(scanEnd/100), (unsigned)(scanEnd%100));
  spr.drawString(endLabel, gx + gw, 143);

  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(muted, panel);
  spr.setFont(FONT_DEFAULT);
  char footer[64];
  snprintf(footer, sizeof(footer), "%u points  |  Turn: select  |  Click: tune & exit",
           (unsigned)count);
  spr.drawString(footer, 160, 162);
}
