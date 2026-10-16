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

  // === KASTEN 1: Oben Links (Unterteilt in oberen Bereich & eigenes Frequenz-Kästchen unten) ===
  int box1X = 10;
  int box1Y = 35;
  int box1W = 195;
  int box1H = 82;
  spr.drawRect(box1X, box1Y, box1W, box1H, 0xFFFF); 

  // Trennlinie analog zu Kasten 2 (teilt Kasten 1 in Band/Modus oben und Frequenz unten)
  spr.drawFastHLine(box1X, box1Y + 30, box1W, 0xFFFF);

  // 1. Oberer Bereich: Band links, Modus/Stereo rechts
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0xFFFF);
  spr.drawString(getCurrentBand()->bandName, box1X + 8, box1Y + 6);

  spr.setTextDatum(TR_DATUM);
  if (currentMode == FM) {
    bool isStereo = (fmStereoIdx != FM_STEREO_MONO) && rx.getCurrentPilot();
    spr.setTextColor(isStereo ? 0x07E0 : 0xF800); 
    spr.drawString(isStereo ? "Stereo" : "Mono", box1X + box1W - 8, box1Y + 6);
  } else {
    spr.setTextColor(0x07E0);
    spr.drawString(bandModeDesc[currentMode], box1X + box1W - 8, box1Y + 6);
  }

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17, FONT_SMALL);
  }

  // 2. Unterer Bereich (Eigenes Frequenz-Kästchen): Perfekt zentriert eingepasst
  drawFrequency(
    currentFrequency,
    box1X + 12, box1Y + 38,
    box1X + 182, box1Y + 38,
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


  // === KASTEN 3: Unten (Wasserfall mit 5px rotem Rand & High-Speed fillRect) ===
  int wfX = 10;
  int wfY = 121;
  int wfW = 300;
  int wfH = 44;
  
  spr.drawRect(wfX, wfY, wfW, wfH, 0xFFFF);

  int wfInnerW = wfW - 2;
  int wfInnerH = wfH - 2;

  if (!wfInitialized || wfSpr.width() != wfInnerW || wfSpr.height() != wfInnerH) {
    if (wfInitialized) {
      wfSpr.deleteSprite();
    }
    wfSpr.setPsram(true);
    wfSpr.setColorDepth(16);
    wfSpr.createSprite(wfInnerW, wfInnerH);
    wfSpr.fillScreen(0x001F); // Dunkles SDR-Blau als Basis
    wfInitialized = true;
  }

  // Wasserfall fließend nach unten scrollen
  wfSpr.scroll(0, 1);

  // Hintergrund der neuen Zeile mit dunklem Blau füllen
  wfSpr.fillRect(0, 0, wfInnerW, 1, 0x001F);

  // Signalstärke auslesen und Farbprofil mit schnellen Blöcken zeichnen
  uint8_t currentRssi = rx.getCurrentRSSI();
  int center = wfInnerW / 2;
  int halfWidth = map(currentRssi, 0, 45, 6, 35);
  halfWidth = constrain(halfWidth, 6, wfInnerW / 2 - 12);

  // 1. Grüner Kern in der Mitte
  int coreW = max(2, halfWidth);
  wfSpr.fillRect(center - coreW / 2, 0, coreW, 1, 0x07E0);

  // 2. Gelber Zwischenbereich daneben
  int yellowW = max(2, halfWidth / 2);
  wfSpr.fillRect(center - coreW / 2 - yellowW, 0, yellowW, 1, 0xFFE0);
  wfSpr.fillRect(center + coreW / 2, 0, yellowW, 1, 0xFFE0);

  // 3. Kräftiger, 5 Pixel dicker roter Rand außen
  int redW = 5;
  int leftRedX = center - coreW / 2 - yellowW - redW;
  int rightRedX = center + coreW / 2 + yellowW;

  if (leftRedX >= 0) {
    wfSpr.fillRect(leftRedX, 0, redW, 1, 0xF800);
  }
  if (rightRedX + redW <= wfInnerW) {
    wfSpr.fillRect(rightRedX, 0, redW, 1, 0xF800);
  }

  // Den Wasserfall in das Haupt-Sprite pushen
  wfSpr.pushSprite(&spr, wfX + 1, wfY + 1);

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
}
