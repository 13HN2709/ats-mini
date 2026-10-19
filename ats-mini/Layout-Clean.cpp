
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

void drawLayoutClean()
{
  const uint16_t COLOR_BLACK = 0x0000;
  const uint16_t COLOR_GREEN = 0x07E0;
  const uint16_t COLOR_WHITE = 0xFFFF;
  const uint16_t COLOR_UNIT  = 0x9CD3;

  // ------------------------------------------------
  // 1. Batterie & Spannung oben rechts
  // ------------------------------------------------
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);
  (void)has_voltage;

  // ------------------------------------------------
  // 2. Bandname: mittig über dem Modus-Kasten
  // ------------------------------------------------
  const int modeBoxX = 10;
  const int modeBoxY = 20;
  const int modeBoxW = 48;
  const int modeBoxH = 18;

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(COLOR_GREEN);

  spr.drawString(
    getCurrentBand()->bandName,
    modeBoxX + modeBoxW / 2,
    modeBoxY - 2
  );

  // ------------------------------------------------
  // 3. Modus-Kasten
  // ------------------------------------------------
  spr.fillRoundRect(
    modeBoxX,
    modeBoxY,
    modeBoxW,
    modeBoxH,
    4,
    COLOR_GREEN
  );

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_BLACK);

  spr.drawString(
    bandModeDesc[currentMode],
    modeBoxX + modeBoxW / 2,
    modeBoxY + modeBoxH / 2
  );

  // ------------------------------------------------
  // 4. Sendername oben: höher und weiter links
  //    Anzeige ungefähr auf Höhe des FM-Feldes
  // ------------------------------------------------
  const int stationAreaX = 65;
  const int stationAreaY = 5;
  const int stationAreaW = 245;
  const int stationAreaH = 23;

  // Bereich immer leeren, damit kein alter Name stehen bleibt
  spr.fillRect(
    stationAreaX,
    stationAreaY,
    stationAreaW,
    stationAreaH,
    COLOR_BLACK
  );

  if (currentMode == FM)
  {
    const char *stationName = getStationName();

    if (stationName != nullptr && *stationName)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(TC_DATUM);
      spr.setTextColor(COLOR_GREEN);

      spr.setClipRect(
        stationAreaX,
        stationAreaY,
        stationAreaW,
        stationAreaH
      );

      if ((uint8_t)*stationName == 0xFF)
      {
        // Langformat wie im Default-Layout
        drawLongStationName(
          stationName + 1,
          stationAreaX,
          stationAreaY
        );
      }
      else
      {
        // Normalen Sendernamen mittig ausrichten
        spr.drawString(
          stationName,
          stationAreaX + stationAreaW / 2,
          stationAreaY
        );
      }

      spr.clearClipRect();
    }
  }

  // ------------------------------------------------
  // 5. RDS-Radiotextfeld
  // ------------------------------------------------
  const int boxX = 20;
  const int boxY = 105;
  const int boxW = 280;
  const int boxH = 36;

  spr.fillRoundRect(
    boxX,
    boxY,
    boxW,
    boxH,
    5,
    COLOR_BLACK
  );

  spr.drawRoundRect(
    boxX,
    boxY,
    boxW,
    boxH,
    5,
    COLOR_GREEN
  );

  if (currentMode == FM)
  {
    const char *rt = getRadioText();

    if (rt != nullptr && *rt)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextColor(COLOR_WHITE);
      spr.setTextDatum(TL_DATUM);

      spr.setClipRect(
        boxX + 5,
        boxY + 3,
        boxW - 10,
        boxH - 6
      );

      spr.drawString(
        rt,
        boxX + 8,
        boxY + 8
      );

      spr.clearClipRect();
    }
  }

  // ------------------------------------------------
  // 6. Frequenz berechnen
  // ------------------------------------------------
  char freqBuf[24];
  const char *unitBuf;

  if (currentMode == FM)
  {
    snprintf(
      freqBuf,
      sizeof(freqBuf),
      "%lu.%02lu",
      (unsigned long)(currentFrequency / 100),
      (unsigned long)(currentFrequency % 100)
    );

    unitBuf = "MHz";
  }
  else if (isSSB())
  {
    long long freq =
      (long long)currentFrequency * 1000LL + currentBFO;

    bool negative = (freq < 0);
    if (negative)
      freq = -freq;

    snprintf(
      freqBuf,
      sizeof(freqBuf),
      "%s%lld.%03lld",
      negative ? "-" : "",
      freq / 1000,
      freq % 1000
    );

    unitBuf = "kHz";
  }
  else
  {
    snprintf(
      freqBuf,
      sizeof(freqBuf),
      "%lu",
      (unsigned long)currentFrequency
    );

    unitBuf = "kHz";
  }

  // ------------------------------------------------
  // 7. Frequenzanzeige: groß und zentriert
  // ------------------------------------------------
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(COLOR_WHITE);

  spr.setClipRect(
    12,
    34,
    296,
    55
  );

  spr.drawString(
    freqBuf,
    160,
    68
  );

  spr.clearClipRect();

  // Einheit direkt hinter der Frequenz
  int freqWidth = spr.textWidth(freqBuf);

  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(COLOR_UNIT);

  int unitX = 160 + (freqWidth / 2) + 4;

  spr.drawString(
    unitBuf,
    unitX,
    68
  );

  // ------------------------------------------------
  // 8. S-Meter: untere Displayzeile
  // ------------------------------------------------
  const int barW = 220;
  const int barH = 7;
  const int barX = (320 - barW) / 2;
  const int barY = 160;

  int currentBarW = map(
    constrain((int)rx.getCurrentRSSI(), 0, 60),
    0,
    60,
    5,
    barW
  );

  spr.fillRoundRect(
    barX,
    barY,
    barW,
    barH,
    3,
    0x2104
  );

  if (currentBarW > 0)
  {
    spr.fillRoundRect(
      barX,
      barY,
      currentBarW,
      barH,
      3,
      COLOR_GREEN
    );
  }

  // ------------------------------------------------
  // 9. Sidebar / Menü
  // ------------------------------------------------
  if (currentCmd != CMD_NONE &&
      currentCmd != CMD_SCAN &&
      currentCmd != CMD_FREQ)
  {
    drawSideBar(
      currentCmd,
      MENU_OFFSET_X,
      MENU_OFFSET_Y,
      MENU_DELTA_X
    );
  }
}
