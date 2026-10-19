
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// --------------------------------------------------
// ATS Mini V4 - Clean Layout
// Display: 320 x 170 Pixel
// --------------------------------------------------

void drawLayoutClean()
{
  const uint16_t COLOR_GREEN = 0x07E0;
  const uint16_t COLOR_WHITE = 0xFFFF;
  const uint16_t COLOR_BLACK = 0x0000;
  const uint16_t COLOR_UNIT  = 0x9CD3;
  const uint16_t COLOR_GREY  = 0x2104;
  const uint16_t COLOR_YELLOW = 0xFFE0;

  // 1. Batterie
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Bandname
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(getCurrentBand()->bandName, 10, 2);

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
    const char *stationName = getStationName();

    if (stationName != nullptr && *stationName)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(TC_DATUM);
      spr.setTextColor(COLOR_GREEN);

      if ((uint8_t)*stationName == 0xFF)
      {
        drawLongStationName(stationName + 1, 160, 20);
      }
      else
      {
        spr.setClipRect(65, 10, 245, 25);
        spr.drawString(stationName, 160, 20);
        spr.clearClipRect();
      }
    }

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

    const char *rt = getRadioText();

    if (rt != nullptr && *rt)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextColor(COLOR_WHITE);
      spr.setTextDatum(TL_DATUM);

      spr.setClipRect(
        boxX + 5, boxY + 3, boxW - 10, boxH - 6
      );
      spr.drawString(rt, boxX + 8, boxY + 8);
      spr.clearClipRect();
    }
  }

  // 5. Frequenz mit originaler FONT_DIGITS und Cursor
  const int freqX = 232;
  const int freqY = 40;

  struct Line
  {
    int x;
    int y;
    int w;
  };

  const Line hlDigitsFM[] =
  {
    { freqX - 30 - 32 * 0,      freqY + 28, 27 },
    { freqX - 30 - 32 * 0 - 16, freqY + 28, 43 },
    { freqX - 30 - 32 * 1,      freqY + 28, 27 },
    { freqX - 30 - 32 * 1 - 22, freqY + 28, 49 },
    { freqX - 30 - 32 * 2 - 12, freqY + 28, 27 },
    { freqX - 30 - 32 * 2 - 28, freqY + 28, 43 },
    { freqX - 30 - 32 * 3 - 12, freqY + 28, 27 },
    { freqX - 30 - 32 * 3 - 28, freqY + 28, 43 },
    { freqX - 30 - 32 * 4 +  4, freqY + 28, 11 },
  };

  const Line hlDigitsAMSSB[] =
  {
    { freqX + 12 + 14 * 2,      freqY + 28, 12 },
    { freqX + 12 + 14 * 2 - 7,  freqY + 28, 19 },
    { freqX + 12 + 14 * 1,      freqY + 28, 12 },
    { freqX + 12 + 14 * 1 - 7,  freqY + 28, 19 },
    { freqX + 12,               freqY + 28, 12 },
    { freqX + 12 - 11,          freqY + 28, 23 },
    { freqX - 30,                freqY + 28, 27 },
    { freqX - 30 - 16,           freqY + 28, 43 },
    { freqX - 30 - 32,           freqY + 28, 27 },
    { freqX - 30 - 32 - 16,      freqY + 28, 43 },
    { freqX - 30 - 64,           freqY + 28, 27 },
    { freqX - 30 - 64 - 16,      freqY + 28, 43 },
    { freqX - 30 - 96,           freqY + 28, 27 },
    { freqX - 30 - 96 - 16,      freqY + 28, 43 },
    { freqX - 30 - 128,          freqY + 28, 27 },
  };

  uint8_t hl =
    currentCmd == CMD_FREQ
      ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0)
      : 100;

  bool selectOn = (hl & 0x80) != 0;
  hl &= 0x7F;

  const Line *li = nullptr;

  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(COLOR_WHITE);

  if (currentMode == FM)
  {
    if (hl < (sizeof(hlDigitsFM) / sizeof(hlDigitsFM[0])))
      li = &hlDigitsFM[hl];

    spr.drawFloat(
      currentFrequency / 100.00,
      2,
      freqX,
      freqY,
      FONT_DIGITS
    );
  }
  else
  {
    if (hl < (sizeof(hlDigitsAMSSB) / sizeof(hlDigitsAMSSB[0])))
      li = &hlDigitsAMSSB[hl];

    if (isSSB())
    {
      long long freq =
        (long long)currentFrequency * 1000LL + currentBFO;

      bool negative = freq < 0;
      if (negative)
        freq = -freq;

      char text[32];

      snprintf(text, sizeof(text), "%03lld", freq / 1000);
      spr.drawString(text, freqX, freqY, FONT_DIGITS);

      spr.setTextDatum(ML_DATUM);
      spr.setTextColor(COLOR_WHITE);

      snprintf(text, sizeof(text), ".%03lld", freq % 1000);
      spr.drawString(text, freqX + 4, freqY + 17, FONT_LARGE);

      if (negative)
      {
        spr.setTextDatum(MR_DATUM);
        spr.drawString("-", freqX - 100, freqY, FONT_DIGITS);
      }
    }
    else
    {
      spr.drawNumber(
        currentFrequency,
        freqX,
        freqY,
        FONT_DIGITS
      );

      spr.setTextDatum(ML_DATUM);
      spr.setTextColor(COLOR_WHITE);
      spr.drawString(".000", freqX + 4, freqY + 17, FONT_LARGE);
    }
  }

  // Cursor unter der aktiven Frequenzziffer
  if (li != nullptr)
  {
    if (selectOn)
    {
      spr.fillRoundRect(
        li->x + 1, li->y - 1, li->w - 2, 3, 1, COLOR_YELLOW
      );

      spr.fillTriangle(
        li->x, li->y,
        li->x + 2, li->y - 2,
        li->x + 2, li->y - 2,
        COLOR_YELLOW
      );

      spr.fillTriangle(
        li->x + li->w - 1, li->y,
        li->x + li->w - 3, li->y - 2,
        li->x + li->w - 3, li->y + 2,
        COLOR_YELLOW
      );
    }
    else
    {
      spr.fillRoundRect(
        li->x, li->y - 1, li->w, 3, 1, COLOR_GREEN
      );
    }
  }

  // Einheit neben der Frequenz
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(COLOR_UNIT);

  spr.drawString(
    currentMode == FM ? "MHz" : "kHz",
    freqX + 5,
    freqY + 17
  );

  // 6. S-Meter
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
    barX, barY, barW, barH, 3, COLOR_GREY
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
