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
  int box1W = 195;
  int box1H = 82;
  spr.drawRect(box1X, box1Y, box1W, box1H, 0xFFFF); 

  spr.drawFastHLine(box1X, box1Y + 30, box1W, 0xFFFF);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0xFFFF);
  spr.drawString(getCurrentBand()->bandName, box1X + 8, box1Y + 8);

  spr.setTextDatum(TR_DATUM);
  if (currentMode == FM) {
    bool isStereo = (fmStereoIdx != FM_STEREO_MONO) && rx.getCurrentPilot();
    spr.setTextColor(isStereo ? 0x07E0 : 0xF800); 
    spr.drawString(isStereo ? "Stereo" : "Mono", box1X + box1W - 8, box1Y + 8);
  } else {
    spr.setTextColor(0x07E0);
    spr.drawString(bandModeDesc[currentMode], box1X + box1W - 8, box1Y + 8);
  }

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

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
  spr.drawRect(box2X, box2Y, box2W, box2H, 0xFFFF); 
  
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


  // === KASTEN 3: Unten (SDR-Wasserfall-Layout mit festen Seitenrahmen) ===
  int wfX = 10;
  int wfY = 121;
  int wfW = 300;
  int wfH = 44;
  
  spr.drawRect(wfX, wfY, wfW, wfH, 0xFFFF);

  int sideWidth = 75; // Breite für die stabilen Seitenbänder links und rechts
  int midWidth = wfW - (sideWidth * 2); // Dynamische Mitte für den Wasserfall

  // 1. Äußere Seitenbereiche zeichnen (wie die farbigen Blöcke im Original-SDR)
  spr.fillRect(wfX + 1, wfY + 1, sideWidth, wfH - 2, 0x03EF); // Cyan/Blau linker Rand
  spr.fillRect(wfX + wfW - sideWidth - 1, wfY + 1, sideWidth, wfH - 2, 0x03EF); // Cyan/Blau rechter Rand

  // 2. Mittlerer Wasserfall-Bereich (schwarzer Grund)
  spr.fillRect(wfX + sideWidth, wfY + 1, midWidth, wfH - 2, 0x0000);

  // 3. Signalstärke-Auswertung für den aktuellen Kanal
  uint8_t currentRssi = rx.getCurrentRSSI();
  uint16_t lineCol = 0x001F; // Dunkles Blau
  if (currentRssi > 5)  lineCol = 0x04FF; // Hellblau
  if (currentRssi > 15) lineCol = 0x07E0; // Grün
  if (currentRssi > 25) lineCol = 0xFFE0; // Gelb
  if (currentRssi > 35) lineCol = 0xF800; // Rot bei starkem Signal

  // Zeichne den Pegel als fokussierten Balken in der Mitte des Wasserfalls
  int barW = map(currentRssi, 0, 45, 4, midWidth - 8);
  barW = constrain(barW, 4, midWidth - 8);
  int barX = wfX + sideWidth + (midWidth - barW) / 2;
  
  spr.fillRect(barX, wfY + 3, barW, wfH - 6, lineCol);

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
}
