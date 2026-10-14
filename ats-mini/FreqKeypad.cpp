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

// Navigation auf dem Keypad per Encoder (Drehen)
bool doFreqKeypad(int16_t enc)
{
  if (!enc) return false;
  
  int step = (scrollDirection * enc > 0) ? 1 : -1;
  
  keypadIndex += step;
  if (keypadIndex < 0) keypadIndex = 12;  // 0-11 = Grid, 12 = Modus-Auswahlfeld
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

// Klick-Verarbeitung: Kurzer Klick = Ziffer wählen, Langer Klick = Abbrechen/Verlassen
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

// Zeichnen des Keypads: Kompakt, passgenau und gut lesbar
void drawFreqKeypad()
{
  int boxX = 25;
  int boxY = 8;
  int boxW = 270;
  int boxH = 215;

  // Tiefschwarzer Hintergrund
  spr.fillRect(boxX, boxY, boxW, boxH, 0x0000);
  spr.drawRoundRect(boxX, boxY, boxW, boxH, 4, 0xFCA0);

  // Titel
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFD20);
  spr.drawString("FREQUENZ & MODUS EINGABE", boxX + boxW / 2, boxY + 4, 1);

  // Frequenz-Anzeigefeld
  spr.fillRect(boxX + 10, boxY + 20, boxW - 100, 22, 0x0000);
  spr.drawRoundRect(boxX + 10, boxY + 20, boxW - 100, 22, 2, 0xFCA0);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0xFD20);
  spr.drawString(inputLen > 0 ? inputFreqStr : "---", boxX + 10 + (boxW - 100) / 2, boxY + 31, 2);

  // Modus-Auswahltaste rechts daneben
  int modeX = boxX + boxW - 85;
  int modeY = boxY + 20;
  int modeW = 75;
  int modeH = 22;
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
  spr.drawString(bandModeDesc[tempMode], modeX + modeW / 2, modeY + modeH / 2, 2);

  // Tastenfeld (Grid 3x4)
  int startX = boxX + 10;
  int startY = boxY + 48;
  int btnW = 80;
  int btnH = 34;
  int gapX = 8;
  int gapY = 3;

  for (int i = 0; i < 12; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = startX + col * (btnW + gapX);
    int y = startY + row * (btnH + gapY);

    bool isSelected = (i == keypadIndex);

    if (isSelected) {
      spr.fillRoundRect(x, y, btnW, btnH, 2, 0xFD20); // Aktives Highlight in Orange
      spr.setTextColor(0x0000);                       // Text schwarz
    } else {
      spr.fillRoundRect(x, y, btnW, btnH, 2, 0x1084); // Tasten dunkelgrau
      spr.setTextColor(0xFD20);                       // Text orange
    }
    spr.drawRoundRect(x, y, btnW, btnH, 2, 0xFCA0);

    spr.setTextDatum(MC_DATUM);
    spr.drawString(keypadLayout[i], x + btnW / 2, y + btnH / 2, 2);
  }
}
