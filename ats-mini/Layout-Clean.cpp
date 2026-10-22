#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// --------------------------------------------------
// ATS Mini V4 - Clean Layout
// Display: 320 x 170 Pixel
// Bandplan-Hinweis: frequenzabhaengig, Modus-unabhaengig
//
// WICHTIG:
// - Keine automatische Umschaltung der Empfangsart.
// - UKW/FM: RDS-Radiotext bleibt erhalten.
// - KW/MW: Bandplan, Betriebsart-Empfehlung und Ziel-QRG.
// - Eintraege sind Empfangshinweise, keine Sendefreigabe.
// --------------------------------------------------

struct BandplanHint
{
  const char *label;
  int targetKHz;
};

static BandplanHint getBandplanHint()
{
  const int32_t freq = currentFrequency;

  // 1. FT8
  // Bereiche zeigen die empfohlene Dialfrequenz.
  if (freq >= 1839 && freq <= 1841)
    return {"FT8 - USB", 1840};

  if (freq >= 3572 && freq <= 3574)
    return {"FT8 - USB", 3573};

  if (freq >= 5356 && freq <= 5358)
    return {"FT8 - USB", 5357};

  if (freq >= 7073 && freq <= 7075)
    return {"FT8 - USB", 7074};

  if (freq >= 10135 && freq <= 10137)
    return {"FT8 - USB", 10136};

  if (freq >= 14070 && freq <= 14077)
    return {"FT8 - USB", 14074};

  if (freq >= 18099 && freq <= 18101)
    return {"FT8 - USB", 18100};

  if (freq >= 21073 && freq <= 21075)
    return {"FT8 - USB", 21074};

  if (freq >= 24914 && freq <= 24916)
    return {"FT8 - USB", 24915};

  if (freq >= 28070 && freq <= 28080)
    return {"FT8 - USB", 28074};

  // 2. FT4
  // FT4 wird vor den allgemeinen Bereichen geprueft.
  if (abs(freq - 3575) <= 1)
    return {"FT4 - USB", 3575};

  if (abs(freq - 7047) <= 1)
    return {"FT4 - USB", 7047};

  if (abs(freq - 10140) <= 1)
    return {"FT4 - USB", 10140};

  if (freq >= 14078 && freq <= 14082)
    return {"FT4 - USB", 14080};

  if (abs(freq - 18104) <= 1)
    return {"FT4 - USB", 18104};

  if (abs(freq - 21140) <= 1)
    return {"FT4 - USB", 21140};

  if (abs(freq - 24919) <= 1)
    return {"FT4 - USB", 24919};

  if (abs(freq - 28180) <= 1)
    return {"FT4 - USB", 28180};

  // 3. SSTV
  if (abs(freq - 7165) <= 3)
    return {"SSTV - USB", 7165};

  if (abs(freq - 7171) <= 3)
    return {"SSTV - USB", 7171};

  if (abs(freq - 14230) <= 3)
    return {"SSTV - USB", 14230};

  if (abs(freq - 27700) <= 3)
    return {"SSTV - USB", 27700};

  if (abs(freq - 28680) <= 3)
    return {"SSTV - USB", 28680};

  // 4. RTTY / Wetterfunk
  // Orientierung an typischen Wetterfunkfrequenzen.
  // Frequenzauflosung und Betriebszeiten bitte am aktuellen
  // Sendeplan pruefen; keine Rufzeichen werden angezeigt.
  if (abs(freq - 147) <= 1)
    return {"WETTER - RTTY", 147};

  if (abs(freq - 4583) <= 2)
    return {"RTTY - USB", 4583};

  if (abs(freq - 7646) <= 2)
    return {"RTTY - USB", 7646};

  if (abs(freq - 10101) <= 2)
    return {"RTTY - USB", 10101};

  if (abs(freq - 11039) <= 2)
    return {"RTTY - USB", 11039};

  if (abs(freq - 14467) <= 2)
    return {"RTTY - USB", 14467};

  // 5. Typische CW-Abschnitte
  if ((freq >= 1800  && freq < 1843)  ||
      (freq >= 3500  && freq < 3600)  ||
      (freq >= 7000  && freq < 7060)  ||
      (freq >= 10100 && freq < 10140) ||
      (freq >= 14000 && freq < 14100) ||
      (freq >= 18068 && freq < 18110) ||
      (freq >= 21000 && freq < 21150) ||
      (freq >= 24890 && freq < 24931) ||
      (freq >= 28000 && freq < 28300))
  {
    return {"CW - MORSE", 0};
  }

  // 6. CB-Funk
  if (freq >= 26565 && freq <= 27405)
    return {"CB - AM/FM/SSB", 0};

  // 7. Typische Sprach-QSO-Bereiche
  // DAY/NIGHT sind Hinweise, keine automatische Berechnung.
  if (freq >= 1843 && freq <= 2000)
    return {"QSO - LSB NIGHT", 0};

  if (freq >= 3600 && freq <= 3800)
    return {"QSO - LSB NIGHT", 0};

  if (freq >= 5354 && freq <= 5366)
    return {"QSO - USB DAY/NIGHT", 0};

  if (freq >= 7060 && freq <= 7200)
    return {"QSO - LSB DAY/NIGHT", 0};

  if (freq >= 14100 && freq <= 14350)
    return {"QSO - USB DAY", 0};

  if (freq >= 18110 && freq <= 18168)
    return {"QSO - USB DAY", 0};

  if (freq >= 21150 && freq <= 21450)
    return {"QSO - USB DAY", 0};

  if (freq >= 24931 && freq <= 24990)
    return {"QSO - USB DAY", 0};

  if (freq >= 28300 && freq < 29100)
    return {"QSO - USB DAY", 0};

  if (freq >= 29100 && freq <= 29700)
    return {"QSO - FM DAY", 0};

  // 8. Kurzwellenrundfunk
  if ((freq >= 3950  && freq <= 4000)  ||
      (freq >= 5900  && freq <= 6200)  ||
      (freq >= 7200  && freq <= 7450)  ||
      (freq >= 9400  && freq <= 9900)  ||
      (freq >= 11600 && freq <= 12100) ||
      (freq >= 13570 && freq <= 13870) ||
      (freq >= 15100 && freq <= 15800) ||
      (freq >= 17480 && freq <= 17900) ||
      (freq >= 18900 && freq <= 19020) ||
      (freq >= 21450 && freq <= 21850) ||
      (freq >= 25670 && freq <= 26100))
  {
    return {"SW-RADIO AM", 0};
  }

  // 9. Utility-Funk - vorlaeufige Orientierung
  if ((freq >= 4000  && freq < 4438)  ||
      (freq >= 6200  && freq < 6525)  ||
      (freq >= 8800  && freq < 9040)  ||
      (freq >= 11175 && freq < 11400) ||
      (freq >= 13200 && freq < 13360) ||
      (freq >= 15010 && freq < 15100) ||
      (freq >= 17900 && freq < 18030) ||
      (freq >= 21870 && freq < 21924))
  {
    return {"UTILITY - USB", 0};
  }

  return {"GEMISCHT", 0};
}

void drawLayoutClean()
{
  const uint16_t COLOR_GREEN = 0x07E0;
  const uint16_t COLOR_WHITE = 0xFFFF;
  const uint16_t COLOR_BLACK = 0x0000;
  const uint16_t COLOR_GREY  = 0x2104;

  // 1. Batterie
  drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Bandname
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(COLOR_GREEN);
  spr.drawString(getCurrentBand()->bandName, 15, 2);

  // 3. Aktuell eingestellter Modus.
  // Dieser Bereich wird vom Bandplan nicht veraendert.
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

  // 4. Sendername bei UKW
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
        drawLongStationName(stationName + 1, 65, 17);
      else
        spr.drawString(stationName, 157, 17);

      spr.clearClipRect();
    }
  }

  // 5. Frequenz mit Originalfunktion und Cursor
  drawFrequency(
    currentFrequency,
    FREQ_OFFSET_X, FREQ_OFFSET_Y,
    FUNIT_OFFSET_X + 15, FUNIT_OFFSET_Y,
    currentCmd == CMD_FREQ
      ? getFreqInputPos() + (pushAndRotate ? 0x80 : 0)
      : 100
  );

  // 6. Unteres Feld:
  // UKW = RDS-Radiotext
  // KW/MW = Bandplan-Hinweis und optionale Ziel-QRG
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

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextColor(COLOR_WHITE);
  spr.setClipRect(
    rtX + 5, rtY + 3,
    rtW - 10, rtH - 6
  );

  if (currentMode == FM)
  {
    // RDS bleibt bei UKW aktiv.
    const char *rt = getRadioText();

    if (rt != nullptr && *rt)
    {
      spr.setTextDatum(ML_DATUM);
      spr.drawString(rt, rtX + 8, rtY + rtH / 2);
    }
  }
  else
  {
    BandplanHint hint = getBandplanHint();
    char hintText[48];

    if (hint.targetKHz > 0)
    {
      snprintf(
        hintText, sizeof(hintText),
        "%s  ZIEL: %d kHz",
        hint.label, hint.targetKHz
      );
    }
    else
    {
      snprintf(
        hintText, sizeof(hintText),
        "%s", hint.label
      );
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(
      hintText,
      rtX + rtW / 2,
      rtY + rtH / 2
    );
  }

  spr.clearClipRect();

  // 7. S-Meter
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

  // 8. Menueanzeige
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
