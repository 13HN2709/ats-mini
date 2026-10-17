#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

void drawLayoutClean()
{
  // 1. Batterie & Spannung oben rechts
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Oben Links: Band-Name (z.B. VHF)
  spr.setFont(&lgfx::fonts::FreeSansBold12pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0x3A5F); // Schönes Blau/Cyanton
  spr.drawString(getCurrentBand()->bandName, 10, 8);

  // Modus in abgerundetem blauen Kasten (Button-Style wie FM)
  int modeBoxX = 10;
  int modeBoxY = 32;
  int modeBoxW = 48;
  int modeBoxH = 22;

  spr.fillRoundRect(modeBoxX, modeBoxY, modeBoxW, modeBoxH, 6, 0x001F); // Blauer Hintergrund
  
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0xFFFF); // Weißer Text
  spr.drawString(bandModeDesc[currentMode], modeBoxX + (modeBoxW / 2), modeBoxY + (modeBoxH / 2));

  // 3. Große Frequenzanzeige in der Mitte (Orbitron-Font)
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(0xFFFF);

  char freqBuf[16];
  if (currentFrequency >= 30000) {
    snprintf(freqBuf, sizeof(freqBuf), "%.2f", (float)currentFrequency / 1000.0);
  } else {
    snprintf(freqBuf, sizeof(freqBuf), "%lu", currentFrequency);
  }
  
  spr.drawString(freqBuf, 145, 95);

  // Einheit (MHz / kHz) rechts neben/unter der Frequenz
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0x9CD3);
  spr.drawString((currentFrequency >= 30000) ? "MHz" : "kHz", 215, 110);

  // 4. Unten Links: S-Wert & SNR
  uint8_t rssi = rx.getCurrentRSSI();
  uint8_t snr = rx.getCurrentSNR();
  uint8_t sVal = map(constrain(rssi, 0, 100), 0, 100, 0, 9);

  char sigBuf[32];
  snprintf(sigBuf, sizeof(sigBuf), "S%d  SNR %d", sVal, snr);
  
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(BL_DATUM);
  spr.setTextColor(0x9CD3);
  spr.drawString(sigBuf, 10, 195);

  // 5. Unten: Abgerundeter grüner S-Meter Balken
  int barX = 70;
  int barY = 202;
  int maxBarW = 110;
  int barH = 12;

  int currentBarW = map(constrain(rssi, 0, 60), 0, 60, 8, maxBarW);

  // Hintergrund-Schiene (Dunkelgrau)
  spr.fillRoundRect(barX, barY, maxBarW, barH, 5, 0x2104);
  
  // Aktueller Signal-Ausschlag (Neongrün)
  if (currentBarW > 0) {
    spr.fillRoundRect(barX, barY, currentBarW, barH, 5, 0x07E0);
  }

  // Sidebar / Menü falls aufgerufen
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }
}

