#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

extern uint32_t lastTuneTime;

void drawLayoutNewUI()
{
  // Icons oben rechts (Akku, WiFi, BLE, Save)
  drawSaveIndicator(SAVE_OFFSET_X, SAVE_OFFSET_Y);
  drawBleIndicator(BLE_OFFSET_X, BLE_OFFSET_Y);
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);
  drawWiFiIndicator(has_voltage ? WIFI_OFFSET_X : BATT_OFFSET_X - 13, WIFI_OFFSET_Y);

  // 1. Band und Modus mittig oben (in Weiß)
  spr.setFont(&lgfx::fonts::Orbitron_Light_24);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFFFF);
  char bandModeBuf[32];
  snprintf(bandModeBuf, sizeof(bandModeBuf), "%s %s", getCurrentBand()->bandName, bandModeDesc[currentMode]);
  spr.drawString(bandModeBuf, 160, 8);

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  // 2. Frequenz mittig ausgerichtet
  drawFrequency(
    currentFrequency,
    185, 45,  // Frequenz-Ziffern
    190, 45,  // Einheit MHz / kHz
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100
  );

  // 3. Sendername / RDS direkt darunter zentriert
  const char *stationName = getStationName();
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0x07E0);
  if(*stationName == 0xFF)
    drawLongStationName(stationName + 1, 160, 92);
  else if(*stationName)
    drawStationName(stationName, 160, 92);

  // Menü / Sidebar bei Tastendruck
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }

  // 4. Schönes langes S-Meter ganz unten über die volle Breite
  int strength = rx.getCurrentRSSI();
  int smeterX = 20;
  int smeterY = 125;
  int smeterWidth = 280;
  
  // Rahmen für das lange S-Meter
  spr.drawRoundRect(smeterX, smeterY, smeterWidth, 14, 3, 0xFFFF);
  
  // Balken berechnen und zeichnen
  int maxBars = 40; 
  int activeBars = map(strength, 0, 60, 0, maxBars);
  activeBars = constrain(activeBars, 0, maxBars);
  
  int segmentWidth = (smeterWidth - 8) / maxBars;
  for (int i = 0; i < maxBars; i++) {
    uint16_t color = (i < activeBars) ? 0x07E0 : TH.smeter_bar_empty;
    spr.fillRect(smeterX + 4 + (i * segmentWidth), smeterY + 3, segmentWidth - 1, 8, color);
  }

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
  else if(!drawStatus(STATUS_OFFSET_X, STATUS_OFFSET_Y))
  {
    const char *rt = getRadioText();
    const char *pi = getProgramInfo();

    // Falls RDS-Lauftext vorhanden ist, wird er unten eingeblendet
    if((rt && *rt) || (pi && *pi))
    {
      spr.setTextColor(0x07E0);
      spr.setTextSize(1.0);
      spr.setTextDatum(TC_DATUM);
      spr.drawString(rt ? rt : pi, 160, 145, 4);
    }
  }
}
