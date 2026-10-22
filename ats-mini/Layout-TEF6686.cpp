
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// --------------------------------------------------
// ATS Mini V4 - TEF6686 Layout
// Display: 320 x 170 Pixel
//
// FM:
// - Original station name (PS)
// - Original radio text (RT)
//
// SW/MW:
// - Bandplan hints in English
// - Recommended target frequency in RT field
//
// IMPORTANT:
// - No automatic frequency changes.
// - No automatic reception mode changes.
// - Target frequency is a recommendation only.
// --------------------------------------------------

struct TEFBandplanHint
{
  const char *label;
  int targetKHz;
};

static TEFBandplanHint getTEFBandplanHint()
{
  const int32_t f = currentFrequency;

  // ------------------------------------------------
  // 1. FT8 frequency ranges (IARU Region 1 guidance)
  // ------------------------------------------------

  if (f >= 1838 && f <= 1842)
    return {"FT8 - USB", 1840};

  if (f >= 3568 && f <= 3573)
    return {"FT8 - USB", 3573};

  if (f >= 5355 && f <= 5359)
    return {"FT8 - USB", 5357};

  if (f >= 7069 && f <= 7079)
    return {"FT8 - USB", 7074};

  if (f >= 10132 && f <= 10137)
    return {"FT8 - USB", 10136};

  if (f >= 14070 && f <= 14077)
    return {"FT8 - USB", 14074};

  if (f >= 18096 && f <= 18102)
    return {"FT8 - USB", 18100};

  if (f >= 21070 && f <= 21078)
    return {"FT8 - USB", 21074};

  if (f >= 24911 && f <= 24916)
    return {"FT8 - USB", 24915};

  if (f >= 28070 && f <= 28080)
    return {"FT8 - USB", 28074};

  // ------------------------------------------------
  // 3. SSTV
  // ------------------------------------------------

  if (f >= 7160 && f <= 7175)
    return {"SSTV - USB", 7165};

  if (f >= 14225 && f <= 14235)
    return {"SSTV - USB", 14230};

  if (f >= 27695 && f <= 27705)
    return {"SSTV - USB", 27700};

  if (f >= 28675 && f <= 28685)
    return {"SSTV - USB", 28680};

  // ------------------------------------------------
  // 4. RTTY / Weather information
  // ------------------------------------------------

  if (f >= 145 && f <= 150)
    return {"WEATHER - RTTY", 147};

  if (f >= 4580 && f <= 4586)
    return {"WEATHER RADIO", 4583};

  if (f >= 7643 && f <= 7649)
    return {"WEATHER RADIO", 7646};

  if (f >= 10098 && f <= 10104)
    return {"DIGITAL - RTTY", 10101};

  if (f >= 11036 && f <= 11042)
    return {"WEATHER RADIO", 11039};

  if (f >= 14464 && f <= 14470)
    return {"WEATHER RADIO", 14467};

  // ------------------------------------------------
  // 5. CW - typical operating sections
  // ------------------------------------------------

  if (f >= 1830 && f <= 1837)
    return {"CW - MORSE", 1835};

  if (f >= 3500 && f <= 3560)
    return {"CW - MORSE", 3530};

  if (f >= 7000 && f <= 7040)
    return {"CW - MORSE", 7020};

  if (f >= 10100 && f <= 10130)
    return {"CW - MORSE", 10120};

  if (f >= 14000 && f <= 14069)
    return {"CW - MORSE", 14030};

  if (f >= 18068 && f <= 18095)
    return {"CW - MORSE", 18080};

  if (f >= 21000 && f <= 21069)
    return {"CW - MORSE", 21030};

  if (f >= 24890 && f <= 24910)
    return {"CW - MORSE", 24900};

  if (f >= 28000 && f <= 28069)
    return {"CW - MORSE", 28030};

  // ------------------------------------------------
  // 6. 12 m band - voice communication
  // ------------------------------------------------

  if (f >= 24940 && f <= 24990)
    return {"QSO - USB", 24950};

  // ------------------------------------------------
  // 7. 10 m band - voice communication
  // ------------------------------------------------

  if (f >= 28300 && f < 29000)
    return {"QSO - USB", 28360};

  if (f >= 29000 && f < 29100)
    return {"AM", 29000};

  if (f >= 29100 && f <= 29200)
    return {"FM", 29600};

  // ------------------------------------------------
  // 8. No specific recommendation
  // ------------------------------------------------

  return {"MIXED", 0};
}


// --------------------------------------------------
// Main layout
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

  // Calculate the bandplan hint once per frame so MODE and TARGET
  // always use the same frequency snapshot during this redraw.
  const TEFBandplanHint bandplanHint = getTEFBandplanHint();

  // 1. Background
  spr.fillSprite(COLOR_BLACK);

  // 2. Battery / voltage
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 3. Band name
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(getCurrentBand()->bandName, 16, 3);

  // 4. Reception mode
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

  // 4a. Mono / Stereo indicator
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

  // 5. Frequency unit
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

  // 6. Frequency - original display function
  const int freqX = 230;
  const int freqY = 45;

  uint8_t freqHighlight =
    currentCmd == CMD_FREQ
      ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0)
      : 100;

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

  // Scale marks
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

  // 8. PS field
  // FM: original station name.
  // Non-FM: bandplan hint.
  const int psX = 10;
  const int psY = 116;
  const int psW = 300;
  const int psH = 21;

  spr.drawRect(psX, psY, psW, psH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_CYAN);

  if (currentMode == FM)
  {
    spr.drawString("PS:", psX + 5, psY + 2);

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
  else
  {

    spr.drawString("MODE:", psX + 5, psY + 2);

    // Extra spacing between MODE: and FT4/FT8.
    const int hintX = psX + 76;
    const int hintClipX = psX + 73;
    const int hintClipW = psW - 78;

    spr.setTextColor(COLOR_GREEN);
    spr.setClipRect(
      hintClipX, psY + 1, hintClipW, psH - 2
    );
    spr.drawString(bandplanHint.label, hintX, psY + 2);
    spr.clearClipRect();
  }

  // 9. RT field
  // FM: original RDS radio text. Non-FM: left unused.
  const int rtX = 10;
  const int rtY = 141;
  const int rtW = 300;
  const int rtH = 22;

  spr.drawRect(rtX, rtY, rtW, rtH, COLOR_BLUE);

  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_CYAN);

  if (currentMode == FM)
  {
    spr.drawString("RT:", rtX + 5, rtY + 2);

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
  // Non-FM bandplan categories are shown in the MODE field above.

  // 10. Menu
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
