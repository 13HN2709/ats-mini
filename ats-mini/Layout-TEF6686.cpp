
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// --------------------------------------------------
// ATS Mini V4 - TEF6686-Style Layout
// Display: 320 x 170 Pixel
// --------------------------------------------------

void drawLayoutTEF6686()
{
  const uint16_t COLOR_BLUE   = 0x001F;
  const uint16_t COLOR_CYAN   = 0x07FF;
  const uint16_t COLOR_GREEN  = 0x07E0;
  const uint16_t COLOR_GREY   = 0x4208;
  const uint16_t COLOR_YELLOW = 0xFFE0;
  const uint16_t COLOR_RED    = 0xF800;
  const uint16_t COLOR_BLACK  = 0x0000;

  // ------------------------------------------------
  // 1. Batterie
  // ------------------------------------------------
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // ------------------------------------------------
  // 2. Bandname und Modusfeld
  // ------------------------------------------------
  const int modeX = 10;
  const int modeY = 20;
  const int modeW = 48;
  const int modeH = 17;

  // Bandname mittig über dem Modusfeld
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(
    getCurrentBand()->bandName,
    modeX + modeW / 2,
    modeY - 2
  );

  // Modusfeld
  spr.drawRoundRect(modeX, modeY, modeW, modeH, 3, COLOR_BLUE);
  spr.fillRect(
    modeX + 2, modeY + 2,
    modeW - 4, modeH - 4,
    COLOR_BLACK
  );
  spr.drawRoundRect(modeX, modeY, modeW, modeH, 3, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(
    bandModeDesc[currentMode],
    modeX + modeW / 2,
    modeY + modeH / 2
  );

  // ------------------------------------------------
  // 3. Rechter Statusblock
  // Zwei verbundene Felder über beide Zeilen
  // ------------------------------------------------
  const int statusX = 242;
  const int statusY = 18;
  const int statusW = 68;
  const int statusH = 50;
  const int statusMidY = statusY + statusH / 2;

  // Innenfläche löschen
  spr.fillRect(
    statusX + 1, statusY + 1,
    statusW - 2, statusH - 2,
    COLOR_BLACK
  );

  // Gemeinsamer Außenrahmen und Trennlinie
  spr.drawRect(statusX, statusY, statusW, statusH, COLOR_BLUE);
  spr.drawLine(
    statusX, statusMidY,
    statusX + statusW - 1, statusMidY,
    COLOR_BLUE
  );

  // Oberes Statusfeld: Stereo/Mono oder Betriebsart
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_CYAN);

  if (currentMode == FM)
  {
    bool stereo =
      (fmStereoIdx != FM_STEREO_MONO) &&
      rx.getCurrentPilot();

    spr.drawString(
      stereo ? "STEREO" : "MONO",
      statusX + statusW / 2,
      statusY + 12
    );
  }
  else
  {
    spr.drawString(
      bandModeDesc[currentMode],
      statusX + statusW / 2,
      statusY + 12
    );
  }

  // ------------------------------------------------
  // 4. Frequenz und Einheit
  // ------------------------------------------------
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

  const int freqX = 64;
  const int freqY = 44;

  // Frequenz linksbündig zeichnen
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(COLOR_GREEN);

  const int freqAvailableW = statusX - freqX - 5;

  spr.setClipRect(
    freqX, 38,
    freqAvailableW, 31
  );

  spr.drawString(freqBuf, freqX, freqY);
  spr.clearClipRect();

  // Tatsächliche Textbreite ermitteln
  const int freqWidth = spr.textWidth(freqBuf);
  const int unitX = freqX + freqWidth + 4;

  // Einheit nur neben der Frequenz anzeigen,
  // wenn sie vollständig vor dem Statusblock Platz hat.
  const int unitWidth = [&]() {
    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    return spr.textWidth(unitBuf);
  }();

  const bool unitFits =
    unitX + unitWidth <= statusX - 4;

  if (unitFits)
  {
    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(COLOR_CYAN);
    spr.drawString(unitBuf, unitX, freqY);
  }

  // Unteres Statusfeld:
  // Bandbreite, sofern die Einheit neben der Frequenz passt.
  // Andernfalls Einheit im Statusfeld anzeigen.
  const Bandwidth *bw = getCurrentBandwidth();

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_GREEN);

  if (!unitFits)
  {
    spr.drawString(
      unitBuf,
      statusX + statusW / 2,
      statusMidY + 13
    );
  }
  else if (bw != nullptr && bw->desc != nullptr)
  {
    spr.drawString(
      bw->desc,
      statusX + statusW / 2,
      statusMidY + 13
    );
  }
  else
  {
    spr.drawString(
      "--",
      statusX + statusW / 2,
      statusMidY + 13
    );
  }

  // ------------------------------------------------
  // 5. S-Meter
  // ------------------------------------------------
  const int meterX = 10;
  const int meterY = 70;
  const int meterW = 300;
  const int meterH = 36;

  const int barX = 38;
  const int barY = 79;
  const int barW = 250;
  const int barH = 7;

  const int scaleValues[] = {0, 5, 10, 15, 20, 30, 45, 60};
  const char *scaleLabels[] = {
    "1", "3", "5", "7", "9", "+10", "+20", "+30"
  };

  int signal = constrain((int)rx.getCurrentRSSI(), 0, 60);
  int targetW = map(signal, 0, 60, 0, barW);

  static int displayedW = 0;
  static unsigned long lastMeterUpdate = 0;

  unsigned long now = millis();

  if (lastMeterUpdate == 0)
    lastMeterUpdate = now;

  unsigned long elapsed = now - lastMeterUpdate;

  if (targetW >= displayedW)
  {
    displayedW = targetW;
    lastMeterUpdate = now;
  }
  else if (elapsed >= 150)
  {
    int steps = elapsed / 150;
    displayedW -= steps * 5;

    if (displayedW < targetW)
      displayedW = targetW;

    lastMeterUpdate = now;
  }

  displayedW = constrain(displayedW, 0, barW);

  // Rahmen
  spr.drawRect(meterX, meterY, meterW, meterH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString("S", 15, 73);

  // Balken
  spr.fillRect(barX, barY, barW, barH, COLOR_GREY);

  const int greenEnd = barW * 65 / 100;
  const int yellowEnd = barW * 82 / 100;

  if (displayedW > 0)
  {
    int greenW = min(displayedW, greenEnd);

    if (greenW > 0)
      spr.fillRect(barX, barY, greenW, barH, COLOR_GREEN);

    if (displayedW > greenEnd)
    {
      int yellowW = min(displayedW, yellowEnd) - greenEnd;

      if (yellowW > 0)
        spr.fillRect(
          barX + greenEnd, barY, yellowW, barH, COLOR_YELLOW
        );
    }

    if (displayedW > yellowEnd)
    {
      int redW = displayedW - yellowEnd;

      spr.fillRect(
        barX + yellowEnd, barY, redW, barH, COLOR_RED
      );
    }
  }

  // Skalenstriche und Zahlen
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextColor(COLOR_CYAN);

  for (int i = 0; i < 8; i++)
  {
    int tickX = barX + map(scaleValues[i], 0, 60, 0, barW);

    spr.drawLine(
      tickX, barY - 1,
      tickX, barY + barH + 2,
      COLOR_BLUE
    );

    if (i == 7)
    {
      spr.setTextDatum(TR_DATUM);
      spr.drawString(
        scaleLabels[i],
        meterX + meterW - 4,
        89
      );
    }
    else
    {
      spr.setTextDatum(TC_DATUM);
      spr.drawString(scaleLabels[i], tickX, 89);
    }
  }

  // ------------------------------------------------
  // 6. Sendername oben mittig
  // ------------------------------------------------
  const int stationTopX = 62;
  const int stationTopY = 18;
  const int stationTopW = statusX - stationTopX - 4;
  const int stationTopH = 19;

  // Fläche für den Sendernamen bereinigen
  spr.fillRect(
    stationTopX, stationTopY,
    stationTopW, stationTopH,
    COLOR_BLACK
  );

  if (currentMode == FM)
  {
    const char *stationName = getStationName();

    if (stationName != nullptr && *stationName)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(MC_DATUM);
      spr.setTextColor(COLOR_GREEN);

      spr.setClipRect(
        stationTopX, stationTopY,
        stationTopW, stationTopH
      );

      if ((uint8_t)*stationName == 0xFF)
      {
        drawLongStationName(
          stationName + 1,
          stationTopX + 2,
          stationTopY
        );
      }
      else
      {
        spr.drawString(
          stationName,
          stationTopX + stationTopW / 2,
          stationTopY + stationTopH / 2
        );
      }

      spr.clearClipRect();
    }
  }

  // Statusrahmen nach dem Löschen der oberen Namensfläche
  // erneut zeichnen, damit er immer sauber bleibt.
  spr.drawRect(statusX, statusY, statusW, statusH, COLOR_BLUE);
  spr.drawLine(
    statusX, statusMidY,
    statusX + statusW - 1, statusMidY,
    COLOR_BLUE
  );

  // Statusbeschriftungen nachzeichnen
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_CYAN);

  if (currentMode == FM)
  {
    bool stereo =
      (fmStereoIdx != FM_STEREO_MONO) &&
      rx.getCurrentPilot();

    spr.drawString(
      stereo ? "STEREO" : "MONO",
      statusX + statusW / 2,
      statusY + 12
    );
  }
  else
  {
    spr.drawString(
      bandModeDesc[currentMode],
      statusX + statusW / 2,
      statusY + 12
    );
  }

  spr.setTextColor(COLOR_GREEN);

  if (!unitFits)
  {
    spr.drawString(
      unitBuf,
      statusX + statusW / 2,
      statusMidY + 13
    );
  }
  else if (bw != nullptr && bw->desc != nullptr)
  {
    spr.drawString(
      bw->desc,
      statusX + statusW / 2,
      statusMidY + 13
    );
  }
  else
  {
    spr.drawString(
      "--",
      statusX + statusW / 2,
      statusMidY + 13
    );
  }

  // ------------------------------------------------
  // 7. PS-Feld: bleibt in allen Modi sichtbar
  // ------------------------------------------------
  const int psX = 10;
  const int psY = 109;
  const int psW = 300;
  const int psH = 24;

  spr.fillRect(
    psX + 1, psY + 1,
    psW - 2, psH - 2,
    COLOR_BLACK
  );
  spr.drawRect(psX, psY, psW, psH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_CYAN);
  spr.drawString("PS:", psX + 5, psY + 3);

  if (currentMode == FM)
  {
    const char *stationName = getStationName();

    if (stationName != nullptr && *stationName)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(TL_DATUM);
      spr.setTextColor(COLOR_GREEN);

      spr.setClipRect(
        psX + 38, psY + 2,
        psW - 43, psH - 4
      );

      if ((uint8_t)*stationName == 0xFF)
      {
        drawLongStationName(
          stationName + 1,
          psX + 40,
          psY + 3
        );
      }
      else
      {
        spr.drawString(
          stationName,
          psX + 40,
          psY + 3
        );
      }

      spr.clearClipRect();
    }
  }

  // ------------------------------------------------
  // 8. RT-Feld: bleibt in allen Modi sichtbar
  // ------------------------------------------------
  const int rtX = 10;
  const int rtY = 136;
  const int rtW = 300;
  const int rtH = 30;

  spr.fillRect(
    rtX + 1, rtY + 1,
    rtW - 2, rtH - 2,
    COLOR_BLACK
  );
  spr.drawRect(rtX, rtY, rtW, rtH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_CYAN);
  spr.drawString("RT:", rtX + 5, rtY + 5);

  if (currentMode == FM)
  {
    const char *rt = getRadioText();

    if (rt != nullptr && *rt)
    {
      spr.setFont(&lgfx::fonts::FreeSans9pt7b);
      spr.setTextColor(COLOR_GREEN);
      spr.setTextDatum(TL_DATUM);

      spr.setClipRect(
        rtX + 38, rtY + 2,
        rtW - 43, rtH - 4
      );

      spr.drawString(rt, rtX + 40, rtY + 5);
      spr.clearClipRect();
    }
  }

  // ------------------------------------------------
  // 9. Menüanzeige
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
