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
  keypadIndex += enc;
  if (keypadIndex < 0) keypadIndex = 12;  // 0-11 = Grid, 12 = Modus-Auswahlfeld oben
  if (keypadIndex > 12) keypadIndex = 0;
  return true;
}

// Automatische Band-Auswahl basierend auf Frequenz und Modus
static void switchToMatchingBand(uint32_t freq, uint8_t mode)
{
  int bestMatch = -1;

  // Durchsuche alle Bänder nach einem passenden Bereich
  for (int i = 0; i < getTotalBands(); i++) {
    const Band *b = &bands[i];
    if (freq >= b->minimumFreq && freq <= b->maximumFreq) {
      if (b->bandMode == mode) {
        bestMatch = i;
        break; // Exakter Treffer (Frequenz + Modus)
      } else if (bestMatch == -1) {
        bestMatch = i; // Fallback: Frequenz passt im Band
      }
    }
  }

  if (bestMatch != -1) {
    selectBand(bestMatch, false);
  }
}

// Klick-Verarbeitung
bool clickFreqKeypad(bool shortPress)
{
  if (!shortPress) {
    currentCmd = CMD_NONE; // Langer Druck -> Abbrechen
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

      // Konvertierung je nach Modus (FM in 10kHz Schritten / AM-SSB in kHz)
      if (tempMode == FM) {
        if (rawVal < 200) targetFreq = rawVal * 100; // z.B. "1079" -> 10790 (107.90 MHz)
      }

      // 1. Passendes Band automatisch suchen und aktivieren
      switchToMatchingBand(targetFreq, tempMode);

      // 2. Modus setzen
      currentMode = tempMode;

      // 3. Exakte Frequenz anwenden
      updateFrequency(targetFreq, false);
      if (isSSB()) updateBFO(0, true);

      prefsRequestSave(SAVE_CUR_BAND);
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

// Zeichnen des Keypads
void drawFreqKeypad()
{
  int boxX = 30;
  int boxY = 15;
  int boxW = 260;
  int boxH = 210;

  spr.fillSmoothRoundRect(boxX, boxY, boxW, boxH, 6, TH.menu_bg);
  spr.drawRoundRect(boxX, boxY, boxW, boxH, 6, TH.menu_border);

  // Titel
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(TH.text);
  spr.drawString("FREQUENZ & MODUS EINGABE", boxX + boxW / 2, boxY + 8, FONT_SMALL);

  // Frequenz-Anzeigefeld
  spr.fillRect(boxX + 15, boxY + 26, boxW - 110, 24, TH.bg);
  spr.drawRoundRect(boxX + 15, boxY + 26, boxW - 110, 24, 3, TH.menu_border);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.freq_text);
  spr.drawString(inputLen > 0 ? inputFreqStr : "---", boxX + 15 + (boxW - 110) / 2, boxY + 38);

  // Modus-Auswahltaste rechts daneben
  int modeX = boxX + boxW - 85;
  int modeY = boxY + 26;
  int modeW = 70;
  int modeH = 24;
  bool modeSelected = (keypadIndex == 12);

  if (modeSelected) {
    spr.fillRoundRect(modeX, modeY, modeW, modeH, 3, TH.freq_hl_sel);
    spr.setTextColor(TH.bg);
  } else {
    spr.fillRoundRect(modeX, modeY, modeW, modeH, 3, TH.menu_item);
    spr.setTextColor(TH.text);
  }
  spr.drawRoundRect(modeX, modeY, modeW, modeH, 3, TH.menu_border);
  spr.setTextDatum(MC_DATUM);
  spr.drawString(bandModeDesc[tempMode], modeX + modeW / 2, modeY + modeH / 2);

  // Tastenfeld (Grid 3x4)
  int startX = boxX + 20;
  int startY = boxY + 60;
  int btnW = 70;
  int btnH = 30;
  int gapX = 15;
  int gapY = 6;

  for (int i = 0; i < 12; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = startX + col * (btnW + gapX);
    int y = startY + row * (btnH + gapY);

    bool isSelected = (i == keypadIndex);

    if (isSelected) {
      spr.fillRoundRect(x, y, btnW, btnH, 3, TH.freq_hl_sel);
      spr.setTextColor(TH.bg);
    } else {
      spr.fillRoundRect(x, y, btnW, btnH, 3, TH.menu_item);
      spr.setTextColor(TH.text);
    }
    spr.drawRoundRect(x, y, btnW, btnH, 3, TH.menu_border);

    spr.setTextDatum(MC_DATUM);
    spr.drawString(keypadLayout[i], x + btnW / 2, y + btnH / 2);
  }
}
