#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// --------------------------------------------------
// ATS Mini V4 - TEF6686-Style Layout (Optimiert für 240x135)
// --------------------------------------------------

void drawLayoutTEF6686()
{
  const uint16_t COLOR_BLUE   = 0x001F;
  const uint16_t COLOR_CYAN   = 0x07FF;
  const uint16_t COLOR_GREEN  = 0xBFE0;
  const uint16_t COLOR_GREY   = 0x4208;
  const uint16_t COLOR_YELLOW = 0xFFE0;
  const uint16_t COLOR_RED    = 0xF800;
  const uint16_t COLOR_BLACK  = 0x0000;

  // 1. Batterie
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Bandname und Modusfeld (links oben)
  const int modeX = 10;
  const int modeY = 18;
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

  spr.fillRect(modeX + 1, modeY + 1, modeW - 2, modeH - 2, COLOR_BLACK);
  spr.drawRoundRect(modeX, modeY, modeW, modeH, 3, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_GREEN);

  spr.drawString(
    bandModeDesc[currentMode],
    modeX + modeW / 2,
    modeY + modeH / 2
  );

  // 3. Rechter Statuskasten (angepasst an 240px Breite)
  const int statusX = 175;
  const int statusY = 18;
  const int statusW = 55;
  const int statusH = 35;
  const int statusMidY = statusY + statusH / 2;

  spr.fillRect(statusX, statusY, statusW, statusH, COLOR_BLACK);
  spr.drawRect(statusX, statusMidY, statusW, statusH - (statusMidY - statusY), COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_CYAN);

  if (currentMode == FM)
  {
    bool stereo = (fmStereoIdx != FM_STEREO_MONO) && rx.getCurrentPilot();
    spr.drawString(stereo ? "STEREO" : "MONO", statusX + statusW / 2, statusY + 10);
  }
  else
  {
    spr.drawString(bandModeDesc[currentMode], statusX + statusW / 2, statusY + 10);
  }

  // 4. Frequenz und Abstimmcursor (Skaliert für 240x135)
  const int freqX = 165;
  const int freqY = 38;

  struct Line { int x; int y; int w; };

  const Line hlDigitsFM[] = {
    { freqX - 25 - 26 * 0, freqY + 22, 22 },
    { freqX - 25 - 26 * 1, freqY + 22, 22 },
    { freqX - 25 - 26 * 2, freqY + 22, 22 },
    { freqX - 25 - 26 * 3, freqY + 22, 22 },
    { freqX - 25 - 26 * 4, freqY + 22, 12 },
  };

  const Line hlDigitsAMSSB[] = {
    { freqX - 25 - 26 * 0, freqY + 22, 22 },
    { freqX - 25 - 26 * 1, freqY + 22, 22 },
    { freqX - 25 - 26 * 2, freqY + 22, 22 },
    { freqX - 25 - 26 * 3, freqY + 22, 22 },
    { freqX - 25 - 26 * 4, freqY + 22, 22 },
  };

  uint8_t hl = currentCmd == CMD_FREQ ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0) : 100;
  bool selectOn = (hl & 0x80) != 0;
  hl &= 0x7F;

  const Line *li = nullptr;

  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(COLOR_GREEN);

  if (currentMode == FM)
  {
    if (hl < (sizeof(hlDigitsFM) / sizeof(hlDigitsFM[0])))
      li = &hlDigitsFM[hl];

    spr.drawFloat(currentFrequency / 100.00, 2, freqX, freqY, FONT_DIGITS);
  }
  else
  {
    if (hl < (sizeof(hlDigitsAMSSB) / sizeof(hlDigitsAMSSB[0])))
      li = &hlDigitsAMSSB[hl];

    if (isSSB())
    {
      long long freq = (long long)currentFrequency * 1000LL + currentBFO;
      bool negative = (freq < 0);
      if (negative) freq = -freq;

      char text[32];
      snprintf(text, sizeof(text), "%03lld", freq / 1000);
      spr.drawString(text, freqX, freqY, FONT_DIGITS);

      spr.setTextDatum(ML_DATUM);
      spr.setTextColor(COLOR_GREEN);
      snprintf(text, sizeof(text), ".%03lld", freq % 1000);
      spr.drawString(text, freqX + 2, freqY + 14, &lgfx::fonts::FreeSans9pt7b);

      if (negative)
      {
        spr.setTextDatum(MR_DATUM);
        spr.drawString("-", freqX - 80, freqY, FONT_DIGITS);
      }
    }
    else
    {
      spr.drawNumber(currentFrequency, freqX, freqY, FONT_DIGITS);

      spr.setTextDatum(ML_DATUM);
      spr.setTextColor(COLOR_GREEN);
      // Sauber positioniert, damit es nicht in die kHz-Anzeige ragt
      spr.drawString(".000", freqX + 2, freqY + 14, &lgfx::fonts::FreeSans9pt7b);
    }
  }

  // 5. Cursor unter Ziffer
  if (li != nullptr)
  {
    if (selectOn)
    {
      spr.fillRoundRect(li->x + 1, li->y - 1, li->w - 2, 3, 1, COLOR_YELLOW);
    }
    else
    {
      spr.fillRoundRect(li->x, li->y - 1, li->w, 3, 1, COLOR_GREEN);
    }
  }

  // 6. Einheit (MHz / kHz) im rechten Statuskasten
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(currentMode == FM ? "MHz" : "kHz", statusX + statusW / 2, statusMidY + 8);

  // 7. S-Meter (angepasst an 240px Display-Breite)
  const int meterX = 10;
  const int meterY = 62;
  const int meterW = 220;
  const int meterH = 30;

  const int barX = 35;
  const int barY = 70;
  const int barW = 175;
  const int barH = 6;

  int signal = constrain((int)rx.getCurrentRSSI(), 0, 60);
  int targetW = map(signal, 0, 60, 0, barW);

  static int displayedW = 0;
  static unsigned long lastMeterUpdate = 0;
  unsigned long now = millis();
  if (lastMeterUpdate == 0) lastMeterUpdate = now;
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
    if (displayedW < targetW) displayedW = targetW;
    lastMeterUpdate = now;
  }
  displayedW = constrain(displayedW, 0, barW);

  spr.drawRect(meterX, meterY, meterW, meterH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString("S", 13, meterY + 3);

  spr.fillRect(barX, barY, barW, barH, COLOR_GREY);

  const int greenEnd = barW * 65 / 100;
  const int yellowEnd = barW * 82 / 100;

  if (displayedW > 0)
  {
    int greenW = min(displayedW, greenEnd);
    if (greenW > 0) spr.fillRect(barX, barY, greenW, barH, COLOR_GREEN);

    if (displayedW > greenEnd)
    {
      int yellowW = min(displayedW, yellowEnd) - greenEnd;
      if (yellowW > 0) spr.fillRect(barX + greenEnd, barY, yellowW, barH, COLOR_YELLOW);
    }
    if (displayedW > yellowEnd)
    {
      int redW = displayedW - yellowEnd;
      spr.fillRect(barX + yellowEnd, barY, redW, barH, COLOR_RED);
    }
  }

  // Skalen-Ticks
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextColor(COLOR_CYAN);
  int scaleValues[] = { 0, 10, 20, 30, 40, 50, 60 };
  for (int i = 0; i < 7; i++)
  {
    int tickX = barX + map(scaleValues[i], 0, 60, 0, barW);
    spr.drawLine(tickX, barY - 1, tickX, barY + barH + 2, COLOR_BLUE);
  }

  // 8. PS-Feld (RDS Stationsname)
  const int psX = 10;
  const int psY = 96;
  const int psW = 220;
  const int psH = 17;

  spr.fillRect(psX + 1, psY + 1, psW - 2, psH - 2, COLOR_BLACK);
  spr.drawRect(psX, psY, psW, psH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_CYAN);
  spr.drawString("PS:", psX + 4, psY + 1);

  if (currentMode == FM)
  {
    const char *stationName = getStationName();
    if (stationName != nullptr && *stationName)
    {
      spr.setTextColor(COLOR_GREEN);
      spr.setClipRect(psX + 30, psY + 1, psW - 35, psH - 2);
      spr.drawString(stationName, psX + 32, psY + 1);
      spr.clearClipRect();
    }
  }

  // 9. RT-Feld (Radiotext)
  const int rtX = 10;
  const int rtY = 115;
  const int rtW = 220;
  const int rtH = 17;

  spr.fillRect(rtX + 1, rtY + 1, rtW - 2, rtH - 2, COLOR_BLACK);
  spr.drawRect(rtX, rtY, rtW, rtH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_CYAN);
  spr.drawString("RT:", rtX + 4, rtY + 1);

  if (currentMode == FM)
  {
    const char *rt = getRadioText();
    if (rt != nullptr && *rt)
    {
      spr.setTextColor(COLOR_GREEN);
      spr.setClipRect(rtX + 30, rtY + 1, rtW - 35, rtH - 2);
      spr.drawString(rt, rtX + 32, rtY + 1);
      spr.clearClipRect();
    }
  }

  // 10. Menüanzeige
  if (currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }
}
