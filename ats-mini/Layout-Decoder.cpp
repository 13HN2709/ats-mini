#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

extern uint32_t lastTuneTime;

static LGFX_Sprite wfSpr(&tft);
static bool wfInitialized = false;

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

  // Trennlinie etwas weiter nach oben gesetzt, damit genug Platz für die Frequenz bleibt
  spr.drawFastHLine(box1X, box1Y + 24, box1W, 0xFFFF);

  // 1. Oberer Bereich: Band links, Modus/Stereo rechts
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0xFFFF);
  spr.drawString(getCurrentBand()->bandName, box1X + 8, box1Y + 4);

  spr.setTextDatum(TR_DATUM);
  if (currentMode == FM) {
    bool isStereo = (fmStereoIdx != FM_STEREO_MONO) && rx.getCurrentPilot();
    spr.setTextColor(isStereo ? 0x07E0 : 0xF800); 
    spr.drawString(isStereo ? "Stereo" : "Mono", box1X + box1W - 8, box1Y + 4);
  } else {
    spr.setTextColor(0x07E0);
    spr.drawString(bandModeDesc[currentMode], box1X + box1W - 8, box1Y + 4);
  }

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  // 2. Frequenzanzeige im unteren Teil von Kasten 1 (sicher zentriert, damit nichts abgeschnitten wird)
  drawFrequency(
    currentFrequency,
    box1X + 25, box1Y + 38,
    box1X + 135, box1Y + 38,
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


  // === KASTEN 3: Unten (Durchgehend fließender Wasserfall) ===
  int wfX = 10;
  int wfY = 121;
  int wfW = 300;
  int wfH = 44;
  
  spr.drawRect(wfX, wfY, wfW, wfH, 0xFFFF);

  int wfInnerW = wfW - 2;
  int wfInnerH = wfH - 2;

  if (!wfInitialized) {
    wfSpr.setPsram(true);
    wfSpr.setColorDepth(16);
    wfSpr.createSprite(wfInnerW, wfInnerH);
    wfSpr.fillScreen(0x03EF); 
    wfInitialized = true;
  }

  wfSpr.scroll(0, 1);

  uint8_t currentRssi = rx.getCurrentRSSI();
  uint16_t lineCol = 0x03EF; 
  if (currentRssi > 3)  lineCol = 0x04FF; 
  if (currentRssi > 10) lineCol = 0x07E0; 
  if (currentRssi > 20) lineCol = 0xFFE0; 
  if (currentRssi > 30) lineCol = 0xF800; 

  int barW = map(currentRssi, 0, 45, 10, wfInnerW - 20);
  barW = constrain(barW, 10, wfInnerW - 20);
  int barX = (wfInnerW - barW) / 2;

  wfSpr.fillRect(0, 0, wfInnerW, 1, 0x03EF); 
  wfSpr.fillRect(barX, 0, barW, 1, lineCol);

  wfSpr.pushSprite(&spr, wfX + 1, wfY + 1);

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
}
