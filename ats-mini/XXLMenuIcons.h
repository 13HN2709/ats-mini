#pragma once
#include <stdint.h>
#include "Common.h"

// Compact 1-bit bitmap artwork for the XXL menu. Rendered at 3x with
// transparent background and cyan/red/blue neon palette; no runtime assets.
static const uint8_t XXL_ICON_BITMAPS[17][8] = {
  {0b00111100, 0b01100110, 0b11011011, 0b10111101, 0b10100101, 0b11011011, 0b01100110, 0b00111100}, // gear
  {0b00011000, 0b00111100, 0b01111110, 0b11011011, 0b00011000, 0b00011000, 0b00011000, 0b00011000}, // mode
  {0b10001000, 0b10001000, 0b10101010, 0b10101010, 0b10111010, 0b10111010, 0b11111010, 0b00000000}, // band
  {0b00010000, 0b00110000, 0b11111100, 0b11111110, 0b11111110, 0b11111100, 0b00110000, 0b00010000}, // volume
  {0b11111111, 0b10011001, 0b10011001, 0b11111111, 0b10011001, 0b10011001, 0b11111111, 0b00000000}, // keypad
  {0b00111100, 0b01000010, 0b10100101, 0b10000001, 0b10100101, 0b01000010, 0b00111100, 0b00000000}, // dual
  {0b00000000, 0b00100100, 0b01011010, 0b10100101, 0b10100101, 0b01011010, 0b00100100, 0b00000000}, // wave
  {0b00011100, 0b00100110, 0b01000010, 0b01011010, 0b01011010, 0b01000010, 0b00100100, 0b00011000}, // scan
  {0b01111110, 0b01000010, 0b01011010, 0b01011010, 0b01000010, 0b01011010, 0b01111110, 0b00000000}, // memory
  {0b10000001, 0b01000010, 0b00100100, 0b00011000, 0b00011000, 0b00100100, 0b01000010, 0b10000001}, // squelch
  {0b00000000, 0b11111111, 0b00110000, 0b00000000, 0b11111111, 0b00001100, 0b00000000, 0b11111111}, // sliders
  {0b00011000, 0b01100110, 0b11000011, 0b00011000, 0b00100100, 0b00011000, 0b00011000, 0b00000000}, // wifi
  {0b10000001, 0b01000010, 0b00100100, 0b00011000, 0b00100100, 0b01000010, 0b10000001, 0b00000000}, // bluetooth
  {0b00111100, 0b01000010, 0b10011001, 0b10001001, 0b10001101, 0b01000010, 0b00111100, 0b00000000}, // clock
  {0b11111111, 0b10000001, 0b10111101, 0b10100101, 0b10111101, 0b10000001, 0b11111111, 0b00011000}, // display
  {0b01111110, 0b01000010, 0b01011010, 0b01011010, 0b01000010, 0b01011010, 0b01111110, 0b00000000}, // eibi
  {0b00011000, 0b00111100, 0b01111110, 0b11011011, 0b00011000, 0b00011000, 0b00011000, 0b00000000}, // update
};
static const uint8_t XXL_ICON_MENU_MAP[18] = {1,2,3,4,5,6,7,8,9,10,11,12,11,13,13,14,15,0};
static const uint8_t XXL_ICON_SETTINGS_MAP[22] = {14,13,13,15,15,14,14,14,14,1,1,1,11,11,16,16,15,15,12,11,0,8};

static void drawXXLBitmapIcon(int icon, int x, int y, uint16_t accent)
{
  // Clean vector-like line icons: readable at 320x170 without chunky pixels.
  // Test pass: every icon uses the same cyan as the menu text, with no red accent.
  const uint16_t cyan = accent ? accent : 0x07FF;
  const uint16_t blue = cyan;
  const uint16_t red = cyan;
  const uint16_t c = cyan;
  const int cx = x + 20, cy = y + 20;
  switch(icon)
  {
    case 0: // settings / gear
      spr.drawCircle(cx, cy, 12, c); spr.drawCircle(cx, cy, 5, cyan);
      spr.drawFastHLine(cx-16,cy,5,blue); spr.drawFastHLine(cx+12,cy,5,blue);
      spr.drawFastVLine(cx,cy-16,5,blue); spr.drawFastVLine(cx,cy+12,5,blue);
      spr.drawLine(cx-11,cy-11,cx-7,cy-7,c); spr.drawLine(cx+7,cy+7,cx+11,cy+11,c);
      spr.drawLine(cx-11,cy+11,cx-7,cy+7,c); spr.drawLine(cx+7,cy-7,cx+11,cy-11,c); break;
    case 1: // modulation
      spr.drawLine(cx,cy-15,cx,cy+15,c); spr.drawLine(cx-12,cy-8,cx+12,cy-8,cyan);
      spr.drawLine(cx-12,cy+8,cx+12,cy+8,cyan); spr.drawCircle(cx,cy,5,blue); break;
    case 2: // band / antenna
      spr.drawLine(cx,cy-14,cx,cy+12,c); spr.drawLine(cx-8,cy+12,cx+8,cy+12,c);
      spr.drawLine(cx,cy-14,cx-12,cy-2,cyan); spr.drawLine(cx,cy-14,cx+12,cy-2,cyan);
      spr.drawArc(cx,cy-12,10,10,210,330,blue); spr.drawArc(cx,cy-12,16,16,210,330,c); break;
    case 3: // volume
      spr.fillTriangle(cx-14,cy-5,cx-6,cy-5,cx-6,cy+6,c);
      spr.drawLine(cx-6,cy-5,cx+1,cy-11,c); spr.drawLine(cx-6,cy+6,cx+1,cy+11,c);
      spr.drawArc(cx,cy,12,12,300,60,cyan); spr.drawArc(cx,cy,18,18,300,60,blue); break;
    case 4: // keypad
      spr.drawRoundRect(cx-13,cy-15,26,30,3,c);
      for(int rr=0;rr<3;rr++) for(int cc=0;cc<3;cc++) spr.fillCircle(cx-7+cc*7,cy-8+rr*7,1,cyan);
      break;
    case 5: // dual RX
      spr.drawCircle(cx-6,cy,7,c); spr.drawCircle(cx+7,cy,7,cyan);
      spr.drawFastHLine(cx-2,cy,5,blue); break;
    case 6: // VFO / signal wave
      spr.drawLine(cx-15,cy,cx-10,cy,c); spr.drawLine(cx-10,cy,cx-5,cy-10,c);
      spr.drawLine(cx-5,cy-10,cx+1,cy+10,cyan); spr.drawLine(cx+1,cy+10,cx+7,cy-6,cyan);
      spr.drawLine(cx+7,cy-6,cx+11,cy,c); spr.drawLine(cx+11,cy,cx+15,cy,c); break;
    case 7: // scan
      spr.drawCircle(cx-2,cy-2,9,c); spr.drawLine(cx+5,cy+5,cx+14,cy+14,c);
      spr.drawFastHLine(cx-7,cy-2,10,cyan); spr.drawFastVLine(cx-2,cy-7,10,blue); break;
    case 8: // memory
      spr.drawRoundRect(cx-12,cy-15,24,30,3,c); spr.drawFastHLine(cx-6,cy-7,12,cyan);
      spr.drawFastHLine(cx-6,cy,12,cyan); spr.drawFastHLine(cx-6,cy+7,8,blue); break;
    case 9: // squelch
      spr.drawCircle(cx,cy,13,c); spr.drawCircle(cx,cy,7,cyan);
      spr.drawLine(cx,cy,cx+9,cy-8,blue); break;
    case 10: // sliders / bandwidth
      spr.drawFastVLine(cx-9,cy-13,26,c); spr.drawFastVLine(cx,cy-13,26,cyan); spr.drawFastVLine(cx+9,cy-13,26,blue);
      spr.fillRoundRect(cx-13,cy-6,8,6,2,c); spr.fillRoundRect(cx-4,cy+4,8,6,2,cyan); spr.fillRoundRect(cx+5,cy-10,8,6,2,blue); break;
    case 11: // wifi / connectivity
      spr.drawArc(cx,cy+7,6,6,205,335,c); spr.drawArc(cx,cy+7,12,12,205,335,cyan);
      spr.drawArc(cx,cy+7,18,18,205,335,blue); spr.fillCircle(cx,cy+10,2,c); break;
    case 12: // bluetooth
      spr.drawLine(cx,cy-15,cx,cy+15,c); spr.drawLine(cx,cy-15,cx+9,cy-6,cyan);
      spr.drawLine(cx+9,cy-6,cx-8,cy+8,cyan); spr.drawLine(cx-8,cy+8,cx,cy+15,c);
      spr.drawLine(cx,cy-1,cx+9,cy+8,blue); spr.drawLine(cx+9,cy+8,cx,cy+15,c); break;
    case 13: // clock
      spr.drawCircle(cx,cy,13,c); spr.drawLine(cx,cy,cx,cy-8,cyan); spr.drawLine(cx,cy,cx+7,cy+4,blue); break;
    case 14: // display / brightness
      spr.drawRoundRect(cx-15,cy-10,30,20,3,c); spr.drawFastHLine(cx-8,cy+14,16,cyan);
      spr.drawFastVLine(cx,cy+10,4,cyan); spr.fillCircle(cx,cy,4,blue); break;
    case 15: // radio text / memory card
      spr.drawRoundRect(cx-13,cy-13,26,26,3,c); spr.drawFastHLine(cx-7,cy-6,14,cyan);
      spr.drawFastHLine(cx-7,cy,14,cyan); spr.drawFastHLine(cx-7,cy+6,10,blue); break;
    case 16: // update
      spr.drawArc(cx,cy,12,12,35,305,c); spr.fillTriangle(cx+8,cy-11,cx+15,cy-10,cx+12,cy-3,cyan);
      spr.drawFastHLine(cx-6,cy,12,blue); spr.drawFastVLine(cx,cy-6,12,cyan); break;
  }
}
