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
  
const char* rdsStation = rx.getRdsStationName();
const char* rt = getRadioText();
const char* pi = getProgramInfo();

static unsigned long lastRdsSwitch = 0;
static bool showRadioText = false;

bool hasStation = (rdsStation && *rdsStation);
bool hasText = (rt && *rt);
bool hasInfo = (pi && *pi);
bool hasRdsText = hasText || hasInfo;

if (currentMode == FM) {
  if (hasRdsText && millis() - lastRdsSwitch >= 10000) {
    lastRdsSwitch = millis();
    showRadioText = !showRadioText;
  }

  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0x07E0);

  if (showRadioText && hasText) {
    spr.drawString(rt, 160, 20);
  } else if (showRadioText && hasInfo) {
    spr.drawString(pi, 160, 20);
  } else if (hasStation) {
    spr.drawString(rdsStation, 160, 20);
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
