
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"
#include <string.h>

// --------------------------------------------------
// ATS Mini V4 - Clean Layout im klassischen TEF6686-Stil
// 320 x 170 Pixel
// Schwarz, blaue Rahmen, neongrüne Schrift
// --------------------------------------------------

void drawLayoutClean()
{
  // Farben im RGB565-Format
  const uint16_t COLOR_BG     = 0x0000;
  const uint16_t COLOR_BLUE   = 0x001F;
  const uint16_t COLOR_CYAN   = 0x07FF;
  const uint16_t COLOR_GREEN  = 0x07E0;
  const uint16_t COLOR_WHITE  = 0xFFFF;
  const uint16_t COLOR_GREY   = 0x4208;

  // -----------------------------------------------
  // 1. Batterieanzeige
  // -----------------------------------------------
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // -----------------------------------------------
  // 2. Bandname oben links
  // -----------------------------------------------
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(getCurrentBand()->bandName, 10, 2);

  // -----------------------------------------------
  // 3. Betriebsmodus im blauen Rahmen
  // -----------------------------------------------
  const int modeX = 10;
  const int modeY = 19;
  const int modeW = 48;
  const int modeH = 17;

  spr.drawRoundRect(
    modeX, modeY, modeW, modeH, 3, COLOR_BLUE
  );

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(
    bandModeDesc[currentMode],
    modeX + modeW / 2,
    modeY + modeH / 2
  );

  // -----------------------------------------------
  // 4. Frequenz und Einheit
  // -----------------------------------------------
  char freqBuf[24];
  const char* unitBuf;

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

  // Große Frequenzanzeige
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(freqBuf, 160, 48);

  // Einheit direkt rechts neben der Frequenz
  int freqWidth = spr.textWidth(freqBuf);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(COLOR_CYAN);

  int unitX = 160 + freqWidth / 2 + 4;
  spr.drawString(unitBuf, unitX, 48);

  // -----------------------------------------------
  // 5. S-Meter mit Skala und Zahlen
  // -----------------------------------------------
  const int meterX = 10;
  const int meterY = 70;
  const int meterW = 300;
  const int meterH = 36;

  // Äußerer Rahmen
  spr.drawRect(
    meterX, meterY, meterW, meterH, COLOR_BLUE
  );

  // S-Beschriftung
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString("S", 15, 73);

  // Balkenbereich
  const int barX = 38;
  const int barY = 79;
  const int barW = 262;
  const int barH = 7;

  spr.fillRect(
    barX, barY, barW, barH, COLOR_GREY
  );

  // Tatsächlichen RSSI-Wert vom Empfänger verwenden
  int signal = constrain((int)rx.getCurrentRSSI(), 0, 60);

  int currentBarW = map(signal, 0, 60, 0, barW);

  if (currentBarW > 0)
  {
    spr.fillRect(
      barX, barY, currentBarW, barH, COLOR_GREEN
    );
  }

  // Skalenmarkierungen und Beschriftungen
  const int scaleValues[] = {0, 5, 10, 15, 20, 30, 45, 60};
  const char* scaleLabels[] = {
    "1", "3", "5", "7", "9", "+10", "+20", "+30"
  };

  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextColor(COLOR_CYAN);
  spr.setTextDatum(TC_DATUM);

  for (int i = 0; i < 8; i++)
  {
    int tickX = barX + map(scaleValues[i], 0, 60, 0, barW);

    spr.drawLine(
      tickX, barY - 1,
      tickX, barY + barH + 2,
      COLOR_BLUE
    );

    spr.drawString(
      scaleLabels[i],
      tickX,
      89
    );
  }

  // -----------------------------------------------
  // 6. Sendername: stabilisierte RDS-Anzeige
  // -----------------------------------------------
  if (currentMode == FM)
  {
    const int stationX = 10;
    const int stationY = 109;
    const int stationW = 300;
    const int stationH = 24;

    spr.drawRect(
      stationX, stationY, stationW, stationH, COLOR_BLUE
    );

    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_CYAN);
    spr.drawString("PS:", stationX + 5, stationY + 3);

    // Namen über die vorhandene ATS-Funktion abrufen
    const char* rawStation = getStationName();

    static char candidateStation[64] = "";
    static char stableStation[64] = "";
    static unsigned long candidateSince = 0;

    if (rawStation != nullptr && *rawStation)
    {
      // Kennzeichen für lange Sendernamen beachten
      const char* stationText = rawStation;

      if ((uint8_t)*stationText == 0xFF)
        stationText++;

      if (strncmp(
            stationText,
            candidateStation,
            sizeof(candidateStation) - 1
          ) != 0)
      {
        strncpy(
          candidateStation,
          stationText,
          sizeof(candidateStation) - 1
        );

        candidateStation[sizeof(candidateStation) - 1] = '\0';
        candidateSince = millis();
      }

      // Erst übernehmen, wenn der Name kurz stabil geblieben ist
      if (candidateStation[0] &&
          (millis() - candidateSince >= 1200))
      {
        strncpy(
          stableStation,
          candidateStation,
          sizeof(stableStation) - 1
        );

        stableStation[sizeof(stableStation) - 1] = '\0';
      }
    }

    if (stableStation[0])
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(TL_DATUM);
      spr.setTextColor(COLOR_GREEN);

      spr.setClipRect(
        stationX + 38,
        stationY + 2,
        stationW - 43,
        stationH - 4
      );

      spr.drawString(
        stableStation,
        stationX + 40,
        stationY + 3
      );

      spr.clearClipRect();
    }

    // ---------------------------------------------
    // 7. Radiotext: linksbündig, ohne Laufschrift
    // ---------------------------------------------
    const int rtX = 10;
    const int rtY = 136;
    const int rtW = 300;
    const int rtH = 30;

    spr.drawRect(
      rtX, rtY, rtW, rtH, COLOR_BLUE
    );

    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_CYAN);
    spr.drawString("RT:", rtX + 5, rtY + 5);

    const char* rt = getRadioText();

    if (rt != nullptr && *rt)
    {
      spr.setFont(&lgfx::fonts::FreeSans9pt7b);
      spr.setTextColor(COLOR_GREEN);
      spr.setTextDatum(TL_DATUM);

      spr.setClipRect(
        rtX + 38,
        rtY + 2,
        rtW - 43,
        rtH - 4
      );

      // Keine X-Verschiebung: Text bleibt links stehen
      spr.drawString(
        rt,
        rtX + 40,
        rtY + 5
      );

      spr.clearClipRect();
    }
  }

  // -----------------------------------------------
  // 8. Menüanzeige bei geöffnetem Menü
  // -----------------------------------------------
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
