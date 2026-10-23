#include "Common.h"
#include "Utils.h"
#include "Menu.h"

// Tuning delays after rx.setFrequency()
#define TUNE_DELAY_DEFAULT 30
#define TUNE_DELAY_FM      60
#define TUNE_DELAY_AM_SSB  80

#define SCAN_POLL_TIME    10 // Tuning status polling interval (msecs)
#define SCAN_POINTS      256 // Number of frequencies to scan

#define SCAN_OFF    0   // Scanner off, no data
#define SCAN_RUN    1   // Scanner running
#define SCAN_DONE   2   // Scanner done, valid data in scanData[]

struct ScanPoint
{
  uint8_t rssi;
  uint8_t snr;
};

static ScanPoint *scanData = nullptr;

static uint32_t scanTime = millis();
static uint8_t  scanStatus = SCAN_OFF;

static uint16_t scanStartFreq;
static uint16_t scanEndFreq;
static uint16_t scanStep;
static uint16_t scanCount;
static uint8_t  scanMinRSSI;
static uint8_t  scanMaxRSSI;
static uint8_t  scanMinSNR;
static uint8_t  scanMaxSNR;

static inline uint8_t min(uint8_t a, uint8_t b) { return(a<b? a:b); }
static inline uint8_t max(uint8_t a, uint8_t b) { return(a>b? a:b); }

// Read the latest completed sweep for the local web spectrum.
bool scanGetPoint(uint16_t index, uint16_t *freq, uint8_t *rssi, uint8_t *snr)
{
  if(scanStatus != SCAN_DONE || index >= scanCount || !scanData) return false;
  if(freq) *freq = scanStartFreq + scanStep * index;
  if(rssi) *rssi = scanData[index].rssi;
  if(snr) *snr = scanData[index].snr;
  return true;
}

uint16_t scanGetCount() { return scanStatus == SCAN_DONE ? scanCount : 0; }

float scanGetRSSI(uint16_t freq)
{
  // Input frequency must be in range of existing data
  if((scanStatus!=SCAN_DONE) || (freq<scanStartFreq) || (freq>=scanStartFreq+scanStep*scanCount))
    return(0.0);

  uint8_t result = scanData[(freq - scanStartFreq) / scanStep].rssi;
  return((result - scanMinRSSI) / (float)(scanMaxRSSI - scanMinRSSI + 1));
}

float scanGetSNR(uint16_t freq)
{
  // Input frequency must be in range of existing data
  if((scanStatus!=SCAN_DONE) || (freq<scanStartFreq) || (freq>=scanStartFreq+scanStep*scanCount))
    return(0.0);

  uint8_t result = scanData[(freq - scanStartFreq) / scanStep].snr;
  return((result - scanMinSNR) / (float)(scanMaxSNR - scanMinSNR + 1));
}

static void scanInitRange(uint16_t startFreq, uint16_t endFreq, uint16_t step)
{
  scanStep    = step;
  scanCount   = 0;
  scanMinRSSI = 255;
  scanMaxRSSI = 0;
  scanMinSNR  = 255;
  scanMaxSNR  = 0;
  scanStatus  = SCAN_RUN;
  scanTime    = millis();
  scanStartFreq = startFreq;
  scanEndFreq = endFreq;
  // Keep the requested start frequency exact and clear the retained sample buffer.
  memset(scanData, 0, SCAN_POINTS * sizeof(*scanData));
}

static bool scanTickTime()
{
  // Scan must be on
  if((scanStatus!=SCAN_RUN) || (scanCount>=SCAN_POINTS)) return(false);

  // Wait for the right time
  if(millis() - scanTime < SCAN_POLL_TIME) return(true);

  // This is our current frequency to scan
  uint16_t freq = scanStartFreq + scanStep * scanCount;

  // Poll for the tuning status
  rx.getStatus(0, 0);
  if(!rx.getTuneCompleteTriggered())
  {
    scanTime = millis();
    return(true);
  }

  // If frequency not yet set, set it and wait until next call to measure
  if(rx.getCurrentFrequency() != freq)
  {
    rx.setFrequency(freq); // Implies tuning delay
    scanTime = millis() - SCAN_POLL_TIME;
    return(true);
  }

  // Measure RSSI/SNR values
  rx.getCurrentReceivedSignalQuality();
  scanData[scanCount].rssi = rx.getCurrentRSSI();
  scanData[scanCount].snr  = rx.getCurrentSNR();

  // Measure range of values
  scanMinRSSI = min(scanData[scanCount].rssi, scanMinRSSI);
  scanMaxRSSI = max(scanData[scanCount].rssi, scanMaxRSSI);
  scanMinSNR  = min(scanData[scanCount].snr, scanMinSNR);
  scanMaxSNR  = max(scanData[scanCount].snr, scanMaxSNR);

  // Next frequency to scan
  freq += scanStep;

  // Set next frequency to scan or expire scan
  if((++scanCount >= SCAN_POINTS) || freq > scanEndFreq || consumeAbortPending())
    scanStatus = SCAN_DONE;
  else
    rx.setFrequency(freq); // Implies tuning delay

  // Save last scan time
  scanTime = millis() - SCAN_POLL_TIME;

  // Return current scan status
  return(scanStatus==SCAN_RUN);
}

//
// Run entire scan once
//
static void scanRunRangeInternal(uint16_t startFreq, uint16_t endFreq, uint16_t step)
{
  // Allocate once and retain the samples for drawing after the scan.
  if(!scanData)
  {
    scanData = static_cast<ScanPoint *>(ps_malloc(SCAN_POINTS * sizeof(*scanData)));
    if(!scanData) return;
  }

  if(!step || endFreq < startFreq) return;
  uint32_t points = ((uint32_t)endFreq - startFreq) / step + 1;
  if(points > SCAN_POINTS) endFreq = startFreq + step * (SCAN_POINTS - 1);

  rx.setMaxDelaySetFrequency(currentMode == FM ? TUNE_DELAY_FM : TUNE_DELAY_AM_SSB);
  muteOn(MUTE_TEMP, true);
  seekStop = false;
  uint16_t curFreq = rx.getFrequency();
  for(scanInitRange(startFreq, endFreq, step); scanTickTime(););
  rx.setFrequency(curFreq);
  muteOn(MUTE_TEMP, false);
  rx.setMaxDelaySetFrequency(TUNE_DELAY_DEFAULT);
}

void scanRun(uint16_t centerFreq, uint16_t step)
{
  const Band *band = getCurrentBand();
  if(!band || !step) return;
  int32_t start = (int32_t)step * ((int32_t)(centerFreq / step) - SCAN_POINTS / 2);
  int32_t end = start + (int32_t)step * (SCAN_POINTS - 1);
  if(start < band->minimumFreq) { start = band->minimumFreq; end = start + (int32_t)step * (SCAN_POINTS - 1); }
  if(end > band->maximumFreq) { end = band->maximumFreq; start = end - (int32_t)step * (SCAN_POINTS - 1); }
  if(start < band->minimumFreq) start = band->minimumFreq;
  scanRunRangeInternal((uint16_t)start, (uint16_t)end, step);
}

void scanRunRange(uint16_t startFreq, uint16_t endFreq, uint16_t step)
{
  // Custom web scans intentionally span the receiver's full supported range,
  // not just the currently selected preset band.
  if(!step || endFreq < startFreq) return;
  scanRunRangeInternal(startFreq, endFreq, step);
}
