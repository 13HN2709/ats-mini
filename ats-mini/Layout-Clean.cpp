
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

void drawLayoutClean()
{
  // 1. Batterie & Spannung oben rechts
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Oben links: Bandname
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0x07E0);
  spr.drawString(getCurrentBand()->bandName, 10, 2);

  // 3. Modus-Kasten
  int modeBoxX = 10;
  int modeBoxY = 20;
  int modeBoxW = 48;
  int modeBoxH = 18;

  spr.fillRoundRect(
    modeBoxX, modeBoxY, modeBoxW, modeBoxH, 4, 0x07E0
  );

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0x0000);
  spr.drawString(
    bandModeDesc[currentMode],
    modeBoxX + (modeBoxW / 2),
    modeBoxY + (modeBoxH / 2)
  );

  // 4. RDS: Sendername und Radiotext
  if (currentMode == FM)
  {
    // Sendername wie im Layout-Default abrufen
    const char* stationName = getStationName();

    if (stationName != nullptr && *stationName)
    {
      // Sonderformat für lange Sendernamen beachten
      if ((uint8_t)*stationName == 0xFF)
        stationName++;

      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(TC_DATUM);
      spr.setTextColor(0x07E0);
      spr.drawString(stationName, 160, 20);
    }

    // RDS-Textfeld
    const int boxX = 20;
    const int boxY = 105;
    const int boxW = 280;
    const int boxH = 36;

    // Hintergrund und Rahmen
    spr.fillRoundRect(
      boxX, boxY, boxW, boxH, 5, 0x0000
    );

    spr.drawRoundRect(
      boxX, boxY, boxW, boxH, 5, 0x07E0
    );

    // Radiotext linksbündig anzeigen
    const char* rt = getRadioText();

    if (rt != nullptr && *rt)
    {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextColor(0xFFFF);
      spr.setTextDatum(TL_DATUM);

      // Text innerhalb des Kastens begrenzen
      spr.setClipRect(
        boxX + 5,
        boxY + 3,
        boxW - 10,
        boxH - 6
      );

      // Der Text beginnt links und wird nicht verschoben
      spr.drawString(rt, boxX + 8, boxY + 8);

      spr.clearClipRect();
    }
  }

  // 5. Frequenzanzeige: Orbitron, zentriert
  char freqBuf[24];
  const char* unitBuf;

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

  // Frequenz zeichnen
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(0xFFFF);
  spr.drawString(freqBuf, 160, 68);

  // Breite der Frequenz ermitteln
  int freqWidth = spr.textWidth(freqBuf);

  // Einheit direkt hinter der Frequenz
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(0x9CD3);

  int unitX = 160 + (freqWidth / 2) + 4;
  spr.drawString(unitBuf, unitX, 68);

  // 6. S-Meter: untere Displayzeile
  int barW = 220;
  int barH = 7;
  int barX = (320 - barW) / 2;
  int barY = 160;

  int currentBarW = map(
    constrain(rssi, 0, 60),
    0, 60,
    5, barW
  );

  // Hintergrund-Schiene
  spr.fillRoundRect(
    barX, barY, barW, barH, 3, 0x2104
  );

  // Signal-Ausschlag
  if (currentBarW > 0)
  {
    spr.fillRoundRect(
      barX, barY, currentBarW, barH, 3, 0x07E0
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
