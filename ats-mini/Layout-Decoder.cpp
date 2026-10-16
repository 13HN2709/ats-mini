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

  // 2. Frequenz & Einheit
  drawFrequency(
    currentFrequency,
    FREQ_OFFSET_X - 15, FREQ_OFFSET_Y,
    FUNIT_OFFSET_X - 15, FUNIT_OFFSET_Y,
    currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100
  );

  // Stereo / Mono Status-Kasten (wird nur im FM-Modus direkt unter .000 / kHz angezeigt)
  if(currentMode == FM)
  {
    bool isStereo = (fmStereoIdx != FM_STEREO_MONO) && rx.getCurrentPilot();
    
    int boxX = 185;
    int boxY = 62;
    int boxW = 65;
    int boxH = 18;
    
    // Weißer Kasten mit schwarzem Hintergrund für maximale Lesbarkeit
    spr.drawRect(boxX, boxY, boxW, boxH, 0xFFFF);
    spr.fillRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, 0x0000);
    
    spr.setTextDatum(CC_DATUM);
    spr.setTextColor(isStereo ? 0x07E0 : 0xF800); // 0x07E0 = Neongrün, 0xF800 = Neonrot
    spr.drawString(isStereo ? "STEREO" : "MONO", boxX + boxW / 2, boxY + boxH / 2, 2);
  }

  // 3. Sendername / RDS weiter nach oben gesetzt (Y = 90)
  const char *stationName = getStationName();
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0x07E0);
  if(*stationName == 0xFF)
    drawLongStationName(stationName + 1, 145, 90);
  else if(*stationName)
    drawStationName(stationName, 145, 90);

  // Menü / Sidebar bei Tastendruck
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }

  // === 4. Unten: Der Wasserfall (perfekt zentrierte Box) ===
  int wfX = 20;
  int wfY = 132;
  int wfW = 280;
  int wfH = 26;
  
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
    wfSpr.fillScreen(0x001F);
    wfInitialized = true;
  }

  // Wasserfall fließend nach unten scrollen
  wfSpr.scroll(0, 1);

  // Hintergrund der neuen Zeile mit dunklem Blau füllen
  wfSpr.fillRect(0, 0, wfInnerW, 1, 0x001F);

  // Signalstärke auslesen und strikt auf die Mitte begrenzen
  uint8_t currentRssi = rx.getCurrentRSSI();
  int center = wfInnerW / 2;
  int maxRssi = (currentMode == FM) ? 35 : 45;
  
  int maxHalfWidth = 55; // Begrenzt den Maximal-Ausschlag nach links/rechts
  int baseWidth = map(currentRssi, 0, maxRssi, 4, maxHalfWidth);
  int jitter = (rand() % 5) - 2; // Lebendiger SDR-Zackeneffekt
  int halfWidth = constrain(baseWidth + jitter, 4, maxHalfWidth);

  // 1. Grüner Kern in der Mitte
  int coreW = max(2, halfWidth);
  wfSpr.fillRect(center - coreW / 2, 0, coreW, 1, 0x07E0);

  // 2. Gelber Zwischenbereich daneben
  int yellowW = max(2, halfWidth / 3);
  wfSpr.fillRect(center - coreW / 2 - yellowW, 0, yellowW, 1, 0xFFE0);
  wfSpr.fillRect(center + coreW / 2, 0, yellowW, 1, 0xFFE0);

  // 3. Kräftiger, roter Rand außen
  int redW = 3;
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
  else if(!drawStatus(STATUS_OFFSET_X, STATUS_OFFSET_Y))
  {
    const char *rt = getRadioText();
    const char *pi = getProgramInfo();

    if((rt && *rt) || (pi && *pi))
    {
      spr.setTextColor(0x07E0);
      spr.setTextSize(1.0);
      spr.setTextDatum(TC_DATUM);
      spr.drawString(rt ? rt : pi, 145, 160, 4);
    }
  }
}
