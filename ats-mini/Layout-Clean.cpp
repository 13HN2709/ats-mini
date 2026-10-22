#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

// --------------------------------------------------
// ATS Mini V4 - Clean Layout
// Display: 320 x 170 Pixel
// Frequenz: originale FONT_DIGITS / drawFrequency()
// Bandplan-Hinweis: frequenzabhaengig, Modus-unabhaengig
// --------------------------------------------------

// Frequenzen in currentFrequency werden hier in kHz ausgewertet.
// Diese Regeln sind eine erste Orientierung fuer den Empfang,
// keine Sendefreigabe. Spezialbereiche (FT8/FT4/CW) haben Vorrang
// vor der allgemeinen QSO-Anzeige.
static const char *getBandplanHint()
{
  const int32_t freq = currentFrequency;

  // 1. FT8 - genaue Frequenzpunkte, unabhaengig vom eingestellten Modus.
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

  // 2. FT4 - genaue Frequenzpunkte, unabhaengig vom eingestellten Modus.
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

  // 3. SSTV - typische Anruffrequenzen.
  if (abs(freq - 7165) <= 3 ||
      abs(freq - 7171) <= 3 ||
      abs(freq - 14230) <= 3 ||
      abs(freq - 27700) <= 3 ||
      abs(freq - 28680) <= 3)
  {
    return "SSTV";
  }

  // 4. Typische CW-Abschnitte.
  if ((freq >= 1800  && freq < 1843) ||
      (freq >= 3500  && freq < 3600) ||
      (freq >= 7000  && freq < 7060) ||
      (freq >= 10100 && freq < 10140) ||
      (freq >= 14000 && freq < 14100) ||
      (freq >= 18068 && freq < 18110) ||
      (freq >= 21000 && freq < 21150) ||
      (freq >= 24890 && freq < 24931) ||
      (freq >= 28000 && freq < 28300))
  {
    return "CW - MORSE";
  }

  // 5. CB-Funk: ungefaehr 26.565 bis 27.405 MHz.
  if (freq >= 26565 && freq <= 27405)
  {
    return "CB - AM/FM/SSB";
  }

  // 6. Typische Sprach-QSO-Bereiche.
  // DAY/NIGHT sind Orientierungshinweise, keine Live-Erkennung.
  if (freq >= 1843 && freq <= 2000)
    return "QSO - LSB NIGHT";

  if (freq >= 3600 && freq <= 3800)
    return "QSO - LSB NIGHT";

  if (freq >= 5354 && freq <= 5366)
    return "QSO - USB DAY/NIGHT";

  if (freq >= 7060 && freq <= 7200)
    return "QSO - LSB DAY/NIGHT";

  if (freq >= 14100 && freq <= 14350)
    return "QSO - USB DAY";

  if (freq >= 18110 && freq <= 18168)
    return "QSO - USB DAY";

  if (freq >= 21150 && freq <= 21450)
    return "QSO - USB DAY";

  if (freq >= 24931 && freq <= 24990)
    return "QSO - USB DAY";

  // 10 m: SSB-Sprachfunk und FM-Bereich.
  if (freq >= 28300 && freq < 29100)
    return "QSO - USB DAY";

  if (freq >= 29100 && freq <= 29700)
    return "QSO - FM DAY";

  // 7. Kurzwellenrundfunk - typische internationale Rundfunksegmente.
  // Spezifische Digital-, CW-, QSO- und SSTV-Regeln haben Vorrang.
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
    return "SW-RADIO AM";
  }

  // 8. Utility-Funk - vorlaeufige Orientierung in typischen HF-Segmenten.
  // Utility ist vielfaeltig; die Zuordnung muss spaeter noch verfeinert werden.
  if ((freq >= 4000  && freq < 4438)  ||
      (freq >= 6200  && freq < 6525)  ||
      (freq >= 8800  && freq < 9040)  ||
      (freq >= 11175 && freq < 11400) ||
      (freq >= 13200 && freq < 13360) ||
      (freq >= 15010 && freq < 15100) ||
      (freq >= 17900 && freq < 18030) ||
      (freq >= 21870 && freq < 21924))
  {
    return "UTILITY - USB";
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
