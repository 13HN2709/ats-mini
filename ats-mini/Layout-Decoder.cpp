#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

extern uint32_t lastTuneTime;

// Statische Variablen hier oben deklarieren, damit sie im Scope bekannt sind
static LGFX_Sprite wfSpr(&tft);
static bool wfInitialized = false;

void drawLayoutDecoder()
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
  
  // === Unten (Wasserfall mit weißer Trennlinie oben & 5px rotem Rand) ===
  int wfX = 10;
  int wfY = 121;
  int wfW = 300;
  int wfH = 44;
  
  spr.drawRect(wfX, wfY, wfW, wfH, 0xFFFF);

  // Weiße Trennlinie direkt bündig am inneren oberen Rand des Wasserfalls
  spr.drawFastHLine(wfX + 1, wfY + 1, wfW - 2, 0xFFFF);

  int wfInnerW = wfW - 2;
  int wfInnerH = wfH - 3; 

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

  // Signalstärke auslesen und Farbprofil mit 5px rotem Rand zeichnen
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

  // Den Wasserfall unter der weißen Trennlinie ins Haupt-Sprite pushen
  wfSpr.pushSprite(&spr, wfX + 1, wfY + 2);

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
}
