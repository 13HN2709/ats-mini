
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"
#include "FreqKeypad.h"

// Erweitertes Tastenfeld: 0-9, BS (Löschen), OK (Übernehmen), MODE (Modus umschalten)
static const char* keypadLayout[12] = {
  "1", "2", "3",
  "4", "5", "6",
  "7", "8", "9",
  "BS", "0", "OK"
};

// Zusätzliche Variable für die temporäre Modus-Auswahl während der Eingabe
static uint8_t tempMode = FM;
static int8_t keypadIndex = 4; // Startet auf '5'
static char inputFreqStr[12] = "";
static uint8_t inputLen = 0;

void freqKeypadReset()
{
  memset(inputFreqStr, 0, sizeof(inputFreqStr));
  inputLen = 0;
  keypadIndex = 4;
  tempMode = currentMode; // Aktuellen Systemmodus als Startwert übernehmen
}

// Navigation per Encoder (0-11 für das Tastenfeld + 1 extra Schritt für den Modus-Button oben)
bool doFreqKeypad(int16_t enc)
{
  keypadIndex += enc;
  if (keypadIndex < 0) keypadIndex = 12;  // 0-11 = Grid, 12 = Modus-Feld oben
  if (keypadIndex > 12) keypadIndex = 0;
  return true;
}

// Klick-Verarbeitung
bool clickFreqKeypad(bool shortPress)
{
  if (!shortPress) {
    // Langer Druck -> Abbrechen
    currentCmd = CMD_NONE;
    return true;
  }

  // Wenn der Modus-Button ganz oben ausgewählt wurde (Index 12)
  if (keypadIndex == 12) {
    // Modus zyklisch durchschalten (FM -> AM -> USB -> LSB -> zurück zu FM)
    tempMode = (tempMode + 1) % 4; // Angenommen 4 Modi stehen zur Verfügung
    return true;
  }

  const char* selectedKey = keypadLayout[keypadIndex];

  if (strcmp(selectedKey, "OK") == 0) {
    // Frequenz und Modus übernehmen
    if (inputLen > 0) {
      uint32_t newFreq = atol(inputFreqStr);
      
      // Modus setzen
      currentMode = tempMode;

      // Frequenz anwenden (abhängig von FM in MHz oder AM/SSB in kHz)
      if (currentMode == FM) {
        if (newFreq < 1000) newFreq *= 100; // z.B. 1079 -> 10790 (107.90 MHz)
      }
      
      updateFrequency(newFreq, false);
      // Wenn SSB, BFO zurücksetzen
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

// Zeichnen des erweiteten Keypads mit Modus-Anzeige
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

  // 1. Eingabe-Anzeige für Frequenz
  spr.fillRect(boxX + 15, boxY + 26, boxW - 110, 24, TH.bg);
  spr.drawRoundRect(boxX + 15, boxY + 26, boxW - 110, 24, 3, TH.menu_border);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.freq_text);
  spr.drawString(inputLen > 0 ? inputFreqStr : "---", boxX + 15 + (boxW - 110) / 2, boxY + 38);

  // 2. Modus-Anzeige / Auswahltaste (rechts neben der Frequenz)
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
