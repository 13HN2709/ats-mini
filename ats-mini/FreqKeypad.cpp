#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Memories.h"
#include "Draw.h"
#include "FreqKeypad.h"

// Tastenfeld im 4x3 Querformat-Layout: 
// Zeile 1: 1, 2, 3, BS
// Zeile 2: 4, 5, 6, 0
// Zeile 3: 7, 8, 9, OK
static const char* keypadLayout[12] = {
  "1", "2", "3", "BS",
  "4", "5", "6", "0",
  "7", "8", "9", "OK"
};

static uint8_t tempMode = FM;
static int8_t keypadIndex = 5; // Startet auf '5'
static char inputFreqStr[12] = "";
static uint8_t inputLen = 0;

void freqKeypadReset()
{
  memset(inputFreqStr, 0, sizeof(inputFreqStr));
  inputLen = 0;
  keypadIndex = 5;
  tempMode = currentMode;
}

// Navigation auf dem Keypad per Encoder (Drehen)
bool doFreqKeypad(int16_t enc)
{
  if (!enc) return false;
  
  int step = (scrollDirection * enc > 0) ? 1 : -1;
  
  keypadIndex += step;
  if (keypadIndex < 0) keypadIndex = 12;  // 0-11 = 4x3 Grid, 12 = Modus-Auswahlfeld
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

// Klick-Verarbeitung: Kurzer Klick = Ziffer/Aktion wählen, Langer Klick = Abbrechen
bool clickFreqKeypad(bool shortPress)
{
  // Langer Klick -> Keypad sofort verlassen (abbrechen)
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

// Zeichnen des Keypads mit sauberen Borders und hoher Lesbarkeit
void drawFreqKeypad()
{
  int boxX = 6;
  int boxY = 2;
  int boxW = 308;
  int boxH = 166;

  // Tiefschwarzer Hintergrund und äußerer Rahmen
  spr.fillRect(boxX, boxY, boxW, boxH, 0x0000);
  spr.drawRoundRect(boxX, boxY, boxW, boxH, 4, 0xFCA0);
  spr.drawRoundRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, 3, 0xFCA0); // Doppelte Umrandung für Tiefe

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

  // Modus-Auswahltaste rechts oben
  int modeX = boxX + boxW - 88;
  int modeY = boxY + 16;
  int modeW = 80;
  int modeH = 20;
  bool modeSelected = (keypadIndex == 12);

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

  // Tastenfeld im 4x3 Format mit klarer Hervorhebung
  int startX = boxX + 8;
  int startY = boxY + 40;
  int btnW = 68;
  int btnH = 38;
  int gapX = 5;
  int gapY = 3;

  for (int i = 0; i < 12; i++) {
    int col = i % 4; // 4 Spalten
    int row = i / 4; // 3 Reihen
    int x = startX + col * (btnW + gapX);
    int y = startY + row * (btnH + gapY);

    bool isSelected = (i == keypadIndex);

    if (isSelected) {
      // Aktive Taste: Leuchtendes Orange mit schwarzem Text für maximalen Kontrast
      spr.fillRoundRect(x, y, btnW, btnH, 3, 0xFD20);
      spr.drawRoundRect(x, y, btnW, btnH, 3, 0xFFFF); // Weißer Innenrand bei Selektion
      spr.setTextColor(0x0000);                       
    } else {
      // Inaktive Tasten: Dunkler Hintergrund mit feinem orangen Rahmen und doppelter Border-Optik
      spr.fillRoundRect(x, y, btnW, btnH, 3, 0x0841); 
      spr.drawRoundRect(x, y, btnW, btnH, 3, 0xFCA0); 
      spr.drawRoundRect(x + 1, y + 1, btnW - 2, btnH - 2, 2, 0x528A); // Dezenter innerer Rahmen
      spr.setTextColor(0xFD20);                       
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(keypadLayout[i], x + btnW / 2, y + btnH / 2, 2);
  }
}
