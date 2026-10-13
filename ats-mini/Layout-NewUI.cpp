#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

extern uint32_t lastTuneTime;

void drawLayoutNewUI()
{
  // Icons oben rechts
  drawSaveIndicator(SAVE_OFFSET_X, SAVE_OFFSET_Y);
  drawBleIndicator(BLE_OFFSET_X, BLE_OFFSET_Y);
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);
  drawWiFiIndicator(has_voltage ? WIFI_OFFSET_X : BATT_OFFSET_X - 13, WIFI_OFFSET_Y);

  // H.J. Berndt S-Meter oben rechts (Y = 8, damit es perfekt unter der Akkuzeile sitzt)
  drawCompactSMeter(rx.getCurrentRSSI(), 215, 8);

  // Band und Modus links oben
  spr.setFont(&lgfx::fonts::Orbitron_Light_24);
  drawBandAndMode(
    getCurrentBand()->bandName,
    bandModeDesc[currentMode],
    15, 12
  );

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  // Frequenz und Einheit
  drawFrequency(
    currentFrequency,
    150, 55,  // Frequenz-Ziffern
    155, 55,  // Einheit MHz/kHz
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100
  );

  // Sendername (z.B. 1Live / JVE) sauber bündig unter der Frequenz links platziert (X = 15, Y = 105)
  const char *stationName = getStationName();
  if(*stationName == 0xFF)
    drawLongStationName(stationName + 1, 15, 105);
  else if(*stationName)
    drawStationName(stationName, 55, 105);

  // Menü / Sidebar bei Tastendruck
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
  else if(!drawStatus(STATUS_OFFSET_X, STATUS_OFFSET_Y))
  {
    const char *rt = getRadioText();
    const char *pi = getProgramInfo();

    // Radiotext getrennt ganz unten
    if((rt && *rt) || (pi && *pi))
    {
      spr.setTextColor(0x07E0);
      spr.setTextSize(1.0);
      spr.setTextDatum(TL_DATUM);
      spr.drawString(rt ? rt : pi, 15, 135, 4);
    }
  }
}
