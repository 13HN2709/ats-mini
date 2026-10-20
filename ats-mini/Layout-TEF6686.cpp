#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// --------------------------------------------------
// ATS Mini V4 - TEF6686 Layout
// Display: 320 x 170 Pixel
// Frequenz: originale FONT_DIGITS / drawFrequency()
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

  // 1. Hintergrund
  spr.fillSprite(COLOR_BLACK);

  // 2. Batterie / Spannung
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 3. Bandname oben links
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(getCurrentBand()->bandName, 34, 3);

  // 4. Modusfeld links
  const int modeX = 10;
  const int modeY = 24;
  const int modeW = 48;
  const int modeH = 20;

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

  
  // 4a. Mono-/Stereo-Anzeige unter dem Modusfeld
  const int audioX = 10;
  const int audioY = 48;
  const int audioW = 48;
  const int audioH = 20;

  spr.drawRoundRect(
    audioX, audioY, audioW, audioH, 3, COLOR_BLUE
  );

  bool stereoActive =
    (currentMode == FM) &&
    (fmStereoIdx != FM_STEREO_MONO) &&
    rx.getCurrentPilot();

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);

  if (stereoActive)
  {
    spr.setTextColor(COLOR_RED);
    spr.drawString(
      "ST",
      audioX + audioW / 2,
      audioY + audioH / 2
    );
  }
  else
  {
    spr.setTextColor(TFT_WHITE);
    spr.drawString(
      "MO",
      audioX + audioW / 2,
      audioY + audioH / 2
    );
  }

  // 5. Einheit rechts oben
  // Das doppelte USB/LSB entfällt hier.
  // Die Modulationsanzeige bleibt links erhalten.
  const int statusX = 242;
  const int statusY = 24;
  const int statusW = 68;
  const int statusH = 20;

  spr.drawRect(
    statusX, statusY, statusW, statusH, COLOR_BLUE
  );

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(
    currentMode == FM ? "MHz" : "kHz",
    statusX + statusW / 2,
    statusY + statusH / 2
  );

  // 6. Frequenz mit originaler Digits-Schrift und Cursor
  const int freqX = 230;
  const int freqY = 45;

  uint8_t freqHighlight =
    currentCmd == CMD_FREQ
      ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0)
      : 100;

  // Einheit wird separat rechts oben gezeichnet.
  // Frequenz- und Cursorlogik bleiben unverändert.
  drawFrequency(
    currentFrequency,
    freqX,
    freqY,
    400,
    400,
    freqHighlight
  );

  // 7. S-Meter
  const int meterX = 10;
  const int meterY = 76;
  const int meterW = 300;
  const int meterH = 32;

  const int barX = 43;
  const int barY = 87;
  const int barW = 250;
  const int barH = 7;

  int signal = constrain((int)rx.getCurrentRSSI(), 0, 45);
  int targetW = map(signal, 0, 45, 0, barW);

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

  spr.drawRect(
    meterX, meterY, meterW, meterH, COLOR_BLUE
  );

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString("S", 18, meterY + 5);

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

  // Skalenmarkierungen
  const int scaleValues[] = { 0, 10, 20, 30, 40, 50, 60 };

  for (int i = 0; i < 7; i++)
  {
    int tickX = barX + map(scaleValues[i], 0, 60, 0, barW);

    spr.drawLine(
      tickX, barY - 2,
      tickX, barY + barH + 2,
      COLOR_CYAN
    );
  }

  // 8. PS-Feld (Sendername)
  const int psX = 10;
  const int psY = 116;
  const int psW = 300;
  const int psH = 21;

  spr.drawRect(psX, psY, psW, psH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_CYAN);
  spr.drawString("PS:", psX + 5, psY + 2);

  if (currentMode == FM)
  {
    const char *stationName = getStationName();

    if (stationName != nullptr && *stationName)
    {
      if ((uint8_t)*stationName == 0xFF)
        stationName++;

      spr.setTextColor(COLOR_GREEN);
      spr.setClipRect(
        psX + 38, psY + 1, psW - 43, psH - 2
      );
      spr.drawString(stationName, psX + 40, psY + 2);
      spr.clearClipRect();
    }
  }

  // 9. RT-Feld (Radiotext)
  const int rtX = 10;
  const int rtY = 141;
  const int rtW = 300;
  const int rtH = 22;

  spr.drawRect(rtX, rtY, rtW, rtH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_CYAN);
  spr.drawString("RT:", rtX + 5, rtY + 2);

  if (currentMode == FM)
  {
    const char *rt = getRadioText();

    if (rt != nullptr && *rt)
    {
      spr.setTextColor(COLOR_GREEN);
      spr.setClipRect(
        rtX + 38, rtY + 1, rtW - 43, rtH - 2
      );
      spr.drawString(rt, rtX + 40, rtY + 2);
      spr.clearClipRect();
    }
  }

  // 10. Menüanzeige
  if (currentCmd != CMD_NONE &&
      currentCmd != CMD_SCAN &&
      currentCmd != CMD_FREQ &&
      currentCmd != CMD_FREQ_KEYPAD)
  {
    drawSideBar(
      currentCmd,
      MENU_OFFSET_X,
      MENU_OFFSET_Y,
      MENU_DELTA_X
    );
  }
}
