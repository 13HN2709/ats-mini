
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

  // 1. Batterie & Spannung oben rechts
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);
  (void)has_voltage;

  // 2. Oben links: Bandname
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(getCurrentBand()->bandName, 34, 2);

  // 3. Modus-Kasten
  const int modeBoxX = 10;
  const int modeBoxY = 20;
  const int modeBoxW = 48;
  const int modeBoxH = 18;

  spr.fillRoundRect(
    modeBoxX, modeBoxY, modeBoxW, modeBoxH, 4, COLOR_GREEN
  );

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_BLACK);
  spr.drawString(
    bandModeDesc[currentMode],
    modeBoxX + modeBoxW / 2,
    modeBoxY + modeBoxH / 2
  );

  // 4. RDS: Sendername und Radiotext
  if (currentMode == FM)
  {
    // Sendername direkt wie im Layout-Default abrufen
    const char *stationName = getStationName();

    // Anzeigebereich immer zuerst leeren, damit kein alter Name bleibt
    const int stationAreaX = 65;
    const int stationAreaY = 17;
    const int stationAreaW = 245;
    const int stationAreaH = 23;

    spr.fillRect(
      stationAreaX, stationAreaY,
      stationAreaW, stationAreaH,
      COLOR_BLACK
    );

    if (stationName != nullptr && *stationName)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(TC_DATUM);
      spr.setTextColor(COLOR_GREEN);

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
        // Normalen Sendernamen zentriert anzeigen
        spr.setClipRect(
          stationAreaX,
          stationAreaY,
          stationAreaW,
          stationAreaH
        );

        spr.drawString(
          stationName,
          stationAreaX + stationAreaW / 2,
          stationAreaY + stationAreaH / 2
        );

        spr.clearClipRect();
      }
    }

    // RDS-Textfeld
    const int boxX = 20;
    const int boxY = 105;
    const int boxW = 280;
    const int boxH = 36;

    spr.fillRoundRect(
      boxX, boxY, boxW, boxH, 5, COLOR_BLACK
    );

    spr.drawRoundRect(
      boxX, boxY, boxW, boxH, 5, COLOR_GREEN
    );

    // Radiotext linksbündig anzeigen
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

      spr.drawString(rt, boxX + 8, boxY + 8);

      spr.clearClipRect();
    }
  }

  // 5. Frequenzanzeige: Orbitron, zentriert
  char freqBuf[24];
  const char *unitBuf;

  if (currentMode == FM)
  {
    snprintf(
      freqBuf, sizeof(freqBuf), "%lu.%02lu",
      (unsigned long)(currentFrequency / 100),
      (unsigned long)(currentFrequency % 100)
    );

    unitBuf = "MHz";
  }
  else if (isSSB())
  {
    long long freq =
      (long long)currentFrequency * 1000LL + currentBFO;

    if (freq < 0)
      freq = 0;

    snprintf(
      freqBuf, sizeof(freqBuf), "%lld.%03lld",
      freq / 1000, freq % 1000
    );

    unitBuf = "kHz";
  }
  else
  {
    snprintf(
      freqBuf, sizeof(freqBuf), "%lu",
      (unsigned long)currentFrequency
    );

    unitBuf = "kHz";
  }

  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(COLOR_WHITE);
  spr.drawString(freqBuf, 160, 68);

  // Breite der Frequenz ermitteln
  int freqWidth = spr.textWidth(freqBuf);

  // Einheit direkt hinter der Frequenz
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(COLOR_UNIT);

  int unitX = 160 + (freqWidth / 2) + 4;
  spr.drawString(unitBuf, unitX, 68);

  // 6. S-Meter: untere Displayzeile
  const int barW = 220;
  const int barH = 7;
  const int barX = (320 - barW) / 2;
  const int barY = 160;

  int currentBarW = map(
    constrain(rssi, 0, 60),
    0, 60,
    5, barW
  );

  spr.fillRoundRect(
    barX, barY, barW, barH, 3, 0x2104
  );

  if (currentBarW > 0)
  {
    spr.fillRoundRect(
      barX, barY, currentBarW, barH, 3, COLOR_GREEN
    );
  }

  // 7. Sidebar / Menü
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
