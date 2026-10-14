#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Memories.h"
#include "Draw.h"
#include "FreqKeypad.h"

// 4 Spalten, 4 Reihen (16 Felder, davon 13 belegt)
static const char* keypadLayout[16] = {
  "1", "2", "3", "BS",
  "4", "5", "6", "0",
  "7", "8", "9", ".",
  "OK", "", "", ""   // Index 12 ist OK, 13-15 leer/ohne Funktion
};

static uint8_t tempMode = FM;
static int8_t keypadIndex = 5; // Startet auf '5'
static char inputFreqStr[15] = "";
static uint8_t inputLen = 0;

void freqKeypadReset()
{
  memset(inputFreqStr, 0, sizeof(inputFreqStr));
  inputLen = 0;
  keypadIndex = 5;
  tempMode = currentMode;
}

// Navigation auf dem Keypad per Encoder (4x4 Grid + Modus = 17 Positionen: 0-15 Grid, 16 Modus)
bool doFreqKeypad(int16_t enc)
{
  if (!enc) return false;
  
  int step = (scrollDirection * enc > 0) ? 1 : -1;
  
  keypadIndex += step;
  if (keypadIndex < 0) keypadIndex = 16;  
  if (keypadIndex > 16) keypadIndex = 0;
  
  // Leere Felder in der letzten Reihe überspringen
  while ((keypadIndex >= 13 && keypadIndex <= 15)) {
    keypadIndex += step;
    if (keypadIndex < 0) keypadIndex = 16;
    if (keypadIndex > 16) keypadIndex = 0;
  }
  
  return true;
}

// Automatische Band-Auswahl
static void switchToMatchingBand(uint32_t freq, uint8_t mode)
{
  int bestMatch = -1;

  for (int i = 0; i < getTotalBands(); i++) {
    const Band *b = &bands[i];
    if (freq >= b->minimumFreq && freq <= b->maximumFreq) {
      if (b->bandMode == mode) {
        bestMatch = i;
        break;
      } else if (bestMatch == -1) {
        bestMatch = i;
      }
    }
  }

  if (bestMatch != -1) {
    selectBand(bestMatch, false);
  }
}

// Klick-Verarbeitung mit universeller FM- und AM/SSB-Skalierung
bool clickFreqKeypad(bool shortPress)
{
  if (!shortPress) {
    currentCmd = CMD_NONE; 
    return true;
  }

  // Modus-Feld oben angewählt (Index 16)
  if (keypadIndex == 16) {
    tempMode = (tempMode + 1) % 4; 
    return true;
  }

  // Leere Felder abfangen
  if (keypadIndex >= 13 && keypadIndex <= 15) return true;

  const char* selectedKey = keypadLayout[keypadIndex];

  if (strcmp(selectedKey, "OK") == 0) {
    if (inputLen > 0) {
      uint32_t targetFreq = 0;

      if (tempMode == FM) {
        // FM: Wandelt z.B. "107.9" in das interne Format um
        float fVal = atof(inputFreqStr);
        if (fVal < 200.0f) {
          targetFreq = (uint32_t)(fVal * 100.0f);
        } else {
          targetFreq = (uint32_t)(fVal / 10.0f);
        }
      } else {
        // AM / SSB: Direkt in kHz
        targetFreq = atol(inputFreqStr);
      }

      switchToMatchingBand(targetFreq, tempMode);
      currentMode = tempMode;
      updateFrequency(targetFreq, false);
      if (isSSB()) updateBFO(0, true);
    }
    currentCmd = CMD_NONE; 
    return true;
  }
  else if (strcmp(selectedKey, "BS") == 0) {
    if (inputLen > 0) {
      inputFreqStr[--inputLen] = '\0';
    }
  }
  else {
    if (inputLen < sizeof(inputFreqStr) - 1) {
      inputFreqStr[inputLen++] = selectedKey[0];
      inputFreqStr[inputLen] = '\0';
    }
  }
  return true;
}

// Zeichnen des Keypads im 4x4 Layout mit sauberen Borders
void drawFreqKeypad()
{
  int boxX = 6;
  int boxY = 2;
  int boxW = 308;
  int boxH = 166;

  spr.fillRect(boxX, boxY, boxW, boxH, 0x0000);
  spr.drawRoundRect(boxX, boxY, boxW, boxH, 4, 0xFCA0);
  spr.drawRoundRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, 3, 0xFCA0);

  // Titel
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFD20);
  spr.drawString("FREQUENZ & MODUS EINGABE", boxX + boxW / 2, boxY + 3, 1);

  // Frequenz-Anzeigefeld
  spr.fillRect(boxX + 8, boxY + 16, boxW - 100, 20, 0x0000);
  spr.drawRoundRect(boxX + 8, boxY + 16, boxW - 100, 20, 2, 0xFCA0);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0xFD20);
  spr.drawString(inputLen > 0 ? inputFreqStr : "---", boxX + 8 + (boxW - 100) / 2, boxY + 26, 2);

  // Modus-Auswahltaste rechts oben (Index 16)
  int modeX = boxX + boxW - 88;
  int modeY = boxY + 16;
  int modeW = 80;
  int modeH = 20;
  bool modeSelected = (keypadIndex == 16);

  if (modeSelected) {
    spr.fillRoundRect(modeX, modeY, modeW, modeH, 2, 0xFD20);
    spr.setTextColor(0x0000);
  } else {
    spr.fillRoundRect(modeX, modeY, modeW, modeH, 2, 0x1084);
    spr.setTextColor(0xFD20);
  }
  spr.drawRoundRect(modeX, modeY, modeW, modeH, 2, 0xFCA0);
  spr.setTextDatum(MC_DATUM);
  spr.drawString(bandModeDesc[tempMode], modeX + modeW / 2, modeY + modeH / 2, 1);

  // 4x4 Tastenfeld (4 Spalten, 4 Reihen)
  int startX = boxX + 8;
  int startY = boxY + 39;
  int btnW = 68;
  int btnH = 28;
  int gapX = 5;
  int gapY = 2;

  for (int i = 0; i < 16; i++) {
    if (i >= 13 && i <= 15) continue;

    int col = i % 4;
    int row = i / 4;
    int x = startX + col * (btnW + gapX);
    int y = startY + row * (btnH + gapY);

    bool isSelected = (i == keypadIndex);

    if (isSelected) {
      spr.fillRoundRect(x, y, btnW, btnH, 3, 0xFD20);
      spr.drawRoundRect(x, y, btnW, btnH, 3, 0xFFFF);
      spr.setTextColor(0x0000);                       
    } else {
      spr.fillRoundRect(x, y, btnW, btnH, 3, 0x0841); 
      spr.drawRoundRect(x, y, btnW, btnH, 3, 0xFCA0); 
      spr.drawRoundRect(x + 1, y + 1, btnW - 2, btnH - 2, 2, 0x528A); 
      spr.setTextColor(0xFD20);                       
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(keypadLayout[i], x + btnW / 2, y + btnH / 2, 2);
  }
}
