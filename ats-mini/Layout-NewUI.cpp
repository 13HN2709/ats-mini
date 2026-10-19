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

  // 1. Band und Modus (mittig oben)
  spr.setFont(&lgfx::fonts::Orbitron_Light_24);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFFFF);
  char bandModeBuf[32];
  snprintf(bandModeBuf, sizeof(bandModeBuf), "%s %s", getCurrentBand()->bandName, bandModeDesc[currentMode]);
  spr.drawString(bandModeBuf, 145, 8);

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  // 2. Frequenz & Einheit (mit Original-Offsets aus dem Default-Layout, leicht nach links)
  drawFrequency(
    currentFrequency,
    FREQ_OFFSET_X - 15, FREQ_OFFSET_Y,
    FUNIT_OFFSET_X - 15, FUNIT_OFFSET_Y,
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100
  );

  // 3. Sendername / RDS direkt darunter zentriert
  const char *stationName = getStationName();
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0x07E0);
  if(*stationName == 0xFF)
    drawLongStationName(stationName + 1, 145, 102);
  else if(*stationName)
    drawStationName(stationName, 145, 102);

  // Menü / Sidebar bei Tastendruck
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }

  // 4. S-Meter: Angepasster Skalenbereich für kraftvollen Ausschlag bei FM
  int strength = rx.getCurrentRSSI();
  int smeterWidth = 296;
  int smeterX = (320 - smeterWidth) / 2; // Exakt zentriert
  int smeterY = 132;
  int smeterHeight = 22;
  int centerX = smeterX + (smeterWidth / 2);
  
  spr.drawRoundRect(smeterX, smeterY, smeterWidth, smeterHeight, 4, 0xFFFF);
  
  int pairsPerSide = 19;
  
  // Da FM-Ortssender ca. bei RSSI ~30 liegen, setzen wir maxRssi auf 35 für vollen Ausschlag
  int maxRssi = (currentMode == FM) ? 35 : 45; 
  int activePairs = map(strength, 0, maxRssi, 0, pairsPerSide);
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
      spr.fillRect(leftX, smeterY + 3, boxWidth, smeterHeight - 6, color);
    } else {
      spr.fillRect(leftX, smeterY + 3, boxWidth, smeterHeight - 6, TH.smeter_bar_empty);
    }

    // Rechte Seite
    int rightX = (centerX + 2) + (i * (boxWidth + spacing));
    if (i < activePairs) {
      spr.fillRect(rightX, smeterY + 3, boxWidth, smeterHeight - 6, color);
    } else {
      spr.fillRect(rightX, smeterY + 3, boxWidth, smeterHeight - 6, TH.smeter_bar_empty);
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
      spr.setTextDatum(TC_DATUM);
      // Modernisierter Font-Aufruf statt veraltetem Index '4'
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.drawString(rt ? rt : pi, 145, 150);
    }
  }
}
