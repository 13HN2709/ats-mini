#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"

void drawLayoutClean()
{
  // 1. Batterie & Spannung oben rechts
  bool has_voltage = drawBattery(BATT_OFFSET_X, BATT_OFFSET_Y);

  // 2. Oben Links: Bandname (z.B. VHF) ganz oben
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0x07E0); // Neongrün
  spr.drawString(getCurrentBand()->bandName, 10, 2);

  // Modus-Kasten (FM) direkt UNTER dem Bandnamen (2. Zeile)
  int modeBoxX = 10;
  int modeBoxY = 20;
  int modeBoxW = 48;
  int modeBoxH = 18;

  spr.fillRoundRect(modeBoxX, modeBoxY, modeBoxW, modeBoxH, 4, 0x07E0);
  
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0x0000); // Schwarzer Text auf neongrünem Grund
  spr.drawString(bandModeDesc[currentMode], modeBoxX + (modeBoxW / 2), modeBoxY + (modeBoxH / 2));


// RDS und Text
  
  // RDS-Sendername dauerhaft oben anzeigen
  const char* rdsStation = rx.getRdsStationName();
  const char* rt = getRadioText();

  if (currentMode == FM) {

    // Sendername oben
    if (rdsStation != nullptr && *rdsStation) {
      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextDatum(TC_DATUM);
      spr.setTextColor(0x07E0);
      spr.drawString(rdsStation, 160, 20);
    }

    // Position und Größe des RDS-Feldes
    const int boxX = 20;
    const int boxY = 105;
    const int boxW = 280;
    const int boxH = 36;

    // Dunkles Feld mit neongrünem Rahmen
    spr.fillRoundRect(boxX, boxY, boxW, boxH, 5, 0x0000);
    spr.drawRoundRect(boxX, boxY, boxW, boxH, 5, 0x07E0);

    if (rt != nullptr && *rt) {
      static unsigned long scrollStart = 0;
      static char previousText[256] = "";

      // Bei neuem Radiotext neu beginnen
      if (strncmp(rt, previousText, sizeof(previousText) - 1) != 0) {
        strncpy(previousText, rt, sizeof(previousText) - 1);
        previousText[sizeof(previousText) - 1] = '\0';
        scrollStart = millis();
      }

      spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
      spr.setTextColor(0xFFFF);
      spr.setTextDatum(TL_DATUM);

      int textW = spr.textWidth(rt);
      int textH = 20;

      // Langsamer Lauftext: 1 Pixel alle 60 ms
      unsigned long elapsed = millis() - scrollStart;
      int travel = textW + boxW;
      unsigned long scrollDuration = (unsigned long)travel * 60UL;
      unsigned long pauseDuration = 2000UL;
      unsigned long cycleDuration = scrollDuration + pauseDuration;

      int offset;

      if (elapsed < scrollDuration) {
        offset = elapsed / 60;
      } else {
        offset = travel;  // Text ist vollständig durchgelaufen
      }

      // Nach 2 Sekunden Pause beginnt der nächste Durchlauf
      if (elapsed >= cycleDuration) {
        scrollStart = millis();
        offset = 0;
      }

      int textX = boxX + boxW - offset;
      int textY = boxY + (boxH - textH) / 2;

      spr.setClipRect(boxX + 3, boxY + 2, boxW - 6, boxH - 4);
      spr.drawString(rt, textX, textY);
      spr.clearClipRect();
    }
  }
  
  // 4. Frequenzanzeige: Orbitron, zentriert, Einheit in derselben Zeile
  char freqBuf[24];
  const char* unitBuf;

  if (currentMode == FM)
  {
    snprintf(freqBuf, sizeof(freqBuf), "%lu.%02lu",
             (unsigned long)(currentFrequency / 100),
             (unsigned long)(currentFrequency % 100));
    unitBuf = "MHz";
  }
  else if (isSSB())
  {
    long long freq = (long long)currentFrequency * 1000LL + currentBFO;

    if (freq < 0) freq = 0;

    snprintf(freqBuf, sizeof(freqBuf), "%lld.%03lld",
             freq / 1000, freq % 1000);
    unitBuf = "kHz";
  }
  else
  {
    snprintf(freqBuf, sizeof(freqBuf), "%lu",
             (unsigned long)currentFrequency);
    unitBuf = "kHz";
  }

  // Frequenz zeichnen: exakt mittig
  spr.setFont(&lgfx::fonts::Orbitron_Light_32);
  spr.setTextDatum(CC_DATUM);
  spr.setTextColor(0xFFFF);
  spr.drawString(freqBuf, 160, 68);

  // Tatsächliche Breite der Frequenz ermitteln
  int freqWidth = spr.textWidth(freqBuf);

  // Einheit direkt hinter der Frequenz in derselben Zeile
  spr.setFont(&lgfx::fonts::FreeSans9pt7b);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(0x9CD3);

  int unitX = 160 + (freqWidth / 2) + 4;
  spr.drawString(unitBuf, unitX, 68);
  

  // S-Meter: letzte Zeile, horizontal zentriert
  int barW = 220;
  int barH = 7;
  int barX = (320 - barW) / 2;  // ergibt 50
  int barY = 160;               // unten im Display

  int currentBarW = map(constrain(rssi, 0, 60), 0, 60, 5, barW);

  // Hintergrund-Schiene
  spr.fillRoundRect(barX, barY, barW, barH, 3, 0x2104);

  // Signal-Ausschlag
  if (currentBarW > 0) {
    spr.fillRoundRect(barX, barY, currentBarW, barH, 3, 0x07E0);
  }

  // Sidebar / Menü falls aufgerufen
  if(currentCmd != CMD_NONE && currentCmd != CMD_SCAN && currentCmd != CMD_FREQ)
  {
    drawSideBar(currentCmd, MENU_OFFSET_X, MENU_OFFSET_Y, MENU_DELTA_X);
  }
}
