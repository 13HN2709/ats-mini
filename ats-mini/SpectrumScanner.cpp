#include "Common.h"
#include "Menu.h"
#include "Themes.h"
#include "Draw.h"
#include "SpectrumScanner.h"

// The scan engine is shared with the web Spectrum Scanner.
void scanRunRange(uint16_t startFreq, uint16_t endFreq, uint16_t step);
uint16_t scanGetCount();
bool scanGetPoint(uint16_t index, uint16_t *freq, uint8_t *rssi, uint8_t *snr);

static constexpr uint16_t VHF_START = 8800;   // 88.00 MHz, frequency unit is 10 kHz
static constexpr uint16_t VHF_END   = 10800;  // 108.00 MHz
static constexpr uint16_t VHF_STEP  = 10;     // 100 kHz spacing, 201 samples

static uint16_t selectedPoint = 100;
static bool spectrumHasData = false;

void spectrumScannerStart()
{
  currentCmd = CMD_SPECTRUM;
  selectedPoint = 100;
  spectrumHasData = false;

  // The first implementation intentionally starts with the VHF preset.
  // Keep the user's original band and tune restored after the sweep.
  const uint8_t savedBand = bandIdx;
  const uint16_t savedFrequency = currentFrequency;
  const int16_t savedBFO = currentBFO;

  selectBand(0, false); // VHF / FM
  currentMode = FM;
  scanRunRange(VHF_START, VHF_END, VHF_STEP);
  spectrumHasData = scanGetCount() > 0;

  if(savedBand != bandIdx) selectBand(savedBand, false);
  currentBFO = savedBFO;
  if(savedFrequency != currentFrequency) updateFrequency(savedFrequency, false);
  currentBFO = savedBFO;

  if(spectrumHasData)
  {
    selectedPoint = scanGetCount() / 2;
    uint16_t freq = 0;
    if(scanGetPoint(selectedPoint, &freq, nullptr, nullptr))
    {
      // Leave the radio tuned to the selected sample while the graph stays open.
      selectBand(0, false);
      currentMode = FM;
      updateFrequency(freq, false);
    }
  }
}

void spectrumScannerRotate(int16_t delta)
{
  const uint16_t count = scanGetCount();
  if(!spectrumHasData || !count || !delta) return;

  int32_t next = (int32_t)selectedPoint + delta;
  while(next < 0) next += count;
  selectedPoint = (uint16_t)(next % count);

  uint16_t freq = 0;
  if(scanGetPoint(selectedPoint, &freq, nullptr, nullptr))
  {
    // Selection also tunes the receiver, without leaving the spectrum view.
    if(bandIdx != 0) selectBand(0, false);
    currentMode = FM;
    updateFrequency(freq, false);
  }
}

void spectrumScannerSelect()
{
  // A short encoder press accepts the highlighted frequency and returns to radio.
  if(spectrumHasData)
  {
    uint16_t freq = 0;
    if(scanGetPoint(selectedPoint, &freq, nullptr, nullptr))
    {
      if(bandIdx != 0) selectBand(0, false);
      currentMode = FM;
      updateFrequency(freq, false);
    }
  }
  currentCmd = CMD_NONE;
}

void drawSpectrumScanner()
{
  const uint16_t bg = 0x0208;
  const uint16_t panel = 0x030D;
  const uint16_t grid = 0x123A;
  const uint16_t cyan = 0x07FF;
  const uint16_t blue = 0x167F;
  const uint16_t green = 0x07E0;
  const uint16_t white = 0xFFFF;
  const uint16_t muted = 0x7192;

  spr.fillSprite(bg);
  spr.fillRoundRect(3, 3, 314, 164, 5, panel);
  spr.drawRoundRect(3, 3, 314, 164, 5, blue);

  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(cyan, panel);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString("VHF SPECTRUM", 160, 7);

  uint16_t selectedFreq = VHF_START;
  if(spectrumHasData)
    scanGetPoint(selectedPoint, &selectedFreq, nullptr, nullptr);

  char freqLabel[24];
  snprintf(freqLabel, sizeof(freqLabel), "%u.%02u MHz",
           (unsigned)(selectedFreq / 100), (unsigned)(selectedFreq % 100));
  spr.setTextColor(green, panel);
  spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);
  spr.drawString(freqLabel, 160, 25);

  const int gx = 12, gy = 48, gw = 296, gh = 91;
  spr.drawRect(gx, gy, gw, gh, blue);
  for(int i = 1; i < 5; ++i)
  {
    int y = gy + (gh * i) / 5;
    for(int x = gx + 2; x < gx + gw; x += 5) spr.drawPixel(x, y, grid);
  }
  for(int i = 1; i < 10; ++i)
  {
    int x = gx + (gw * i) / 10;
    for(int y = gy + 2; y < gy + gh; y += 5) spr.drawPixel(x, y, grid);
  }

  const uint16_t count = scanGetCount();
  if(spectrumHasData && count > 1)
  {
    uint8_t minR = 255, maxR = 0;
    for(uint16_t i = 0; i < count; ++i)
    {
      uint8_t r = 0;
      if(scanGetPoint(i, nullptr, &r, nullptr))
      {
        if(r < minR) minR = r;
        if(r > maxR) maxR = r;
      }
    }
    if(maxR <= minR) maxR = minR + 1;
    int lastX = gx, lastY = gy + gh - 2;
    for(uint16_t i = 0; i < count; ++i)
    {
      uint8_t r = 0;
      if(!scanGetPoint(i, nullptr, &r, nullptr)) continue;
      int x = gx + 1 + (int)((uint32_t)i * (gw - 3) / (count - 1));
      int y = gy + gh - 2 - (int)((uint32_t)(r - minR) * (gh - 5) / (maxR - minR));
      if(i) spr.drawLine(lastX, lastY, x, y, cyan);
      lastX = x;
      lastY = y;
    }
    int sx = gx + 1 + (int)((uint32_t)selectedPoint * (gw - 3) / (count - 1));
    spr.drawLine(sx, gy + 1, sx, gy + gh - 1, green);
    spr.fillTriangle(sx - 4, gy - 1, sx + 4, gy - 1, sx, gy + 5, green);
  }
  else
  {
    spr.setTextColor(muted, panel);
    spr.setFont(FONT_SMALL);
    spr.drawString("No scan data", 160, 83);
  }

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(white, panel);
  spr.setFont(FONT_DEFAULT);
  spr.drawString("88.00", gx, 143);
  spr.setTextDatum(TR_DATUM);
  spr.drawString("108.00 MHz", gx + gw, 143);

  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(muted, panel);
  spr.setFont(FONT_DEFAULT);
  char footer[64];
  snprintf(footer, sizeof(footer), "%u points  |  Turn: select  |  Click: tune & exit",
           (unsigned)count);
  spr.drawString(footer, 160, 162);
}
