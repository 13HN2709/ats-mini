#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Memories.h"
#include "Draw.h"
#include "FreqKeypad.h"

// Tastenfeld: Ziffern 0-9, Löschen (BS), Übernehmen (OK)
static const char* keypadLayout[12] = {
  "1", "2", "3",
  "4", "5", "6",
  "7", "8", "9",
  "BS", "0", "OK"
};

static uint8_t tempMode = FM;
static int8_t keypadIndex = 4; // Startet auf '5'
static char inputFreqStr[12] = "";
static uint8_t inputLen = 0;

void freqKeypadReset()
{
  memset(inputFreqStr, 0, sizeof(inputFreqStr));
  inputLen = 0;
  keypadIndex = 4;
  tempMode = currentMode;
}

// Navigation auf dem Keypad & Modus-Button per Encoder
bool doFreqKeypad(int16_t enc)
{
  if (!enc) return false;
  
  // Berücksichtige die Scroll-Richtung des Users für natürliche Bedienung
  int step = (scrollDirection * enc > 0) ? 1 : -1;
  
  keypadIndex += step;
  if (keypadIndex < 0) keypadIndex = 12;  // 0-11 = Grid, 12 = Modus-Auswahlfeld oben
  if (keypadIndex > 12) keypadIndex = 0;
  return true;
}

// Automatische Band-Auswahl basierend auf Frequenz und Modus
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

// Klick-Verarbeitung: Kurzer Klick wählt die Taste, langer Klick bricht ab
bool clickFreqKeypad(bool shortPress)
{
  // Langer Klick -> Keypad verlassen (abbrechen)
  if (!shortPress) {
    currentCmd = CMD_NONE; 
    return true;
  }

  // Modus-Feld oben angewählt (Index 12)
  if (keypadIndex == 12) {
    tempMode = (tempMode + 1) % 4; // Durchschalten: FM, AM, LSB, USB
    return true;
  }

  const char* selectedKey = keypadLayout[keypadIndex];

  if (strcmp(selectedKey, "OK") == 0) {
    if (inputLen > 0) {
      uint32_t rawVal = atol(inputFreqStr);
      uint32_t targetFreq = rawVal;

      if (tempMode == FM) {
        if (rawVal < 200) targetFreq = rawVal * 100; 
      }

      switchToMatchingBand(targetFreq, tempMode);
      currentMode = tempMode;
      updateFrequency(targetFreq, false);
      if (isSSB()) updateBFO(0, true);
    }
    currentCmd = CMD_NONE; // Keypad nach OK schließen
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

// Zeichnen des Keypads in Schwarz mit großen orangen Ziffern
void drawFreqKeypad()
{
  int boxX = 20;
  int boxY = 10;
  int boxW = 280;
  int boxH = 220;

  // Tiefschwarzer Hintergrund für maximalen Kontrast
  spr.fillRect(boxX, boxY, boxW, boxH, 0x0000);
  spr.drawRoundRect(boxX, boxY, boxW, boxH, 6, 0xFCA0);

  // Titel in hellem Orange/Weiß
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFD20);
  spr.drawString("FREQUENZ & MODUS EINGABE", boxX + boxW / 2, boxY + 6, FONT_SMALL);

  // Frequenz-Anzeigefeld (schwarz hinterlegt)
  spr.fillRect(boxX + 15, boxY + 24, boxW - 115, 26, 0x0000);
  spr.drawRoundRect(boxX + 15, boxY + 24, boxW - 115, 26, 3, 0xFCA0);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0xFD20);
  spr.drawString(inputLen > 0 ? inputFreqStr : "---", boxX + 15 + (boxW - 115) / 2, boxY + 37);

  // Modus-Auswahltaste rechts daneben
  int modeX = boxX + boxW - 95;
  int modeY = boxY + 24;
  int modeW = 80;
  int modeH = 26;
  bool modeSelected = (keypadIndex == 12);

  if (modeSelected) {
    spr.fillRoundRect(modeX, modeY, modeW, modeH, 3, 0xFD20); // Aktives Gelb/Orange
    spr.setTextColor(0x0000); // Schwarzer Text auf hellem Grund
  } else {
    spr.fillRoundRect(modeX, modeY, modeW, modeH, 3, 0x1084); // Dunkelgrau
    spr.setTextColor(0xFD20); // Orangener Text
  }
  spr.drawRoundRect(modeX, modeY, modeW, modeH, 3, 0xFCA0);
  spr.setTextDatum(MC_DATUM);
  spr.drawString(bandModeDesc[tempMode], modeX + modeW / 2, modeY + modeH / 2);

  // Tastenfeld (Grid 3x4) mit größeren Ziffern
  int startX = boxX + 15;
  int startY = boxY + 58;
  int btnW = 78;
  int btnH = 34;
  int gapX = 12;
  int gapY = 5;

  for (int i = 0; i < 12; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = startX + col * (btnW + gapX);
    int y = startY + row * (btnH + gapY);

    bool isSelected = (i == keypadIndex);

    if (isSelected) {
      spr.fillRoundRect(x, y, btnW, btnH, 3, 0xFD20); // Aktives Highlight in Orange
      spr.setTextColor(0x0000);                       // Text schwarz
    } else {
      spr.fillRoundRect(x, y, btnW, btnH, 3, 0x1084); // Tasten-Hintergrund dunkelgrau
      spr.setTextColor(0xFD20);                       // Ziffern in kräftigem Orange
    }
    spr.drawRoundRect(x, y, btnW, btnH, 3, 0xFCA0);

    spr.setTextDatum(MC_DATUM);
    spr.drawString(keypadLayout[i], x + btnW / 2, y + btnH / 2);
  }
}
