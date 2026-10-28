#include "Common.h"
#include "Themes.h"
#include "Storage.h"
#include "Utils.h"
#include "Menu.h"
#include "BleMode.h"
#include "Draw.h"
#include "FreqKeypad.h" // <--- Eingebundenes Keypad-Modul
#include "SpectrumScanner.h"

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
// Draw band and mode indicators (Farbe auf Weiß 0xFFFF angepasst)
//
void drawBandAndMode(const char *band, const char *mode, int x, int y)
{
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFFFF);
  uint16_t band_width = spr.drawString(band, x, y);

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(0xFFFF);
  spr.drawString(mode, x + band_width / 2 + 10, y);
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
// Draw frequency
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
    spr.drawFloat(freq/100.00, 2, x, y, FONT_DIGITS);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(unitColor);
    spr.drawString("MHz", ux, uy + 10, &lgfx::fonts::FreeSansBold9pt7b);
  }
  else
  {
    li = hl<ITEM_COUNT(hlDigitsAMSSB)? &hlDigitsAMSSB[hl] : 0;

    if(isSSB())
    {
      char text[32];
      freq = freq * 1000 + currentBFO;
      sprintf(text, "%3.3lu", freq / 1000);
      spr.drawString(text, x, y, FONT_DIGITS);
      spr.setTextDatum(ML_DATUM);
      sprintf(text, ".%3.3lu", freq % 1000);
      spr.drawString(text, 4+x, 17+y, FONT_LARGE);
    }
    
else
{
  // AM: Ganzzahl ohne künstliche Nachkommastellen
  spr.drawNumber(freq, x, y, FONT_DIGITS);
}

    spr.setTextColor(unitColor);
    spr.setTextDatum(ML_DATUM);
    spr.drawString("kHz", ux, uy + 10, &lgfx::fonts::FreeSansBold9pt7b);
  }

  if(li)
  {
    if(selectOn)
    {
      spr.fillRoundRect(li->x + 1, li->y - 1, li->w - 2, 3, 1, TH.freq_hl_sel);
      spr.fillTriangle(li->x, li->y, li->x + 2, li->y - 2, li->x + 2, li->y - 2, TH.freq_hl_sel);
      spr.fillTriangle(li->x + li->w - 1, li->y, li->x + li->w - 3, li->y - 2, li->x + li->w - 3, li->y + 2, TH.freq_hl_sel);
    }
    else
    {
      spr.fillRoundRect(li->x, li->y - 1, li->w, 3, 1, TH.freq_hl);
    }
  }
}

//
// Draw tuner scale Roehrenradiostyle
//
void drawScale(uint32_t freq)
{
  spr.fillCircle(160, 140, 5, 0xF800);
  spr.drawFastVLine(160, 145, 24, 0xF800);
  spr.drawFastVLine(161, 145, 24, 0xF800);
  spr.drawFastHLine(0, 148, 320, TH.scale_line);
  spr.drawFastHLine(0, 169, 320, TH.scale_line);

  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.scale_text);

  int16_t slack = 3;
  int16_t offset = ((freq % 10) / 10.0 + slack) * 8;
  freq = freq / 10 - 20 - slack;

  const Band *band = getCurrentBand();
  uint32_t minFreq = band->minimumFreq / 10;
  uint32_t maxFreq = band->maximumFreq / 10;

  for(int i=0 ; i<(slack + 41 + slack) ; i++, freq++)
  {
    int16_t x = i * 8 - offset;
    if(freq >= minFreq && freq <= maxFreq)
    {
      uint16_t lineColor = (i==20) && (!offset || (!(freq%5) && offset==1))?
        0xF800 : TH.scale_line;

      if((freq % 10) == 0)
      {
        spr.drawFastVLine(x, 149, 20, lineColor);
        spr.drawFastVLine(x + 1, 149, 20, lineColor);

        if(currentMode == FM)
          spr.drawFloat(freq / 10.0, 1, x, 140, FONT_SMALL);
        else if(freq >= 100)
          spr.drawFloat(freq / 100.0, 3, x, 140, FONT_SMALL);
        else
          spr.drawNumber(freq * 10, x, 140, FONT_SMALL);
      }
      else if((freq % 5) == 0 && (freq % 10) != 0)
      {
        spr.drawFastVLine(x, 157, 12, lineColor);
      }
      else
      {
        spr.drawFastVLine(x, 162, 7, lineColor);
      }
    }
  }
}

//
// Standard S-Meter
//
void drawSMeter(int strength, int x, int y)
{
  static int peakBar = 0;
  static uint32_t lastPeakTime = 0;
  static uint32_t lastDecayTime = 0;

  uint32_t now = millis();

  if (strength >= peakBar) {
    peakBar = strength;
    lastPeakTime = now;
    lastDecayTime = now;
  } 
  else if (now - lastPeakTime > 1200) {
    if (now - lastDecayTime > 150) {
      if (peakBar > 0) peakBar--;
      lastDecayTime = now;
    }
  }

  spr.drawTriangle(x + 1, y + 1, x + 11, y + 1, x + 6, y + 6, 0xFCA0);
  spr.drawLine(x + 6, y + 1, x + 6, y + 14, 0xFCA0);

  for(int i = 0; i < 17; i++)
  {
    int barX = 15 + x + (i * 4);

    if (i == peakBar && peakBar > 0)
    {
      spr.fillRect(barX, 2 + y, 2, 12, 0xF800);
    }
    else if (i < 10 && i < strength)
    {
      spr.fillRect(barX, 2 + y, 2, 12, 0xFCA0);
    }
    else if (i < strength)
    {
      spr.fillRect(barX, 2 + y, 2, 12, TH.smeter_bar_plus);
    }
    else
    {
      spr.fillRect(barX, 2 + y, 2, 12, TH.smeter_bar_empty);
    }
  }
}

//
// Kompaktes S-Meter im H.J. Berndt Design
//
void drawCompactSMeter(int strength, int x, int y)
{
  int smeterX = x;
  int smeterY = y;

  spr.drawRoundRect(smeterX, smeterY, 96, 22, 2, 0xFFFF);
  
  spr.fillTriangle(smeterX + 6, smeterY + 11, smeterX + 12, smeterY + 6, smeterX + 12, smeterY + 16, 0xFFFF);
  spr.fillTriangle(smeterX + 90, smeterY + 11, smeterX + 84, smeterY + 6, smeterX + 84, smeterY + 16, 0xFFFF);

  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(0xFFFF);
  char sigText[16];
  sprintf(sigText, "SIG %02d/%02d", rx.getCurrentRSSI(), rx.getCurrentSNR());
  spr.drawString(sigText, smeterX + 48, smeterY + 3, FONT_SMALL);

  int bars = map(strength, 0, 60, 0, 10);
  bars = constrain(bars, 0, 10);
  for (int i = 0; i < 10; i++) {
    uint16_t color = (i < bars) ? 0x07E0 : TH.smeter_bar_empty;
    spr.fillRect(smeterX + 16 + (i * 6), smeterY + 14, 4, 6, color);
  }
}

//
// Draw stereo indicator
//
void drawStereoIndicator(int x, int y, bool stereo)
{
  if(stereo)
  {
    spr.fillRect(15 + x, 7 + y, 4 * 17 - 2, 2, TH.bg);
  }
}

//
// Draw RDS station name
//
void drawStationName(const char *name, int x, int y)
{
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(TH.rds_text);
  spr.drawString(name, x, y, FONT_LARGE);
}

//
// Draw long (EIBI) station name
//
void drawLongStationName(const char *name, int x, int y)
{
  int width = spr.textWidth(name, FONT_SMALL);
  spr.setTextColor(TH.rds_text);

  if((x + width) >= 320)
  {
    spr.setTextDatum(TL_DATUM);
    spr.drawString(name, x, y, FONT_SMALL);
  }
  else if(width <= 60)
  {
    spr.setTextDatum(TC_DATUM);
    spr.drawString(name, x + (320 - x) / 3, y, FONT_SMALL);
  }
  else
  {
    spr.setTextDatum(TC_DATUM);
    spr.drawString(name, x + (320 - x + width) / 4, y, FONT_SMALL);
  }
}

//
// Draw scan graphs
//
void drawScanGraphs(uint32_t freq)
{
  int16_t offset = (freq % 10) / 10.0 * 8;
  freq = freq / 10 - 20;

  const Band *band = getCurrentBand();
  uint32_t minFreq = band->minimumFreq / 10;
  uint32_t maxFreq = band->maximumFreq / 10;

  for(int i=0 ; i<41 ; i++, freq++)
  {
    int16_t x = i * 8 - offset;

    if(freq >= minFreq && freq <= maxFreq)
    {
      if((freq % 5) == 0) {
        for(int y=0; y<42; y+=2) {
          spr.drawPixel(x, 169-y, TH.scan_grid);
        }
      }

      if((freq+1) <= maxFreq) {
        for(int xd=x; xd<(x+8); xd+=2) {
          spr.drawPixel(xd, 169-40, TH.scan_grid);
          spr.drawPixel(xd, 169-30, TH.scan_grid);
          spr.drawPixel(xd, 169-20, TH.scan_grid);
          spr.drawPixel(xd, 169-10, TH.scan_grid);
          spr.drawPixel(xd, 169-0, TH.scan_grid);
        }
        int snr1 = 40 * scanGetSNR(freq * 10);
        int snr2 = 40 * scanGetSNR((freq+1) * 10);
        spr.drawLine(x, 169-snr1, x+8, 169-snr2, TH.scan_snr);
        int rssi1 = 40 * scanGetRSSI(freq * 10);
        int rssi2 = 40 * scanGetRSSI((freq+1) * 10);
        spr.drawLine(x, 169-rssi1, x+8, 169-rssi2, TH.scan_rssi);
      }
    }
  }
  spr.fillTriangle(156, 125, 160, 130, 164, 125, TH.scale_pointer);
  spr.drawLine(160, 130, 160, 169, TH.scale_pointer);
}

static void drawDualRxScreen()
{
  const uint16_t panelBg = 0x0861, frameBlue = 0x001F, brightBlue = 0x07FF;
  // Dual RX alternates between A and B, so retain each slot's last measured RSSI.
  static int slotStrength[2] = {0, 0};
  if (dualRxSetupStage == 4 && dualRxDisplaySlot < 2)
    slotStrength[dualRxDisplaySlot] = rx.getCurrentRSSI();

  spr.fillSprite(0x0000);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(brightBlue);
  // TEF6686 layout uses FreeSans Bold for status text and the MHz/kHz label.
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString("DUAL RX", 160, 12);
  spr.drawFastHLine(12, 27, 296, frameBlue);
  if(dualRxSetupStage < 3)
  {
    spr.setFont(FONT_SMALL);
    spr.setTextColor(0xFFFF);
    spr.drawString(dualRxSetupStage == 1 ? "SET FREQUENCY AND MODE A" : "SET FREQUENCY AND MODE B", 160, 58);
    spr.setTextColor(0xBDF7);
    if(dualRxSetupStage == 1)
    {
      spr.drawString("Press encoder to open frequency keypad", 160, 86);
      spr.drawString("Choose frequency, mode and press OK", 160, 106);
    }
    else
    {
      char savedA[40];
      const VfoState &a = dualRxVfos[0];
      const bool aIsFm = a.band < getTotalBands() && bands[a.band].bandType == FM_BAND_TYPE;
      if(aIsFm) snprintf(savedA, sizeof(savedA), "A: %.2f MHz  %s", a.freq / 100.0f, bandModeDesc[a.mode]);
      else snprintf(savedA, sizeof(savedA), "A: %u kHz  %s", a.freq, bandModeDesc[a.mode]);
      spr.setTextColor(0x07E0);
      spr.drawString(savedA, 160, 84);
      spr.setTextColor(0xBDF7);
      spr.drawString("Press encoder to enter frequency B", 160, 108);
    }
    spr.setTextColor(brightBlue);
    spr.drawString(dualRxSetupStage == 1 ? "STEP 1 OF 2" : "STEP 2 OF 2", 160, 139);
    return;
  }
  for(uint8_t i=0; i<2; ++i)
  {
    int y = i == 0 ? 32 : 99;
    const VfoState &v = dualRxVfos[i];
    spr.fillRect(10, y, 300, 56, panelBg);
    spr.drawRect(10, y, 300, 56, frameBlue);
    spr.drawRect(11, y+1, 298, 54, brightBlue);
    spr.setFont(FONT_SMALL);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(0xBDF7);
    // Move the complete A/B + RX/WAIT labels slightly down so they clear the top frame.
    spr.drawString(i == 0 ? "A" : "B", 19, y+12);
    spr.setTextColor((dualRxSetupStage == 4 && i == dualRxDisplaySlot) ? 0x07E0 : 0x7BEF);
    spr.drawString((dualRxSetupStage == 4 && i == dualRxDisplaySlot) ? "RX" : "WAIT", 36, y+12);
    char freqText[24];
    bool isFm = v.band < getTotalBands() && bands[v.band].bandType == FM_BAND_TYPE;
    if(isFm) snprintf(freqText, sizeof(freqText), "%.2f", v.freq / 100.0f);
    else snprintf(freqText, sizeof(freqText), "%u", v.freq);
    // Keep the exact 7-segment FONT_DIGITS used by Layout-Default.
    // MR_DATUM is vertically centered, so y+28 centers the 48px digits in the 56px frame.
    spr.setFont(FONT_DIGITS);
    spr.setTextDatum(MR_DATUM);
    spr.setTextColor(0xFFFF);
    spr.drawString(freqText, 258, y+28);
    spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(0x07E0);
    spr.drawString(isFm ? "MHz" : "kHz", 265, y+27);
    spr.setFont(FONT_SMALL);
    spr.setTextColor(0xBDF7);
    spr.drawString(v.mode < getTotalModes() ? bandModeDesc[v.mode] : "?", 265, y+41);

    // Compact per-slot S-meter at lower left. Values are retained between A/B scans.
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(0xBDF7);
    spr.drawString("S", 19, y+25);
    int bars = constrain(map(slotStrength[i], 0, 60, 0, 10), 0, 10);
    for (int bar = 0; bar < 10; ++bar) {
      uint16_t barColor = bar < bars ? (bar < 7 ? 0x07E0 : 0xFFE0) : 0x4208;
      spr.fillRect(31 + bar * 5, y+40, 3, 9, barColor);
    }
  }
  // Keep the encoder instruction visible within the 170px display height.
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0x07E0);
  spr.drawString(dualRxSetupStage == 4 ? "Click Encoder to Stop Dual RX" : "Click Encoder to Start", 160, 165);
}

//
// Draw screen according to given command
//
void drawScreen()
{
  if(sleepOn()) return;

  // Full-screen TEF6686-style menu replaces the compact sidebar menu.
  if(currentCmd == CMD_MENU || currentCmd == CMD_SETTINGS || currentCmd == CMD_SPECTRUM_MENU)
  {
    drawFullScreenMenu(currentCmd);
    spr.pushSprite(0, 0);
    return;
  }

  // Keypad-Overlay sofort zeichnen und anzeigen, wenn aktiv
  if(currentCmd == CMD_FREQ_KEYPAD)
  {
    drawFreqKeypad();
    spr.pushSprite(0, 0);
    return;
  }

  spr.fillSprite(TH.bg);

  if(currentCmd == CMD_SPECTRUM)
  {
    drawSpectrumScanner();
    spr.pushSprite(0, 0);
    return;
  }

  if(currentCmd == CMD_DUAL_RX)
  {
    drawDualRxScreen();
    spr.pushSprite(0, 0);
    return;
  }

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
