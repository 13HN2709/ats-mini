#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// --------------------------------------------------
// ATS Mini V4 - Clean Layout
// Display: 320 x 170 Pixel
// Frequenz: originale FONT_DIGITS / drawFrequency()
// Testversion: FT8 / FT4 Bandplan-Hinweis
// --------------------------------------------------

static const char *getBandplanHint()
{
  // Nur fuer USB und LSB Bandplan-Hinweise anzeigen.
  // currentFrequency wird bei AM/LSB/USB in kHz gespeichert.
  if (currentMode != USB && currentMode != LSB)
    return "GEMISCHT";

  const int32_t freq = currentFrequency;

  // FT8 - schmale Erkennungsfenster um die Dialfrequenzen.
  // Werte in kHz; keine automatische Modulationsumschaltung.
  if (currentMode == USB)
  {
    if (abs(freq - 1840) <= 1 ||
        abs(freq - 3573) <= 1 ||
        abs(freq - 5357) <= 1 ||
        abs(freq - 7074) <= 1 ||
        abs(freq - 10136) <= 1 ||
        abs(freq - 14074) <= 1 ||
        abs(freq - 18100) <= 1 ||
        abs(freq - 21074) <= 1 ||
        abs(freq - 24915) <= 1 ||
        abs(freq - 28074) <= 1)
    {
      return "FT8 - USB";
    }

    // FT4 - getrennt von FT8.
    if (abs(freq - 3575) <= 1 ||
        abs(freq - 7047) <= 1 ||
        abs(freq - 10140) <= 1 ||
        abs(freq - 14080) <= 1 ||
        abs(freq - 18104) <= 1 ||
        abs(freq - 21140) <= 1 ||
        abs(freq - 24919) <= 1 ||
        abs(freq - 28180) <= 1)
    {
      return "FT4 - USB";
    }
  }

  return "GEMISCHT";
}

void drawLayoutClean()
{
  const uint16_t COLOR_GREEN = 0x07E0;
  const uint16_t COLOR_WHITE = 0xFFFF;
  const uint16_t COLOR_BLACK = 0x0000;
  const uint16_t COLOR_GREY  = 0x2104;

  // 1. Batterie oben rechts
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Bandname oben links
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(getCurrentBand()->bandName, 15, 2);

  // 3. Modusfeld
  const int modeX = 10;
  const int modeY = 20;
  const int modeW = 48;
  const int modeH = 18;

  spr.fillRoundRect(
    modeX, modeY, modeW, modeH, 4, COLOR_GREEN
  );

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_BLACK);
  spr.drawString(
    bandModeDesc[currentMode],
    modeX + modeW / 2,
    modeY + modeH / 2
  );

  // 4. Sendername oberhalb der Frequenz
  if (currentMode == FM)
  {
    const char *stationName = getStationName();

    if (stationName != nullptr && *stationName)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(TC_DATUM);
      spr.setTextColor(COLOR_GREEN);

      spr.setClipRect(65, 15, 185, 20);

      if ((uint8_t)*stationName == 0xFF)
      {
        drawLongStationName(stationName + 1, 65, 17);
      }
      else
      {
        spr.drawString(stationName, 157, 17);
      }

      spr.clearClipRect();
    }
  }

  // 5. Frequenz mit Originalfunktion und Cursor
  // MHz/kHz um 15 Pixel nach rechts verschoben.
  drawFrequency(
    currentFrequency,
    FREQ_OFFSET_X, FREQ_OFFSET_Y,
    FUNIT_OFFSET_X + 15, FUNIT_OFFSET_Y,
    currentCmd == CMD_FREQ
      ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0)
      : 100
  );

  // 6. Bandplan-Hinweis im bisherigen RDS-Feld
  const int rtX = 20;
  const int rtY = 105;
  const int rtW = 280;
  const int rtH = 36;

  spr.fillRoundRect(
    rtX, rtY, rtW, rtH, 5, COLOR_BLACK
  );
  spr.drawRoundRect(
    rtX, rtY, rtW, rtH, 5, COLOR_GREEN
  );

  const char *hint = getBandplanHint();

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_WHITE);

  spr.setClipRect(
    rtX + 5, rtY + 3,
    rtW - 10, rtH - 6
  );

  spr.drawString(
    hint,
    rtX + rtW / 2,
    rtY + rtH / 2
  );

  spr.clearClipRect();

  // 7. S-Meter unten
  const int barX = 50;
  const int barY = 157;
  const int barW = 220;
  const int barH = 7;

  int signal = constrain((int)rssi, 0, 60);
  int currentBarW = map(signal, 0, 60, 5, barW);

  spr.fillRoundRect(
    barX, barY, barW, barH, 3, COLOR_GREY
  );

  if (currentBarW > 0)
  {
    spr.fillRoundRect(
      barX, barY, currentBarW, barH, 3, COLOR_GREEN
    );
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
