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

  // 2. Frequenz & Einheit: Perfekt aufeinander abgestimmt, damit .000 unter kHz landet
  drawFrequency(
    currentFrequency,
    175, 56,  // Frequenz-Ziffern
    230, 52,  // Einheit MHz / kHz
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100
  );

  // 3. Sendername / RDS direkt darunter zentriert
  const char *stationName = getStationName();
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0x07E0);
  if(*stationName == 0xFF)
    drawLongStationName(stationName + 1, 160, 102);
  else if(*stationName)
    drawStationName(stationName, 160, 102);

  // Menü / Sidebar bei Tastendruck
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }

  // 4. S-Meter: Exakt zentriert (Breite 280, Start = 20, Mitte = 160), Höhe = 24
  int strength = rx.getCurrentRSSI();
  int smeterWidth = 280;
  int smeterX = (320 - smeterWidth) / 2; // Automatisch exakt zentriert (20)
  int smeterY = 132;
  int smeterHeight = 24;
  int centerX = smeterX + (smeterWidth / 2); // Exakt 160
  
  spr.drawRoundRect(smeterX, smeterY, smeterWidth, smeterHeight, 4, 0xFFFF);
  
  int pairsPerSide = 20;
  int activePairs = map(strength, 0, 60, 0, pairsPerSide);
  activePairs = constrain(activePairs, 0, pairsPerSide);

  int boxWidth = 5;
  int spacing = 2;

  for (int i = 0; i < pairsPerSide; i++) {
    uint16_t color;
    if (i < pairsPerSide / 3) color = 0x07E0;      // Grün
    else if (i < (pairsPerSide * 2) / 3) color = 0xFFE0; // Gelb
    else color = 0xF800;                           // Rot

    // Linke Seite
    int leftX = (centerX - 3) - (i * (boxWidth + spacing));
    if (i < activePairs) {
      spr.fillRect(leftX, smeterY + 4, boxWidth, smeterHeight - 8, color);
    } else {
      spr.fillRect(leftX, smeterY + 4, boxWidth, smeterHeight - 8, TH.smeter_bar_empty);
    }

    // Rechte Seite
    int rightX = (centerX + 2) + (i * (boxWidth + spacing));
    if (i < activePairs) {
      spr.fillRect(rightX, smeterY + 4, boxWidth, smeterHeight - 8, color);
    } else {
      spr.fillRect(rightX, smeterY + 4, boxWidth, smeterHeight - 8, TH.smeter_bar_empty);
    }
  }

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
  else if(!drawStatus(STATUS_OFFSET_X, STATUS_OFFSET_Y))
  {
    const char *rt = getRadioText();
    const char *pi = getProgramInfo();

    if((rt && *rt) || (pi && *pi))
    {
      spr.setTextColor(0x07E0);
      spr.setTextSize(1.0);
      spr.setTextDatum(TC_DATUM);
      spr.drawString(rt ? rt : pi, 160, 150, 4);
    }
  }
}
