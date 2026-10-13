#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

extern uint32_t lastTuneTime;

void drawLayoutNewUI()
{
  // Draw preferences write request icon
  drawSaveIndicator(SAVE_OFFSET_X, SAVE_OFFSET_Y);

  // Draw BLE icon
  drawBleIndicator(BLE_OFFSET_X, BLE_OFFSET_Y);

  // Draw battery indicator & voltage
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // Draw WiFi icon
  drawWiFiIndicator(has_voltage ? WIFI_OFFSET_X : BATT_OFFSET_X - 13, WIFI_OFFSET_Y);

  // H.J. Berndt S-Meter oben rechts (Y = 18, frei von der Frequenz-Einheit)
  drawCompactSMeter(rx.getCurrentRSSI(), 215, 18);

  // Set font we are going to use
  spr.setFont(&lgfx::fonts::Orbitron_Light_24);

  // Band und Modus linksbündig oben
  drawBandAndMode(
    getCurrentBand()->bandName,
    bandModeDesc[currentMode],
    55, 15
  );

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  // Frequenz und Einheit sauber platziert
  drawFrequency(
    currentFrequency,
    155, 65,  // Frequenz-Ziffern
    160, 65,  // Einheit MHz/kHz
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100
  );

  // Sendername (z.B. JVE) sitzt jetzt sauber bündig unter der Frequenz bei Y = 110
  const char *stationName = getStationName();
  if(*stationName == 0xFF)
    drawLongStationName(stationName + 1, 15, 110);
  else if(*stationName)
    drawStationName(stationName, 15, 110);

  // Menü / Sidebar einblenden, wenn die Menütaste gedrückt wird
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

    // Langer Radiotext / Programminfo wandert nach unten (Y = 135), getrennt vom Sendername
    if((rt && *rt) || (pi && *pi))
    {
      spr.setTextColor(0x07E0);
      spr.setTextSize(1.0);
      spr.setTextDatum(TL_DATUM);
      spr.drawString(rt ? rt : pi, 15, 135, 4);
    }
  }
}
