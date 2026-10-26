#include "Common.h"
#include "Menu.h"
#include "Draw.h"
#include "CWMode.h"
#include "CWDecoder.h"
#include "CWAudioDetector.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static CWDecoder cwDecoder;
static bool cwActive = false;
static bool cwWifiBlocked = false;
static uint32_t cwLastUiTick = 0;
static bool cwSignalGreen = false;
static CWAudioDetector cwAudio;
static int16_t cwSamples[80];
static uint32_t cwLastAudioBlock = 0;
static uint32_t cwLastToneSeen = 0;
static uint32_t cwLastToneStateChange = 0;
static bool cwToneState = false;
#ifndef CW_AUDIO_PIN
#define CW_AUDIO_PIN 11
#endif
static constexpr uint32_t CW_SAMPLE_RATE = 4000;
static constexpr uint16_t CW_TONE_HZ = 700;
static constexpr uint16_t CW_BLOCK_SIZE = 80;

// Wi-Fi is not started/stopped by CW mode. The user can switch it off in
// Settings before entering the decoder; the mode only checks the configured state.
extern uint8_t wifiModeIdx;
#ifndef NET_OFF
#define NET_OFF 0
#endif

bool cwDecoderActive() { return cwActive; }
bool cwDecoderWifiBlocked() { return cwWifiBlocked; }

void cwDecoderEnter()
{
  cwWifiBlocked = (wifiModeIdx != NET_OFF);
  if (cwWifiBlocked) {
    cwActive = false;
    currentCmd = CMD_CW_DECODER;
    return;
  }
  cwDecoder.reset();
  cwAudio.configure(CW_SAMPLE_RATE, CW_TONE_HZ, CW_BLOCK_SIZE, 0.20f);
  pinMode(CW_AUDIO_PIN, INPUT);
  cwSignalGreen = false;
  cwToneState = false;
  cwLastAudioBlock = millis();
  cwLastToneSeen = 0;
  cwLastToneStateChange = millis();
  cwLastUiTick = millis();
  cwActive = true;
  currentCmd = CMD_CW_DECODER;
}

void cwDecoderExit()
{
  cwActive = false;
  cwWifiBlocked = false;
  currentCmd = CMD_NONE;
}

void cwDecoderTick()
{
  if (!cwActive) return;
  uint32_t now = millis();
  if (now - cwLastAudioBlock >= 20) {
    const uint32_t blockStartUs = micros();
    for (uint16_t i = 0; i < CW_BLOCK_SIZE; ++i) {
      cwSamples[i] = (int16_t)analogRead(CW_AUDIO_PIN);
      const uint32_t targetUs = blockStartUs + ((uint32_t)(i + 1) * 1000000UL / CW_SAMPLE_RATE);
      while ((int32_t)(micros() - targetUs) < 0) { }
    }
    const bool toneNow = cwAudio.processBlock(cwSamples, CW_BLOCK_SIZE);
    now = millis();
    cwLastAudioBlock = now;
    if (toneNow) cwLastToneSeen = now;
    if (toneNow != cwToneState) {
      cwToneState = toneNow;
      cwLastToneStateChange = now;
    }
    cwDecoder.feedToneState(toneNow, now);
  }
  cwDecoder.tick(now);
  cwSignalGreen = (cwLastToneSeen != 0 && now - cwLastToneSeen < 350 &&
                   now - cwLastToneStateChange < 1200);
  if (now - cwLastUiTick >= 100) cwLastUiTick = now;
}

void drawCWDecoderScreen()
{
  const uint16_t navy = 0x0008;
  const uint16_t blueA = 0x0359;
  const uint16_t blueB = 0x047D;
  const uint16_t frame = 0xF800;
  const uint16_t green = 0x07E0;
  const uint16_t red = 0xF800;
  spr.fillSprite(navy);

  if (cwWifiBlocked) {
    spr.drawRect(8, 8, 304, 154, 0xF800);
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(0xF800);
    spr.drawString("PLEASE TURN WIFI/AP OFF", 160, 62, FONT_LARGE);
    spr.setTextColor(0xFFFF);
    spr.drawString("CW decoder needs Wi-Fi disabled", 160, 91, FONT_SMALL);
    spr.setTextColor(0x07E0);
    spr.drawString("CLICK ENCODER TO RETURN", 160, 125, FONT_SMALL);
    return;
  }

  // Header/frequency box: intentionally retained as the first panel.
  spr.fillRect(8, 5, 304, 31, blueA);
  spr.drawRect(8, 5, 304, 31, frame);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(0xFFFF);
  char freq[40];
  if (currentMode == FM) snprintf(freq, sizeof(freq), "%.2f MHz  FM", currentFrequency / 100.0f);
  else snprintf(freq, sizeof(freq), "%u kHz  %s", currentFrequency, currentMode < 4 ? bandModeDesc[currentMode] : "?");
  spr.drawString(freq, 16, 13, FONT_SMALL);

  // Alternating blue text panels from the reference photo, filling the screen.
  const int panelY[] = {40, 69, 98, 127};
  const int panelH = 25;
  const char *decoded = cwDecoder.text();
  size_t len = strlen(decoded);
  const int charsPerLine = 34;
  for (int row = 0; row < 4; ++row) {
    spr.fillRect(8, panelY[row], 304, panelH, (row & 1) ? blueB : blueA);
    spr.drawRect(8, panelY[row], 304, panelH, 0x04BF);
    size_t end = len > (size_t)((row + 1) * charsPerLine) ? len - (size_t)((row + 1) * charsPerLine) : 0;
    size_t start = len > (size_t)((row + 1) * charsPerLine) ? len - (size_t)((row + 1) * charsPerLine) : 0;
    // Render the most recent text from right to left by rows, oldest visible row first.
    size_t visibleLen = len > (size_t)(row * charsPerLine) ? len - (size_t)(row * charsPerLine) : 0;
    if (visibleLen > charsPerLine) visibleLen = charsPerLine;
    if (len >= (size_t)((row + 1) * charsPerLine)) {
      start = len - (size_t)((row + 1) * charsPerLine);
      visibleLen = charsPerLine;
    } else {
      start = 0;
      visibleLen = row == 0 ? len : 0;
    }
    if (visibleLen) {
      char line[charsPerLine + 1];
      memcpy(line, decoded + start, visibleLen);
      line[visibleLen] = '\0';
      spr.setTextDatum(ML_DATUM);
      spr.setTextColor(0xFFFF);
      spr.drawString(line, 14, panelY[row] + 5, FONT_SMALL);
    }
    (void)end;
  }

  // Signal status indicator. Red is the safe default until live audio is connected.
  const uint16_t led = cwSignalGreen ? green : red;
  if ((millis() / 350) & 1) spr.fillCircle(296, 20, 5, led);
  else spr.drawCircle(296, 20, 5, led);
  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(cwSignalGreen ? green : red);
  spr.drawString(cwSignalGreen ? "SIGNAL OK" : "NO SIGNAL", 284, 20, FONT_SMALL);
  spr.setTextColor(0xBDF7);
  spr.setTextDatum(MC_DATUM);
  spr.drawString("CLICK: EXIT  |  ROTATE: TUNE/BFO", 160, 163, FONT_SMALL);
}
