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

  // === KASTEN 1: Oben Links (Band, Modus & Frequenz) ===
  int box1X = 10;
  int box1Y = 35;
  int box1W = 200;
  int box1H = 85;
  spr.drawRect(box1X, box1Y, box1W, box1H, 0xFFFF); // Rahmen Kasten 1

  // Band und Modus
  spr.setFont(&lgfx::fonts::Orbitron_Light_24);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFFFF);
  char bandModeBuf[32];
  snprintf(bandModeBuf, sizeof(bandModeBuf), "%s %s", getCurrentBand()->bandName, bandModeDesc[currentMode]);
  spr.drawString(bandModeBuf, box1X + 100, box1Y + 6);

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  // Mono/Stereo Anzeige
  spr.setTextDatum(TC_DATUM);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  if (currentMode == FM) {
    bool isStereo = (fmStereoIdx != FM_STEREO_MONO) && rx.getCurrentPilot();
    spr.setTextColor(isStereo ? 0x07E0 : 0xF800); 
    spr.drawString(isStereo ? "Stereo" : "Mono", box1X + 100, box1Y + 30);
  } else {
    spr.setTextColor(0x8410); 
    spr.drawString(bandModeDesc[currentMode], box1X + 100, box1Y + 30);
  }

  // Frequenz & Einheit
  drawFrequency(
    currentFrequency,
    box1X + 50, box1Y + 58,
    box1X + 165, box1Y + 58,
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100
  );


  // === KASTEN 2: Oben Rechts (S:, SNR:, Uhrzeit/Datum) ===
  int box2X = 215;
  int box2Y = 35;
  int box2W = 95;
  int box2H = 85;
  spr.drawRect(box2X, box2Y, box2W, box2H, 0xFFFF); // Rahmen Kasten 2
  
  // Trennlinie in Kasten 2
  spr.drawFastHLine(box2X, box2Y + 45, box2W, 0xFFFF);

  spr.setTextDatum(TL_DATUM);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextColor(0xFFFF);
  
  char rssiBuf[16], snrBuf[16];
  snprintf(rssiBuf, sizeof(rssiBuf), "S:%d", rx.getCurrentRSSI());
  snprintf(snrBuf, sizeof(snrBuf), "SNR:%ddB", rx.getCurrentSNR());
  
  spr.drawString(rssiBuf, box2X + 8, box2Y + 8);
  spr.drawString(snrBuf, box2X + 8, box2Y + 26);

  uint8_t hour, minute;
  clockGetHM(&hour, &minute);
  char timeBuf[16];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
  spr.drawString(timeBuf, box2X + 8, box2Y + 55);

  // Menü / Sidebar bei Tastendruck
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }


  // === KASTEN 3: Unten (Eco-Waterfall) ===
  int wfX = 10;
  int wfY = 125;
  int wfW = 300;
  int wfH = 95;
  
  // Äußeren Kasten für den Wasserfall zeichnen
  spr.drawRect(wfX, wfY, wfW, wfH, 0xFFFF);

  // Linker und rechter statischer blauer Bereich
  int midStartRel = 90;
  int midWidth = 60;
  
  spr.fillRect(wfX, wfY, midStartRel, wfH, 0x001F); // Dunkles Blau links
  spr.fillRect(wfX + midStartRel + midWidth, wfY, wfW - (midStartRel + midWidth), wfH, 0x001F); // Dunkles Blau rechts

  // Dynamischer Mittelstreifen
  uint8_t currentRssi = rx.getCurrentRSSI();
  uint16_t sigColor = (currentRssi < 5) ? 0x0000 : (currentRssi < 20 ? 0xF800 : (currentRssi < 30 ? 0xFFE0 : 0x07E0));
  
  spr.fillRect(wfX + midStartRel, wfY, midWidth, wfH, 0x0000); // Hintergrund Mitte Schwarz
  int barH = map(currentRssi, 0, 45, 0, wfH);
  barH = constrain(barH, 0, wfH);
  spr.fillRect(wfX + midStartRel + 10, wfY + wfH - barH, midWidth - 20, barH, sigColor);

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
}
