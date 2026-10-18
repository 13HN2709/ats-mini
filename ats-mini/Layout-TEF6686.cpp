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

  // 1. Batterie
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Bandname mittig über dem Modusfeld
  const int modeX = 10;
  const int modeY = 19;
  const int modeW = 48;
  const int modeH = 17;

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(
    getCurrentBand()->bandName,
    modeX + modeW / 2,
    modeY - 2
  );

  // 3. Modusfeld
  spr.drawRoundRect(modeX, modeY, modeW, modeH, 3, COLOR_BLUE);

  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(
    bandModeDesc[currentMode],
    modeX + modeW / 2,
    modeY + modeH / 2
  );

  // 4. Statusfeld rechts: Stereo/Mono und Bandbreite
  const int statusX = 242;
  const int statusY = 19;
  const int statusW = 68;
  const int statusH = 36;
  const int statusMidY = statusY + statusH / 2;

  spr.drawRect(statusX, statusY, statusW, statusH, COLOR_BLUE);
  spr.drawLine(
    statusX, statusMidY,
    statusX + statusW - 1, statusMidY,
    COLOR_BLUE
  );

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_CYAN);

  // Stereo-Erkennung wie im Layout-Default
  if (currentMode == FM)
  {
    bool stereo =
      (fmStereoIdx != FM_STEREO_MONO) &&
      rx.getCurrentPilot();

    spr.drawString(
      stereo ? "STEREO" : "MONO",
      statusX + statusW / 2,
      statusY + statusH / 4
    );
  }
  else
  {
    spr.drawString(
      bandModeDesc[currentMode],
      statusX + statusW / 2,
      statusY + statusH / 4
    );
  }

  // Tatsächliche Bandbreite
  const Bandwidth *bw = getCurrentBandwidth();

  spr.setTextColor(COLOR_GREEN);

  if (bw != nullptr && bw->desc != nullptr)
  {
    spr.drawString(
      bw->desc,
      statusX + statusW / 2,
      statusY + (statusH * 3) / 4
    );
  }
  else
  {
    spr.drawString(
      "BW",
      statusX + statusW / 2,
      statusY + (statusH * 3) / 4
    );
  }

  // 5. Frequenz linksbündig an fester Position
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
  const int freqY = 48;

  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(COLOR_GREEN);

  // Frequenz vom Statusfeld rechts fernhalten
  spr.setClipRect(62, 36, 176, 31);
  spr.drawString(freqBuf, freqX, freqY);
  spr.clearClipRect();

  const int freqWidth = spr.textWidth(freqBuf);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(COLOR_CYAN);

  // Einheit ebenfalls vor dem Statusfeld begrenzen
  spr.setClipRect(62, 36, 176, 31);
  spr.drawString(unitBuf, freqX + freqWidth + 4, freqY);
  spr.clearClipRect();

  // 6. S-Meter
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

  spr.drawRect(meterX, meterY, meterW, meterH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString("S", 15, 73);

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
      // +30 vollständig innerhalb des Rahmens
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

  // 7. Sendername und Radiotext
  if (currentMode == FM)
  {
    const int stationX = 10;
    const int stationY = 109;
    const int stationW = 300;
    const int stationH = 24;

    spr.drawRect(
      stationX, stationY, stationW, stationH, COLOR_BLUE
    );

    // PS-Feld innen leeren, damit kein alter Name stehen bleibt
    spr.fillRect(
      stationX + 1, stationY + 1,
      stationW - 2, stationH - 2,
      COLOR_BLACK
    );

    spr.drawRect(
      stationX, stationY, stationW, stationH, COLOR_BLUE
    );

    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_CYAN);
    spr.drawString("PS:", stationX + 5, stationY + 3);

    // Direkt wie im Layout-Default: kein eigener Namensspeicher
    const char *stationName = getStationName();

    if (stationName != nullptr && *stationName)
    {
      if ((uint8_t)*stationName == 0xFF)
      {
        drawLongStationName(
          stationName + 1,
          stationX + 40,
          stationY + 3
        );
      }
      else
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

        spr.drawString(stationName, stationX + 40, stationY + 3);
        spr.clearClipRect();
      }
    }

    // Radiotext
    const int rtX = 10;
    const int rtY = 136;
    const int rtW = 300;
    const int rtH = 30;

    spr.fillRect(rtX + 1, rtY + 1, rtW - 2, rtH - 2, COLOR_BLACK);
    spr.drawRect(rtX, rtY, rtW, rtH, COLOR_BLUE);

    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(COLOR_CYAN);
    spr.drawString("RT:", rtX + 5, rtY + 5);

    const char *rt = getRadioText();

    if (rt != nullptr && *rt)
    {
      spr.setFont(&lgfx::fonts::FreeSans9pt7b);
      spr.setTextColor(COLOR_GREEN);
      spr.setTextDatum(TL_DATUM);

      spr.setClipRect(rtX + 38, rtY + 2, rtW - 43, rtH - 4);
      spr.drawString(rt, rtX + 40, rtY + 5);
      spr.clearClipRect();
    }
  }

  // 8. Menüanzeige
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
