#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

void drawLayoutClean()
{
  // 1. Batterie & Spannung oben rechts
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Modus-Kasten links oben (z.B. FM) auf Y = 10
  int modeBoxX = 10;
  int modeBoxY = 10;
  int modeBoxW = 48;
  int modeBoxH = 20;

  spr.fillRoundRect(modeBoxX, modeBoxY, modeBoxW, modeBoxH, 5, 0x07E0);
  
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0x0000); // Schwarzer Text auf neongrünem Grund
  spr.drawString(bandModeDesc[currentMode], modeBoxX + (modeBoxW / 2), modeBoxY + (modeBoxH / 2));

  // 3. RDS-Sendername oder Bandname genau auf gleicher Höhe (Y = 10) in der Mitte
  const char* rdsStation = rx.getRdsStationName();
  bool hasRds = (currentMode == FM && rdsStation != nullptr && strlen(rdsStation) > 0);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0x07E0); // Neongrün
  
  if (hasRds) {
    spr.drawString(rdsStation, 120, 10);
  } else {
    spr.drawString(getCurrentBand()->bandName, 120, 10);
  }

  // 4. Große Frequenzanzeige in der Mitte (Orbitron-Font)
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(0xFFFF);

  char freqBuf[16];
  snprintf(freqBuf, sizeof(freqBuf), "%.2f", (float)currentFrequency / 100.0);
  
  spr.drawString(freqBuf, 120, 68);

  // Einheit (MHz) direkt unter dem hinteren Teil der Frequenz
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0x9CD3);
  spr.drawString("MHz", 175, 88);

  // 5. Unten Links: S-Wert & SNR
  uint8_t rssi = rx.getCurrentRSSI();
  uint8_t snr = rx.getCurrentSNR();
  uint8_t sVal = map(constrain(rssi, 0, 100), 0, 100, 0, 9);

  char sigBuf[32];
  snprintf(sigBuf, sizeof(sigBuf), "S%d  SNR %d", sVal, snr);
  
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0x9CD3);
  spr.drawString(sigBuf, 10, 100);

  // 6. S-Meter Balken ganz unten direkt UNTER dem S-Wert & SNR
  int barX = 10;
  int barY = 120;
  int maxBarW = 220;
  int barH = 8;

  int currentBarW = map(constrain(rssi, 0, 60), 0, 60, 5, maxBarW);

  // Hintergrund-Schiene (Dunkelgrau)
  spr.fillRoundRect(barX, barY, maxBarW, barH, 3, 0x2104);
  
  // Aktueller Signal-Ausschlag (Neongrün)
  if (currentBarW > 0) {
    spr.fillRoundRect(barX, barY, currentBarW, barH, 3, 0x07E0);
  }

  // Sidebar / Menü falls aufgerufen
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }
}
