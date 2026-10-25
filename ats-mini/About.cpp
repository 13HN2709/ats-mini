#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// ATS Mini About pages: compact vector artwork, optimized for 320x170 pixels.
static constexpr uint16_t ABOUT_BG      = 0x0008;
static constexpr uint16_t ABOUT_PANEL   = 0x000F;
static constexpr uint16_t ABOUT_BLUE    = 0x041F;
static constexpr uint16_t ABOUT_CYAN    = 0x07FF;
static constexpr uint16_t ABOUT_GREEN   = 0x07E0;
static constexpr uint16_t ABOUT_RED     = 0xF800;
static constexpr uint16_t ABOUT_WHITE   = 0xFFFF;
static constexpr uint16_t ABOUT_MUTED   = 0xBDF7;
static constexpr uint16_t ABOUT_EARTH   = 0x039F;

static void drawAboutGlobe(int cx, int cy)
{
  // Stylized Earth with radio mast: vector-only, no external bitmap required.
  spr.fillCircle(cx, cy, 27, ABOUT_BG);
  spr.drawCircle(cx, cy, 27, ABOUT_BLUE);
  spr.drawCircle(cx, cy, 25, ABOUT_CYAN);

  // Latitude lines clipped visually by keeping them within the globe.
  spr.drawLine(cx - 20, cy - 9, cx + 20, cy - 9, ABOUT_EARTH);
  spr.drawLine(cx - 25, cy,     cx + 25, cy,     ABOUT_EARTH);
  spr.drawLine(cx - 20, cy + 9, cx + 20, cy + 9, ABOUT_EARTH);

  // Longitude arcs suggested by paired vertical curves/lines.
  spr.drawLine(cx - 10, cy - 22, cx - 15, cy - 10, ABOUT_EARTH);
  spr.drawLine(cx - 15, cy - 10, cx - 16, cy + 8, ABOUT_EARTH);
  spr.drawLine(cx - 16, cy + 8,  cx - 8, cy + 22, ABOUT_EARTH);
  spr.drawLine(cx + 10, cy - 22, cx + 15, cy - 10, ABOUT_EARTH);
  spr.drawLine(cx + 15, cy - 10, cx + 16, cy + 8, ABOUT_EARTH);
  spr.drawLine(cx + 16, cy + 8,  cx + 8, cy + 22, ABOUT_EARTH);

  // Radio tower rises from the globe, with three transmission rings.
  const int mastBaseY = cy - 20;
  const int mastTopY = cy - 48;
  spr.drawLine(cx, mastTopY, cx - 10, mastBaseY, ABOUT_WHITE);
  spr.drawLine(cx, mastTopY, cx + 10, mastBaseY, ABOUT_WHITE);
  spr.drawLine(cx - 7, mastBaseY - 8, cx + 7, mastBaseY - 8, ABOUT_CYAN);
  spr.drawLine(cx - 5, mastBaseY - 14, cx + 5, mastBaseY - 14, ABOUT_CYAN);
  spr.drawLine(cx - 3, mastBaseY - 20, cx + 3, mastBaseY - 20, ABOUT_CYAN);
  spr.fillCircle(cx, mastTopY, 2, ABOUT_GREEN);

  // Signal marks kept small so the display remains uncluttered.
  spr.drawLine(cx - 9, mastTopY + 2, cx - 14, mastTopY + 8, ABOUT_BLUE);
  spr.drawLine(cx - 14, mastTopY + 8, cx - 16, mastTopY + 14, ABOUT_BLUE);
  spr.drawLine(cx + 9, mastTopY + 2, cx + 14, mastTopY + 8, ABOUT_BLUE);
  spr.drawLine(cx + 14, mastTopY + 8, cx + 16, mastTopY + 14, ABOUT_BLUE);
}

static void drawAboutFrame(uint8_t page)
{
  spr.fillSprite(ABOUT_BG);
  spr.fillRoundRect(3, 3, 314, 164, 6, ABOUT_PANEL);
  spr.drawRoundRect(3, 3, 314, 164, 6, ABOUT_BLUE);
  spr.drawRoundRect(5, 5, 310, 160, 5, ABOUT_CYAN);

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(ABOUT_WHITE);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString("ATS MINI RECEIVER", 12, 8);

  spr.setTextColor(ABOUT_CYAN);
  spr.setFont(FONT_SMALL);
  spr.drawString(page == 0 ? "ALL BAND RECEIVER" : "OPEN-SOURCE PROJECT", 12, 29);
  spr.drawFastHLine(12, 47, 296, ABOUT_BLUE);

  char pageText[8];
  snprintf(pageText, sizeof(pageText), "%u/2", (unsigned)(page + 1));
  spr.setTextDatum(TR_DATUM);
  spr.setTextColor(ABOUT_CYAN);
  spr.drawString(pageText, 306, 148, FONT_SMALL);
  spr.setTextDatum(TL_DATUM);

  drawAboutGlobe(277, 111);
}

static void drawAboutProject()
{
  drawAboutFrame(0);

  spr.setTextColor(ABOUT_MUTED);
  spr.setFont(FONT_SMALL);
  spr.drawString("Firmware:", 13, 56);
  spr.drawString("Build:", 13, 76);
  spr.drawString("Device:", 13, 96);

  spr.setTextColor(0xFFE0);
  char version[12];
  snprintf(version, sizeof(version), "v%1.1d.%2.2d", VER_APP / 100, VER_APP % 100);
  spr.drawString(version, 87, 56);
  spr.drawString(__DATE__, 87, 76);
  spr.drawString("ATS Mini Receiver", 87, 96);

  spr.drawRoundRect(12, 121, 235, 28, 4, ABOUT_BLUE);
  spr.fillRoundRect(14, 123, 231, 24, 3, ABOUT_BG);
  spr.setTextColor(ABOUT_GREEN);
  spr.drawString("github.com/13HN2709/ats-mini", 20, 129, FONT_SMALL);

  spr.setTextColor(ABOUT_MUTED);
  spr.drawString("Rotate: next page", 12, 151, FONT_SMALL);
}

static void drawAboutCredits()
{
  drawAboutFrame(1);

  spr.setTextColor(ABOUT_MUTED);
  spr.setFont(FONT_SMALL);
  spr.drawString("Original: ATS Mini / SI4732", 13, 55);
  spr.drawString("Developer: Max Arnold + team", 13, 75);
  spr.drawString("Library: PU2CLR SI4735", 13, 95);

  spr.setTextColor(0xFFE0);

  spr.drawFastHLine(12, 119, 235, ABOUT_BLUE);
  spr.setTextColor(ABOUT_GREEN);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString("Custom Firmware & UI", 13, 125);
  spr.drawString("13HN2709", 13, 145);

  spr.setTextColor(ABOUT_MUTED);
  spr.setFont(FONT_SMALL);
  spr.drawString("Rotate: previous page", 13, 151);
}

void drawAboutHelp(uint8_t arrow)
{
  (void)arrow;
  drawAboutProject();
  spr.pushSprite(0, 0);
}

void drawAbout()
{
  // doAbout() is advanced by the encoder handler in Menu.cpp.
  if(doAbout(0) == 0)
    drawAboutProject();
  else
    drawAboutCredits();

  spr.pushSprite(0, 0);
}
