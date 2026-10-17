#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

void drawLayoutClean()
{
  // 1. Batterie & Spannung oben rechts
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Oben Links: Automatischer RDS-Wechsel (Sendename vs. RDS-Text) oder Bandname
  static uint32_t lastRdsToggle = 0;
  static bool showRdsText = false;
  
  // Alle 4 Sekunden zwischen Stationsname und RDS-Text umschalten (bei UKW / FM)
  if (millis() - lastRdsToggle > 4000) {
    lastRdsToggle = millis();
    showRdsText = !showRdsText;
  }

  // Prüfen, ob RDS-Daten vorhanden sind (über die Receiver-API / rx-Objekt)
  const char* rdsStation = rx.getRdsStationName(); // Stationsname (PS)
  const char* rdsText = rx.getRdsText();         // Radiotext (RT)
  
  bool hasRds = (currentMode == FM && rdsStation != nullptr && strlen(rdsStation) > 0);

  // Styling oben links (VHF / FM-Bereich)
  int modeBoxX = 10;
  int modeBoxY = 28;
  int modeBoxW = 48;
  int modeBoxH = 20;

  if (hasRds) {
    // Wenn RDS da ist, zeigen wir den Sendernamen oder den Lauftext im Wechsel
    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(0x07E0); // Neongrün

    if (showRdsText && rdsText != nullptr && strlen(rdsText) > 0) {
      // Zeige RDS-Radiotext
      spr.drawString(rdsText, 10, 5);
    } else {
      // Zeige RDS-Stationsnamen groß oben
      spr.drawString(rdsStation, 10, 5);
    }
  } else {
    // Standard-Fallback, wenn kein RDS empfangen wird: Band-Name (z.B. VHF)
    spr.setFont(&lgfx::fonts::FreeSansBold12pt7b);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(0x07E0); // Neongrün
    spr.drawString(getCurrentBand()->bandName, 10, 5);
  }

  // Modus in abgerundetem neongrünen Kasten (Button-Style für FM)
  spr.fillRoundRect(modeBoxX, modeBoxY, modeBoxW, modeBoxH, 5, 0x07E0);
  
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0x0000); // Schwarzer Text auf neongrünem Grund
  spr.drawString(bandModeDesc[currentMode], modeBoxX + (modeBoxW / 2), modeBoxY + (modeBoxH / 2));

  // 3. Große Frequenzanzeige in der Mitte (Orbitron-Font)
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(0xFFFF);

  char freqBuf[16];
  snprintf(freqBuf, sizeof(freqBuf), "%.2f", (float)currentFrequency / 100.0);
  
  spr.drawString(freqBuf, 120, 75);

  // Einheit (MHz / kHz) direkt rechts unter/neben der Frequenz
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0x9CD3);
  spr.drawString("MHz", 205, 90);

  // 4. Unten Links: S-Wert & SNR
  uint8_t rssi = rx.getCurrentRSSI();
  uint8_t snr = rx.getCurrentSNR();
  uint8_t sVal = map(constrain(rssi, 0, 100), 0, 100, 0, 9);

  char sigBuf[32];
  snprintf(sigBuf, sizeof(sigBuf), "S%d  SNR %d", sVal, snr);
  
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(BL_DATUM);
  spr.setTextColor(0x9CD3);
  spr.drawString(sigBuf, 10, 125);

  // 5. Unten: Abgerundeter grüner S-Meter Balken
  int barX = 95;
  int barY = 115;
  int maxBarW = 105;
  int barH = 10;

  int currentBarW = map(constrain(rssi, 0, 60), 0, 60, 5, maxBarW);

  // Hintergrund-Schiene (Dunkelgrau)
  spr.fillRoundRect(barX, barY, maxBarW, barH, 4, 0x2104);
  
  // Aktueller Signal-Ausschlag (Neongrün)
  if (currentBarW > 0) {
    spr.fillRoundRect(barX, barY, currentBarW, barH, 4, 0x07E0);
  }

  // Sidebar / Menü falls aufgerufen
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }
}
