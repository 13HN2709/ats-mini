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

  // Draw band and mode
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

  // 1. Linke Menü-Box (Standard-Position)
  drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);

  // 2. Oben im Kasten den Titel "AMNVOLT" sauber überdecken und durch "13HN2709" in Orange (0xFD20) ersetzen
  spr.fillRect(MENU_OFFSET_X + 1, MENU_OFFSET_Y + 1, 74, 14, TH.bg);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFD20); // Orange
  spr.drawString("13HN2709", MENU_OFFSET_X + 38, MENU_OFFSET_Y + 2, FONT_SMALL);

  // 3. Falls unten durch Datum/Uhrzeit der Kasten zu lang wird oder unsauber in die Skala ragt:
  // Wir maskieren den Bereich unterhalb von AVC (ab Y = MENU_OFFSET_Y + 105) bis zur Skala mit der Hintergrundfarbe aus.
  // Das schneidet evtl. überstehende Uhrenzeilen ab und hält den Kasten kompakt über der Skala.
  // (Falls das bei dir genau passt, lässt es den Kasten unberührt).
  // spr.fillRect(MENU_OFFSET_X, MENU_OFFSET_Y + 108, 76, 20, TH.bg);
  // spr.drawRect(MENU_OFFSET_X, MENU_OFFSET_Y, 76, 108, TH.menu_border); // Rahmen unten schließen

  // H.J. Berndt S-Meter oben rechts
  drawSMeter(rx.getCurrentRSSI(), 180, 10); 

  // FM Stereo Indikator
  drawStereoIndicator(180, 10,
    (currentMode==FM) && (fmStereoIdx!=FM_STEREO_MONO) && rx.getCurrentPilot());

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
  else if(!drawStatus(STATUS_OFFSET_X, STATUS_OFFSET_Y))
  {
    // RDS-Ticker an alter Position & Röhrenskala unten
    drawRadioText(118);
    drawScale(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
}
