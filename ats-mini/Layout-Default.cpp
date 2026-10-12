#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

extern uint32_t lastTuneTime;

void drawLayoutDefault()
{
  // Draw preferences write request icon
  drawSaveIndicator(SAVE_OFFSET_X, SAVE_OFFSET_Y);

  // Draw BLE icon
  drawBleIndicator(BLE_OFFSET_X, BLE_OFFSET_Y);

  // Draw battery indicator & voltage
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // Draw WiFi icon
  drawWiFiIndicator(has_voltage ? WIFI_OFFSET_X : BATT_OFFSET_X - 13, WIFI_OFFSET_Y);

  // Set font we are going to use
  spr.setFont(&lgfx::fonts::Orbitron_Light_24);

  // Band und Modulationsart ("VHF FM") oben Mitte
  drawBandAndMode(
    getCurrentBand()->bandName,
    bandModeDesc[currentMode],
    BAND_OFFSET_X, BAND_OFFSET_Y
  );

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  // Draw frequency, units, and optionally highlight a digit
  drawFrequency(
    currentFrequency,
    FREQ_OFFSET_X, FREQ_OFFSET_Y,
    FUNIT_OFFSET_X, FUNIT_OFFSET_Y,
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100
  );

  // Show station or channel name, if present
  const char *stationName = getStationName();
  if(*stationName == 0xFF)
    drawLongStationName(stationName + 1, MENU_OFFSET_X + 1 + 76 + MENU_DELTA_X + 2, RDS_OFFSET_Y - 7);
  else if(*stationName)
    drawStationName(stationName, RDS_OFFSET_X, RDS_OFFSET_Y - 7);

  // Linke Menü-Box zeichnen
  drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);

  // Kasten-Kopf komplett schwarz wischen (20px hoch) und Rufzeichen zentriert einsetzen
  spr.fillRect(MENU_OFFSET_X + 1, MENU_OFFSET_Y + 1, 74, 18, TH.bg);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFD20); // Orange
  spr.drawString("13HN2709", MENU_OFFSET_X + 38, MENU_OFFSET_Y + 3, FONT_SMALL);

  // H.J. Berndt S-Meter unter der MHz-Anzeige (X=205, Y=40 schiebt es aus dem VHF-FM-Bereich raus)
  drawSMeter(rx.getCurrentRSSI(), 205, 40); 

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
  else if(!drawStatus(STATUS_OFFSET_X, STATUS_OFFSET_Y))
  {
    // RDS-Ticker & Röhrenskala unten
    drawRadioText(118);
    drawScale(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
}
