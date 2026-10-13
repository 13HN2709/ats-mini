#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

extern uint32_t lastTuneTime;

void drawLayoutNewUI()
{
  // Bildschirm mit der aktuellen Hintergrundfarbe des Themes füllen
  spr.fillSprite(TH.bg);
  // Preferences write request icon
  drawSaveIndicator(SAVE_OFFSET_X, SAVE_OFFSET_Y);

  // BLE icon
  drawBleIndicator(BLE_OFFSET_X, BLE_OFFSET_Y);

  // Battery indicator & voltage
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // WiFi icon
  drawWiFiIndicator(has_voltage ? WIFI_OFFSET_X : BATT_OFFSET_X - 13, WIFI_OFFSET_Y);

  // Band und Modulationsart ("VHF FM")
  drawBandAndMode(
    getCurrentBand()->bandName,
    bandModeDesc[currentMode],
    120, BAND_OFFSET_Y
  );

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  spr.setFont(&lgfx::fonts::Orbitron_Light_24);

  // Frequenz, Einheiten und Ziffern-Highlight
  drawFrequency(
    currentFrequency,
    FREQ_OFFSET_X, FREQ_OFFSET_Y,
    FUNIT_OFFSET_X, FUNIT_OFFSET_Y,
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100
  );

  // Stations- oder Kanalname (RDS), falls vorhanden
  const char *stationName = getStationName();
  if(*stationName == 0xFF)
    drawLongStationName(stationName + 1, MENU_OFFSET_X + 1 + 76 + MENU_DELTA_X + 2, RDS_OFFSET_Y - 7);
  else if(*stationName)
    drawStationName(stationName, RDS_OFFSET_X, RDS_OFFSET_Y - 7);

  // Seitenleiste / Menü für NewUI zeichnen, damit es nicht mehr unsichtbar bleibt
  drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);

  // Kompaktes S-Meter
  drawSMeter(rx.getCurrentRSSI(), 195, 42); 

  // FM Stereo Indikator
  drawStereoIndicator(195, 42,
    (currentMode==FM) && (fmStereoIdx!=FM_STEREO_MONO) && rx.getCurrentPilot());
}
