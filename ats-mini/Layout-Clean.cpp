#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

void drawLayoutClean()
{
  // 1. Batterie & Spannung oben rechts
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Oben Links: Bandname (z.B. VHF) ganz oben
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0x07E0); // Neongrün
  spr.drawString(getCurrentBand()->bandName, 10, 2);

  // Modus-Kasten (FM) direkt UNTER dem Bandnamen (2. Zeile)
  int modeBoxX = 10;
  int modeBoxY = 20;
  int modeBoxW = 48;
  int modeBoxH = 18;

  spr.fillRoundRect(modeBoxX, modeBoxY, modeBoxW, modeBoxH, 4, 0x07E0);
  
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0x0000); // Schwarzer Text auf neongrünem Grund
  spr.drawString(bandModeDesc[currentMode], modeBoxX + (modeBoxW / 2), modeBoxY + (modeBoxH / 2));

  // 3. RDS-Sendername in derselben Höhe wie FM (2. Zeile), aber genau über der Frequenz zentriert
  const char* rdsStation = rx.getRdsStationName();
  bool hasRds = (currentMode == FM && rdsStation != nullptr && strlen(rdsStation) > 0);

  if (hasRds) {
    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(0x07E0); // Neongrün
    spr.drawString(rdsStation, 120, 20);
  }

  // 4. Frequenzanzeige mit Orbitron-Font und angepasster Formatierung
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(0xFFFF);

  char freqBuf[16];
  const char* unitStr = "kHz";

  if (currentMode == FM) {
    // FM: Immer mit Punkt und 2 Nachkommastellen (z.B. 107.90)
    snprintf(freqBuf, sizeof(freqBuf), "%.2f", (float)currentFrequency / 100.0);
    unitStr = "MHz";
  } else {
    // Kurzwelle / CB / Andere: Ganze kHz-Zahl (mit optionaler Feinabstimmung falls vorhanden)
    snprintf(freqBuf, sizeof(freqBuf), "%d", currentFrequency);
    unitStr = "kHz";
  }
  
  // Frequenz perfekt in der Mitte zentriert
  spr.drawString(freqBuf, 120, 65);

  // Einheit (MHz / kHz) rechtsbündig daneben platziert
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TR_DATUM);
  spr.setTextColor(0x9CD3);
  spr.drawString(unitStr, 225, 75);

  // 5. Unten: S-Wert & SNR (zentriert)
  uint8_t rssi = rx.getCurrentRSSI();
  uint8_t snr = rx.getCurrentSNR();
  uint8_t sVal = map(constrain(rssi, 0, 100), 0, 100, 0, 9);

  char sigBuf[32];
  snprintf(sigBuf, sizeof(sigBuf), "S%d  SNR %d", sVal, snr);
  
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0x9CD3);
  spr.drawString(sigBuf, 120, 104);

  // 6. S-Meter Balken zentriert und perfekt unter S/SNR platziert
  int barX = 20;
  int barY = 118;
  int maxBarW = 200;
  int barH = 7;

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
