#include "Common.h"
#include "Themes.h"
#include "Storage.h"
#include "Utils.h"
#include "Menu.h"
#include "BleMode.h"
#include "Draw.h"
#include "FreqKeypad.h" // <--- Eingebundenes Keypad-Modul

// Layout-Prototypen
void drawLayoutSmeter();
void drawLayoutNewUI();
void drawLayoutDecoder();
void drawLayoutDefault();
void drawLayoutClean();
void drawLayoutTEF6686();
static void drawDualRxScreen()
{
  const uint16_t panelBg = 0x0861, frameBlue = 0x249F, brightBlue = 0x3DFF;
  spr.fillSprite(0x0000);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(brightBlue);
  spr.setFont(&lgfx::fonts::Orbitron_Light_24);
  spr.drawString("DUAL RX", 160, 12);
  spr.drawFastHLine(12, 27, 296, frameBlue);
  if(dualRxSetupStage < 3)
  {
    spr.setFont(FONT_SMALL);
    spr.setTextColor(0xFFFF);
    spr.drawString(dualRxSetupStage == 1 ? "SET FREQUENCY AND MODE A" : "SET FREQUENCY AND MODE B", 160, 58);
    spr.setTextColor(0xBDF7);
    spr.drawString("Press encoder to open frequency keypad", 160, 86);
    spr.drawString("Choose frequency, mode and press OK", 160, 106);
    spr.setTextColor(brightBlue);
    spr.drawString(dualRxSetupStage == 1 ? "STEP 1 OF 2" : "STEP 2 OF 2", 160, 139);
    spr.setTextColor(0x7BEF);
    spr.drawString("Long press: cancel", 160, 175);
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
    spr.drawString(i == 0 ? "A" : "B", 19, y+7);
    spr.setTextColor((dualRxSetupStage == 4 && i == dualRxDisplaySlot) ? 0x07E0 : 0x7BEF);
    spr.drawString((dualRxSetupStage == 4 && i == dualRxDisplaySlot) ? "RX" : "WAIT", 19, y+32);
    char freqText[24];
    bool isFm = v.band < getTotalBands() && bands[v.band].bandType == FM_BAND_TYPE;
    if(isFm) snprintf(freqText, sizeof(freqText), "%.2f", v.freq / 100.0f);
    else snprintf(freqText, sizeof(freqText), "%u", v.freq);
    spr.setFont(&lgfx::fonts::Orbitron_Light_24);
    spr.setTextDatum(MR_DATUM);
    spr.setTextColor(0xFFFF);
    spr.drawString(freqText, 258, y+8);
    spr.setFont(FONT_SMALL);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(brightBlue);
    spr.drawString(isFm ? "MHz" : "kHz", 265, y+29);
    spr.setTextColor(0xBDF7);
    spr.drawString(v.mode < getTotalModes() ? bandModeDesc[v.mode] : "?", 265, y+42);
  }
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(dualRxSetupStage == 4 ? 0x07E0 : brightBlue);
  spr.drawString(dualRxSetupStage == 4 ? "A/B ALTERNATING SCAN" : "PRESS TO START DUAL RX", 160, 165);
  spr.setTextColor(0x7BEF);
  spr.drawString(dualRxSetupStage == 4 ? "PRESS ENCODER TO EXIT" : "Long press: cancel", 160, 184);
}

//
// Draw screen according to given command
//
void drawScreen()
{
  if(sleepOn()) return;

  // Keypad-Overlay sofort zeichnen und anzeigen, wenn aktiv
  if(currentCmd == CMD_FREQ_KEYPAD)
  {
    drawFreqKeypad();
    spr.pushSprite(0, 0);
    return;
  }

  spr.fillSprite(TH.bg);

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
