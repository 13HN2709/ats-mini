#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

extern uint32_t lastTuneTime;

void drawLayoutDecoder()
{
  // Icons oben rechts (Akku, WiFi, BLE, Save)
  drawSaveIndicator(SAVE_OFFSET_X, SAVE_OFFSET_Y);
  drawBleIndicator(BLE_OFFSET_X, BLE_OFFSET_Y);
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);
  drawWiFiIndicator(has_voltage ? WIFI_OFFSET_X : BATT_OFFSET_X - 13, WIFI_OFFSET_Y);

  // === KASTEN 1: Oben Links (Mit fester Unterteilung für Band/Modus & Frequenz) ===
  int box1X = 10;
  int box1Y = 35;
  int box1W = 195;
  int box1H = 82;
  spr.drawRect(box1X, box1Y, box1W, box1H, 0xFFFF); // Äußerer Rahmen Kasten 1

  // Interne Trennlinie in Kasten 1 (teilt Oben [Band/Modus] und Unten [Frequenz])
  spr.drawFastHLine(box1X, box1Y + 30, box1W, 0xFFFF);

  // 1. Oberer Bereich von Kasten 1: Bandname links, Modus/Stereo rechts
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  
  // Bandname links platziert
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0xFFFF);
  spr.drawString(getCurrentBand()->bandName, box1X + 8, box1Y + 8);

  // Modus oder Stereo/Mono rechts platziert
  spr.setTextDatum(TR_DATUM);
  if (currentMode == FM) {
    bool isStereo = (fmStereoIdx != FM_STEREO_MONO) && rx.getCurrentPilot();
    spr.setTextColor(isStereo ? 0x07E0 : 0xF800); 
    spr.drawString(isStereo ? "Stereo" : "Mono", box1X + box1W - 8, box1Y + 8);
  } else {
    spr.setTextColor(0x07E0); // Schönes Grün für den Modus (USB/LSB/AM)
    spr.drawString(bandModeDesc[currentMode], box1X + box1W - 8, box1Y + 8);
  }

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  // 2. Unterer Bereich von Kasten 1: Frequenzanzeige sauber zentriert
  drawFrequency(
    currentFrequency,
    box1X + 45, box1Y + 45,
    box1X + 155, box1Y + 45,
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : getFreqInputPos()
  );


  // === KASTEN 2: Oben Rechts (S:, SNR:, Uhrzeit) ===
  int box2X = 210;
  int box2Y = 35;
  int box2W = 100;
  int box2H = 82;
  spr.drawRect(box2X, box2Y, box2W, box2H, 0xFFFF); // Rahmen Kasten 2
  
  spr.drawFastHLine(box2X, box2Y + 42, box2W, 0xFFFF);

  spr.setTextDatum(TL_DATUM);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextColor(0xFFFF);
  
  char rssiBuf[16], snrBuf[16];
  snprintf(rssiBuf, sizeof(rssiBuf), "S:%d", rx.getCurrentRSSI());
  snprintf(snrBuf, sizeof(snrBuf), "SNR:%ddB", rx.getCurrentSNR());
  
  spr.drawString(rssiBuf, box2X + 6, box2Y + 6);
  spr.drawString(snrBuf, box2X + 6, box2Y + 22);

  uint8_t hour, minute;
  clockGetHM(&hour, &minute);
  char timeBuf[16];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
  spr.drawString(timeBuf, box2X + 6, box2Y + 50);

  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }


  // === KASTEN 3: Unten (Eco-Waterfall, passgenau für 170px Display-Höhe) ===
  int wfX = 10;
  int wfY = 121;
  int wfW = 300;
  int wfH = 44;
  
  spr.drawRect(wfX, wfY, wfW, wfH, 0xFFFF);

  int midStartRel = 90;
  int midWidth = 60;
  
  spr.fillRect(wfX, wfY, midStartRel, wfH, 0x001F); // Dunkles Blau links
  spr.fillRect(wfX + midStartRel + midWidth, wfY, wfW - (midStartRel + midWidth), wfH, 0x001F); // Dunkles Blau rechts

  uint8_t currentRssi = rx.getCurrentRSSI();
  
  // Sichtbare Farbe erzwingen zum Testen
  uint16_t sigColor = 0x07E0; 
  if (currentRssi > 10) sigColor = 0xFFE0; // Gelb
  if (currentRssi > 25) sigColor = 0xF800; // Rot

  spr.fillRect(wfX + midStartRel, wfY, midWidth, wfH, 0x0000); // Hintergrund Mitte Schwarz
  
  int barH = map(currentRssi, 0, 45, 4, wfH);
  barH = constrain(barH, 4, wfH);
  
  spr.fillRect(wfX + midStartRel + 10, wfY + wfH - barH, midWidth - 20, barH, sigColor);

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
}
