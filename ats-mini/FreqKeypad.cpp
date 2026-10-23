#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Memories.h"
#include "Draw.h"
#include "FreqKeypad.h"

// 4 Spalten, 4 Reihen (16 Felder: Index 13 ist UNIT, 14-15 frei)
static const char* keypadLayout[16] = {
  "1", "2", "3", "BS",
  "4", "5", "6", "0",
  "7", "8", "9", ".",
  "OK", "UNIT", "", "CLOSE"   // Index 12 = OK, Index 13 = UNIT, Index 15 = CLOSE
};

static uint8_t tempMode = FM;
static int8_t keypadIndex = 5; // Startet auf '5'
static char inputFreqStr[15] = "";
static uint8_t inputLen = 0;
static bool inputInMHz = false; // Status für MHz / kHz Umschaltung
static bool freqRangeError = false;

void freqKeypadReset()
{
  memset(inputFreqStr, 0, sizeof(inputFreqStr));
  inputLen = 0;
  keypadIndex = 5;
  tempMode = currentMode;
  inputInMHz = (tempMode == FM);
  freqRangeError = false;
}

// Navigation auf dem Keypad per Encoder (4x4 Grid + Modus = 17 Positionen: 0-15 Grid, 16 Modus)
bool doFreqKeypad(int16_t enc)
{
  if (!enc) return false;
  
  int step = (scrollDirection * enc > 0) ? 1 : -1;
  
  keypadIndex += step;
  if (keypadIndex < 0) keypadIndex = 16;  
  if (keypadIndex > 16) keypadIndex = 0;
  
  // Index 14 bleibt leer; Index 15 ist die neue Schließen-Taste.
  while (keypadIndex == 14) {
    keypadIndex += step;
    if (keypadIndex < 0) keypadIndex = 16;
    if (keypadIndex > 16) keypadIndex = 0;
  }
  
  return true;
}

// Automatische Band-Auswahl mit Fallback, damit freie Eingaben nicht blockieren
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
  } else if (getTotalBands() > 0) {
    selectBand(0, false); 
  }
}

// Klick-Verarbeitung mit MHz/kHz-Berücksichtigung
bool clickFreqKeypad(bool shortPress)
{
  if (!shortPress) {
    currentCmd = CMD_NONE; 
    return true;
  }

  // Modus-Feld oben angewählt (Index 16)
  if (keypadIndex == 16) {
    tempMode = (tempMode + 1) % 4;
    inputInMHz = (tempMode == FM);
    freqRangeError = false;
    return true;
  }

  // Separate Schließen-Taste unten rechts.
  if (keypadIndex == 15) {
    currentCmd = CMD_NONE;
    return true;
  }

  // Index 14 bleibt leer.
  if (keypadIndex == 14) return true;

  const char* selectedKey = keypadLayout[keypadIndex];

  // UNIT-Schalter (Index 13) umschalten
  if (strcmp(selectedKey, "UNIT") == 0) {
    if(tempMode != FM) inputInMHz = !inputInMHz;
    else inputInMHz = true; // FM input is always entered in MHz.
    freqRangeError = false;
    return true;
  }

  if (strcmp(selectedKey, "OK") == 0) {
    if (inputLen > 0) {
      uint32_t targetFreq = 0;

      if (tempMode == FM) {
        float fVal = atof(inputFreqStr);
        if (fVal < 200.0f) {
          targetFreq = (uint32_t)(fVal * 100.0f);
        } else {
          targetFreq = (uint32_t)(fVal / 10.0f);
        }
      } else {
        float fVal = atof(inputFreqStr);
        if (inputInMHz) {
          targetFreq = (uint32_t)(fVal * 1000.0f);
        } else {
          targetFreq = atol(inputFreqStr);
        }
      }

      const bool dualRxSetup = (dualRxSetupStage == 1 || dualRxSetupStage == 2);
      if(dualRxSetup)
      {
        // Reject frequencies that do not fit any band supported by the
        // selected modulation. Do not accidentally store the old frequency.
        bool validTarget = false;
        for(int i = 0; i < getTotalBands(); ++i)
        {
          const Band &b = bands[i];
          const bool modeMatchesBand = (tempMode == FM) == (b.bandType == FM_BAND_TYPE);
          if(modeMatchesBand && targetFreq >= b.minimumFreq && targetFreq <= b.maximumFreq)
          {
            validTarget = true;
            break;
          }
        }
        if(!validTarget) { freqRangeError = true; return true; }
        freqRangeError = false;
      }

      switchToMatchingBand(targetFreq, tempMode);
      currentMode = tempMode;
      if(!updateFrequency(targetFreq, false)) return true;
      if (isSSB()) updateBFO(0, true);
      if(dualRxSetup)
      {
        dualRxKeypadComplete(captureVfo());
        return true;
      }
    }
    if((dualRxSetupStage == 1 || dualRxSetupStage == 2) && inputLen == 0)
    {
      currentCmd = CMD_DUAL_RX;
      return true;
    }
    currentCmd = CMD_NONE; 
    return true;
  }
  else if (strcmp(selectedKey, "BS") == 0) {
    freqRangeError = false;
    if (inputLen > 0) {
      inputFreqStr[--inputLen] = '\0';
    }
  }
  else {
    freqRangeError = false;
    if (inputLen < sizeof(inputFreqStr) - 1) {
      inputFreqStr[inputLen++] = selectedKey[0];
      inputFreqStr[inputLen] = '\0';
    }
  }
  return true;
}

// Zeichnen des Keypads mit modernen IFont-Zeigern (kompatibel mit -Werror)
void drawFreqKeypad()
{
  int boxX = 6;
  int boxY = 2;
  int boxW = 308;
  int boxH = 166;

  spr.fillRect(boxX, boxY, boxW, boxH, 0x0000);
  const uint16_t frameBlue = 0x001F;  // TEF6686 COLOR_BLUE
  const uint16_t brightBlue = 0x07FF; // TEF6686 COLOR_CYAN
  spr.drawRoundRect(boxX, boxY, boxW, boxH, 4, frameBlue);
  spr.drawRoundRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, 3, frameBlue);

  // Kleinere Überschrift, damit sie sauber in den Rahmen passt.
  spr.setFont(FONT_SMALL);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(brightBlue);
  spr.drawString("FREQUENCY INPUT MENU", boxX + boxW / 2, boxY + 3);

  // Frequenz-Anzeigefeld
  int displayH = 26;
  spr.fillRect(boxX + 8, boxY + 15, boxW - 100, displayH, 0x0000);
  spr.drawRoundRect(boxX + 8, boxY + 15, boxW - 100, displayH, 2, freqRangeError ? 0xF800 : frameBlue);
  
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(freqRangeError ? 0xF800 : brightBlue);
  
  char displayBuf[20];
  if (inputLen > 0) {
    snprintf(displayBuf, sizeof(displayBuf), "%s %s", inputFreqStr, inputInMHz ? "MHz" : "kHz");
  } else {
    snprintf(displayBuf, sizeof(displayBuf), "--- %s", inputInMHz ? "MHz" : "kHz");
  }
  
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString(displayBuf, boxX + 8 + (boxW - 100) / 2, boxY + 15 + displayH / 2);

  // Modus-Auswahltaste rechts oben (Index 16)
  int modeX = boxX + boxW - 88;
  int modeY = boxY + 15;
  int modeW = 80;
  int modeH = displayH;
  bool modeSelected = (keypadIndex == 16);

  if (modeSelected) {
    spr.fillRoundRect(modeX, modeY, modeW, modeH, 2, brightBlue);
    spr.setTextColor(0x0000);
  } else {
    spr.fillRoundRect(modeX, modeY, modeW, modeH, 2, 0x1084);
    spr.setTextColor(brightBlue);
  }
  spr.drawRoundRect(modeX, modeY, modeW, modeH, 2, frameBlue);
  spr.setTextDatum(MC_DATUM);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString(bandModeDesc[tempMode], modeX + modeW / 2, modeY + modeH / 2);

  // 4x4 Tastenfeld
  int startX = boxX + 8;
  int startY = boxY + 45;
  int btnW = 68;
  int btnH = 27;
  int gapX = 5;
  int gapY = 2;

  for (int i = 0; i < 16; i++) {
    if (i == 14) continue; // Index 14 bleibt frei; Index 15 ist CLOSE

    int col = i % 4;
    int row = i / 4;
    int x = startX + col * (btnW + gapX);
    int y = startY + row * (btnH + gapY);

    bool isSelected = (i == keypadIndex);

    if (isSelected) {
      spr.fillRoundRect(x, y, btnW, btnH, 3, brightBlue);
      spr.drawRoundRect(x, y, btnW, btnH, 3, 0xFFFF);
      spr.setTextColor(0x0000);                       
    } else {
      spr.fillRoundRect(x, y, btnW, btnH, 3, 0x0841); 
      spr.drawRoundRect(x, y, btnW, btnH, 3, frameBlue); 
      spr.drawRoundRect(x + 1, y + 1, btnW - 2, btnH - 2, 2, frameBlue); 
      spr.setTextColor(brightBlue);                       
    }

    spr.setTextDatum(MC_DATUM);
    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    
    if (i == 13) {
      spr.drawString(inputInMHz ? "MHz" : "kHz", x + btnW / 2, y + btnH / 2);
    } else {
      spr.drawString(keypadLayout[i], x + btnW / 2, y + btnH / 2);
    }
  }
}
