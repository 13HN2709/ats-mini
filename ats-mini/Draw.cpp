
#include "Common.h"
#include "Themes.h"
#include "Storage.h"
#include "Utils.h"
#include "Menu.h"
#include "BleMode.h"
#include "Draw.h"
#include "FreqKeypad.h" // Eingebundenes Keypad-Modul

// Layout-Prototypen
void drawLayoutSmeter();
void drawLayoutNewUI();
void drawLayoutDecoder();
void drawLayoutDefault();
void drawLayoutClean();
void drawLayoutTEF6686();

uint32_t lastTuneTime = 0;

//
// Draw preferences write indicator
//
void drawSaveIndicator(int x, int y)
{
  if(prefsAreWritten() || switchThemeEditor())
  {
    spr.fillRect(x+3, y+2, 3, 5, TH.save_icon);
    spr.fillTriangle(x+1, y+7, x+7, y+7, x+4, y+10, TH.save_icon);
    spr.drawLine(x, y+12, x, y+13, TH.save_icon);
    spr.drawLine(x, y+13, x+8, y+13, TH.save_icon);
    spr.drawLine(x+8, y+13, x+8, y+12, TH.save_icon);
  }
}

//
// Draw Bluetooth indicator
//
void drawBleIndicator(int x, int y)
{
  int8_t status = getBleStatus();

  if(status || switchThemeEditor())
  {
    uint16_t color = (status>0) ? TH.rf_icon_conn : TH.rf_icon;

    if(switchThemeEditor())
      color = millis()&0x2000? TH.rf_icon_conn : TH.rf_icon;

    spr.drawLine(x+3, y+1, x+3, y+13, color);
    spr.drawLine(x+3, y+1, x+6, y+4, color);
    spr.drawLine(x+6, y+4, x, y+10, color);
    spr.drawLine(x, y+4, x+6, y+10, color);
    spr.drawLine(x+6, y+10, x+3, y+13, color);
  }
}

//
// Draw WiFi indicator
//
void drawWiFiIndicator(int x, int y)
{
  int8_t status = getWiFiStatus();

  if(status || switchThemeEditor())
  {
    uint16_t color = (status>0) ? TH.rf_icon_conn : TH.rf_icon;

    if(switchThemeEditor())
      color = millis()&0x2000? TH.rf_icon_conn : TH.rf_icon;

    spr.fillArc(x, 15+y, 14, 13, 240, 300, color);
    spr.fillArc(x, 15+y, 9, 8, 240, 300, color);
    spr.fillArc(x, 15+y, 4, 3, 240, 300, color);
  }
}

//
// Draw operation status
//
bool drawStatus(int x, int y)
{
  if(statusLines[0][0] || statusLines[1][0])
  {
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(0x07E0);
    spr.drawString(statusLines[0], x, y, FONT_SMALL);
    spr.drawString(statusLines[1], x, y+17, FONT_SMALL);
    return(true);
  }

  return(false);
}

//
// Draw zoomed menu item
//
void drawZoomedMenu(const char *text, bool force)
{
  if (!zoomMenu && !force) return;

  spr.fillSmoothRoundRect(RDS_OFFSET_X - 72 + 1, RDS_OFFSET_Y - 3 + 1, 152, 26, 4, TH.menu_bg);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(TH.menu_item);
  spr.drawString(text, RDS_OFFSET_X + 5, RDS_OFFSET_Y, FONT_LARGE);
  spr.drawRoundRect(RDS_OFFSET_X - 72, RDS_OFFSET_Y - 3, 154, 28, 4, TH.menu_border);
}

//
// Show overlay message in large letters
//
void drawMessage(const char *msg)
{
  if(sleepOn()) return;

  drawZoomedMenu(msg, true);
  spr.pushSprite(0, 0);
}

//
// Draw band and mode indicators
//
void drawBandAndMode(const char *band, const char *mode, int x, int y)
{
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(TH.band_text);
  uint16_t band_width = spr.drawString(band, x, y);

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(TH.mode_text);
  uint16_t mode_width = spr.drawString(mode, x + band_width / 2 + 12, y + 8, FONT_SMALL);

  spr.drawRoundRect(x + band_width / 2 + 8, y + 7, mode_width + 8, 17, 4, TH.mode_border);
}

//
// Draw radio text
//
void drawRadioText(int y, int ymax)
{
  const char *rt = getRadioText();

  spr.setTextColor(0x07E0);
  spr.setTextSize(1.0);

  if (rt && *rt)
  {
    spr.setTextDatum(TC_DATUM);
    spr.drawString(rt, 160, y, 4);
  }
  else
  {
    spr.setTextDatum(TL_DATUM);

    char sigBuf[32];
    snprintf(sigBuf, sizeof(sigBuf), "S:%d | SNR:%d dB", rx.getCurrentRSSI(), rx.getCurrentSNR());
    spr.drawString(sigBuf, 90, y - 6, 4);
  }
}

//
// Draw frequency (Stabile Original-Logik nach Max Arnold)
//
void drawFrequency(uint32_t freq, int x, int y, int ux, int uy, uint8_t hl)
{
  static uint32_t lastFreq = 0;

  if (freq != lastFreq) {
    lastFreq = freq;
    lastTuneTime = millis();
  }

  uint16_t unitColor = ((millis() - lastTuneTime) < 800) ? 0xFFE0 : 0x07E0;

  struct Line { int x, y, w; };

  const Line hlDigitsFM[] =
  {
    { x - 30 - 32 * 0 -  0, y + 28, 27 },
    { x - 30 - 32 * 0 - 16, y + 28, 27 + 16 },
    { x - 30 - 32 * 1 -  0, y + 28, 27 },
    { x - 30 - 32 * 1 - 22, y + 28, 27 + 22 },
    { x - 30 - 32 * 2 - 12, y + 28, 27 },
    { x - 30 - 32 * 2 - 28, y + 28, 27 + 16 },
    { x - 30 - 32 * 3 - 12, y + 28, 27 },
    { x - 30 - 32 * 3 - 28, y + 28, 27 + 16 },
    { x - 30 - 32 * 4 +  4, y + 28, 11 },
  };

  const Line hlDigitsAMSSB[] =
  {
    { x + 12 + 14 * 2 -  0, y + 28, 12 },
    { x + 12 + 14 * 2 -  7, y + 28, 12 + 7 },
    { x + 12 + 14 * 1 -  0, y + 28, 12 },
    { x + 12 + 14 * 1 -  7, y + 28, 12 + 7 },
    { x + 12 + 14 * 0 -  0, y + 28, 12 },
    { x + 12 + 14 * 0 - 11, y + 28, 12 + 11 },
    { x - 30 - 32 * 0 -  0, y + 28, 27 },
    { x - 30 - 32 * 0 - 16, y + 28, 27 + 16 },
    { x - 30 - 32 * 1 -  0, y + 28, 27 },
    { x - 30 - 32 * 1 - 16, y + 28, 27 + 16 },
    { x - 30 - 32 * 2 -  0, y + 28, 27 },
    { x - 30 - 32 * 2 - 16, y + 28, 27 + 16 },
    { x - 30 - 32 * 3 -  0, y + 28, 27 },
    { x - 30 - 32 * 3 - 16, y + 28, 27 + 16 },
    { x - 30 - 32 * 4 -  0, y + 28, 27 },
  };

  bool selectOn = hl & 0x80;
  const struct Line *li;
  hl &= 0x7F;

  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(TH.freq_text);

  if(currentMode==FM)
  {
    li = hl<ITEM_COUNT(hlDigitsFM)? &hlDigitsFM[hl] : 0;
    spr.drawFloat(freq/1000000.0, 2, x, y, FONT_DIGITS);
    spr.setTextDatum(L_BASELINE);
    spr.setTextColor(unitColor);
    spr.drawString("MHz", ux, uy + 17);
  }
  else
  {
    li = hl<ITEM_COUNT(hlDigitsAMSSB)? &hlDigitsAMSSB[hl] : 0;

    if(isSSB())
    {
      char text[32];
      uint32_t ssbFreq = freq * 1000 + currentBFO;
      sprintf(text, "%3.3lu", ssbFreq / 1000);
      spr.drawString(text, x, y, FONT_DIGITS);
      spr.setTextDatum(ML_DATUM);
      sprintf(text, ".%3.3lu", ssbFreq % 1000);
      spr.drawString(text, 4+x, 17+y, FONT_LARGE);
    }
    else
    {
      // AM: Reine Ganzzahl ohne künstliches .000 (Crash-sicher)
      spr.drawNumber(freq / 1000, x, y, FONT_DIGITS);
    }

    spr.setTextDatum(L_BASELINE);
    spr.setTextColor(unitColor);
    spr.drawString("kHz", ux, uy + 19);
  }

  if(li)
  {
    if(selectOn)
    {
      spr.fillRoundRect(li->x + 1, li->y - 1, li->w - 2, 3, 1, TH.freq_hl_sel);
      spr.fillTriangle(li->x, li->y, li->x + 2, li->y - 2, li->x + 2, li->y + 2, TH.freq_hl_sel);
      spr.fillTriangle(li->x + li->w - 1, li->y, li->x + li->w - 3, li->y - 2, li->x + li->w - 3, li->y + 2, TH.freq_hl_sel);
    }
    else
    {
      spr.fillRoundRect(li->x, li->y - 1, li->w, 3, 1, TH.freq_hl);
    }
  }
}

//
// Draw tuner scale
//
void drawScale(uint32_t freq)
{
  spr.fillTriangle(156, 120, 160, 130, 164, 120, TH.scale_pointer);
  spr.drawLine(160, 130, 160, 169, TH.scale_pointer);

  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.scale_text);

  int16_t slack = 3;
  const uint32_t step = currentMode == FM ? 100000 : 10000;
  int16_t offset = (uint64_t)(freq % step) * 8 / step + slack * 8;
  int64_t tickFreq = ((int64_t)(freq / step) - 20 - slack) * step;

  const Band *band = getCurrentBand();
  uint32_t minFreq = band->minimumFreq;
  uint32_t maxFreq = band->maximumFreq;

  for(int i=0 ; i<(slack + 41 + slack) ; i++, tickFreq+=step)
  {
    int16_t x = i * 8 - offset;
    if(tickFreq >= minFreq && tickFreq <= maxFreq)
    {
      uint16_t lineColor = (i==20) && (!offset || (!(tickFreq % (5 * step)) && offset==1))?
        TH.scale_pointer : TH.scale_line;

      if((tickFreq % (10 * step)) == 0)
      {
        spr.drawLine(x, 169, x, 150, lineColor);
        spr.drawLine(x + 1, 169, x + 1, 150, lineColor);
        if(currentMode == FM)
          spr.drawFloat(tickFreq / 1000000.0, 1, x, 140, FONT_SMALL);
        else if(tickFreq >= 1000000)
          spr.drawFloat(tickFreq / 1000000.0, 3, x, 140, FONT_SMALL);
        else
          spr.drawNumber(tickFreq / 1000, x, 140, FONT_SMALL);
      }
      else if((tickFreq % (5 * step)) == 0 && (tickFreq % (10 * step)) != 0)
      {
        spr.drawLine(x, 169, x, 155, lineColor);
        spr.drawLine(x + 1, 169, x + 1, 155, lineColor);
      }
      else
      {
        spr.drawLine(x, 169, x, 160, lineColor);
      }
    }
  }
}

//
// Draw screen according to given command
//
void drawScreen()
{
  if(sleepOn()) return;

  if(currentCmd == CMD_FREQ_KEYPAD)
  {
    drawFreqKeypad();
    spr.pushSprite(0, 0);
    return;
  }

  spr.fillSprite(TH.bg);

  if(currentCmd==CMD_ABOUT)
  {
    drawAbout();
    return;
  }

  switch(uiLayoutIdx)
  {
    case 1:
      drawLayoutSmeter();
      break;
    case 2:
      drawLayoutNewUI();
      break;
    case 3:
      drawLayoutDecoder();
      break;
    case 4:
      drawLayoutClean();
      break;
    case 5:
      drawLayoutTEF6686();
      break;
    default:
      drawLayoutDefault();
      break;
  }

  spr.pushSprite(0, 0);
}
