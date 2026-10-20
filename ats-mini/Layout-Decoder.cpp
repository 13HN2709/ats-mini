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
  drawWiFiIndicator(
    has_voltage ? WIFI_OFFSET_X : BATT_OFFSET_X - 13,
    WIFI_OFFSET_Y
  );

  // 1. Band und Modus (mittig oben)
  spr.setFont(&lgfx::fonts::Orbitron_Light_24);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFFFF);

  char bandModeBuf[32];
  snprintf(
    bandModeBuf,
    sizeof(bandModeBuf),
    "%s %s",
    getCurrentBand()->bandName,
    bandModeDesc[currentMode]
  );

  spr.drawString(bandModeBuf, 145, 8);

  if(switchThemeEditor())
  {
    spr.setTextDatum(TR_DATUM);
    spr.setTextColor(TH.text_warn);
    spr.setFont(&lgfx::fonts::FreeSans9pt7b);
    spr.drawString(TH.name, 319, BATT_OFFSET_Y + 17);
  }

  // 2. Frequenz & Einheit
  drawFrequency(
    currentFrequency,
    FREQ_OFFSET_X - 15, FREQ_OFFSET_Y,
    FUNIT_OFFSET_X - 15, FUNIT_OFFSET_Y,
    currentCmd == CMD_FREQ
      ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0)
      : 100
  );

  // 3. Stereo / Mono ohne Kasten
  if(currentMode == FM)
  {
    bool isStereo =
      (fmStereoIdx != FM_STEREO_MONO) && rx.getCurrentPilot();

    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(isStereo ? 0x07E0 : 0xF800);
    spr.setFont(&lgfx::fonts::FreeSans9pt7b);

    spr.drawString(
      isStereo ? "STEREO" : "MONO",
      240,
      62
    );
  }

  // 4. Sendername (Position unverändert bei Y = 90)
  const char *stationName = getStationName();

  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0x07E0);

  if(*stationName == 0xFF)
  {
    drawLongStationName(stationName + 1, 145, 90);
  }
  else if(*stationName)
  {
    drawStationName(stationName, 145, 90);
  }

  // Menü / Sidebar bei Tastendruck
  if(currentCmd != CMD_NONE &&
     currentCmd != CMD_SCAN &&
     currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }

  // 5. Unten: Der Wasserfall
  int wfX = 20;
  int wfY = 132;
  int wfW = 280;
  int wfH = 26;

  spr.drawRect(wfX, wfY, wfW, wfH, 0xFFFF);

  // Weiße Trennlinie am inneren oberen Rand
  spr.drawFastHLine(
    wfX + 1,
    wfY + 1,
    wfW - 2,
    0xFFFF
  );

  int wfInnerW = wfW - 2;
  int wfInnerH = wfH - 3;

  if(!wfInitialized ||
     wfSpr.width() != wfInnerW ||
     wfSpr.height() != wfInnerH)
  {
    if(wfInitialized)
    {
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

  // Signalstärke auslesen und auf die Mitte begrenzen
  uint8_t currentRssi = rx.getCurrentRSSI();

  int center = wfInnerW / 2;
  int maxRssi = (currentMode == FM) ? 35 : 45;

  int maxHalfWidth = 55;
  int baseWidth = map(
    currentRssi,
    0,
    maxRssi,
    4,
    maxHalfWidth
  );

  int jitter = (rand() % 5) - 2;
  int halfWidth = constrain(
    baseWidth + jitter,
    4,
    maxHalfWidth
  );

  // 1. Grüner Kern in der Mitte
  int coreW = max(2, halfWidth);

  wfSpr.fillRect(
    center - coreW / 2,
    0,
    coreW,
    1,
    0x07E0
  );

  // 2. Gelber Zwischenbereich
  int yellowW = max(2, halfWidth / 3);

  wfSpr.fillRect(
    center - coreW / 2 - yellowW,
    0,
    yellowW,
    1,
    0xFFE0
  );

  wfSpr.fillRect(
    center + coreW / 2,
    0,
    yellowW,
    1,
    0xFFE0
  );

  // 3. Roter Rand außen
  int redW = 3;

  int leftRedX =
    center - coreW / 2 - yellowW - redW;

  int rightRedX =
    center + coreW / 2 + yellowW;

  if(leftRedX >= 0)
  {
    wfSpr.fillRect(
      leftRedX,
      0,
      redW,
      1,
      0xF800
    );
  }

  if(rightRedX + redW <= wfInnerW)
  {
    wfSpr.fillRect(
      rightRedX,
      0,
      redW,
      1,
      0xF800
    );
  }

  // Wasserfall unter der weißen Trennlinie anzeigen
  wfSpr.pushSprite(&spr, wfX + 1, wfY + 2);

  // 6. Statusanzeige
  // Kein zusätzlicher RDS-Text unter dem Wasserfall
  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(
      isSSB()
        ? (currentFrequency + currentBFO / 1000)
        : currentFrequency
    );
  }
  else
  {
    drawStatus(STATUS_OFFSET_X, STATUS_OFFSET_Y);
  }
}
