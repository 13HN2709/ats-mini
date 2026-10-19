#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// --------------------------------------------------
// ATS Mini V4 - Clean Layout (Optimiert für 240x135)
// --------------------------------------------------

void drawLayoutClean()
{
  const uint16_t COLOR_GREEN  = 0x07E0;
  const uint16_t COLOR_WHITE  = 0xFFFF;
  const uint16_t COLOR_BLACK  = 0x0000;
  const uint16_t COLOR_UNIT   = 0x9CD3;
  const uint16_t COLOR_GREY   = 0x2104;

  // 1. Batterie oben rechts
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Oben Links: Bandname (z.B. VHF)
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(getCurrentBand()->bandName, 10, 2);

  // 3. Modus-Kasten (FM) direkt unter dem Bandnamen
  const int modeBoxX = 10;
  const int modeBoxY = 20;
  const int modeBoxW = 48;
  const int modeBoxH = 18;

  spr.fillRoundRect(modeBoxX, modeBoxY, modeBoxW, modeBoxH, 4, COLOR_GREEN);
  
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_BLACK);
  spr.drawString(bandModeDesc[currentMode], modeBoxX + (modeBoxW / 2), modeBoxY + (modeBoxH / 2));

  // 4. RDS-Sendername (PS) genau in der Mitte ÜBER der Frequenz
  if (currentMode == FM)
  {
    const char *stationName = getStationName();
    if (stationName != nullptr *stationName)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(TC_DATUM);
      spr.setTextColor(COLOR_GREEN);

      if ((uint8_t)*stationName == 0xFF)
      {
        drawLongStationName(stationName + 1, 120, 20);
      }
      else
      {
        spr.setClipRect(65, 12, 110, 20);
        spr.drawString(stationName, 120, 20);
        spr.clearClipRect();
      }
    }
  }

  // 5. Frequenzanzeige (mit Orbitron-Font und automatischem MHz/kHz Wechsel)
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(COLOR_WHITE);

  char freqBuf[16];
  const char* unitStr = "kHz";

  if (currentMode == FM) {
    snprintf(freqBuf, sizeof(freqBuf), "%.2f", (float)currentFrequency / 100.0);
    unitStr = "MHz";
  } else {
    snprintf(freqBuf, sizeof(freqBuf), "%d", currentFrequency);
    unitStr = "kHz";
  }
  
  spr.drawString(freqBuf, 115, 62);

  // Einheit (MHz / kHz) rechtsbündig daneben
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_UNIT);
  spr.drawString(unitStr, 185, 72);

  // 6. Neongrüner Kasten für den RDS-Radiotext (RT)
  const int rtBoxX = 20;
  const int rtBoxY = 92;
  const int rtBoxW = 200;
  const int rtBoxH = 24;

  spr.fillRoundRect(rtBoxX, rtBoxY, rtBoxW, rtBoxH, 4, COLOR_BLACK);
  spr.drawRoundRect(rtBoxX, rtBoxY, rtBoxW, rtBoxH, 4, COLOR_GREEN);

  if (currentMode == FM)
  {
    const char *rt = getRadioText();
    if (rt != nullptr *rt)
    {
      spr.setFont(&lgfx::fonts::FreeSans9pt7b);
      spr.setTextColor(COLOR_WHITE);
      spr.setTextDatum(TL_DATUM);

      spr.setClipRect(rtBoxX + 4, rtBoxY + 2, rtBoxW - 8, rtBoxH - 4);
      spr.drawString(rt, rtBoxX + 6, rtBoxY + 4);
      spr.clearClipRect();
    }
  }

  // 7. S-Meter Balken ganz unten
  int barX = 20;
  int barY = 124;
  int maxBarW = 200;
  int barH = 6;

  int currentBarW = map(constrain(rssi, 0, 60), 0, 60, 5, maxBarW);

  spr.fillRoundRect(barX, barY, maxBarW, barH, 3, COLOR_GREY);
  if (currentBarW > 0) {
    spr.fillRoundRect(barX, barY, currentBarW, barH, 3, COLOR_GREEN);
  }

  // 8. Sidebar / Menü falls aufgerufen
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }
}
