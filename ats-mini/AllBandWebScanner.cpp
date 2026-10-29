#include "Common.h"
#include "Utils.h"
#include "Menu.h"
#include "AllBandWebScanner.h"

// Independent web ALL Band result storage. Do not reuse Scan.cpp's VHF/FM data.
static const uint16_t WEB_ALL_MAX_POINTS = 201;
struct WebAllScanPoint { uint8_t rssi; uint8_t snr; };
static WebAllScanPoint webAllData[WEB_ALL_MAX_POINTS];
static uint16_t webAllCount = 0;
static uint16_t webAllStart = 0;
static uint16_t webAllStep = 0;
static int8_t webAllMode = AM;

static void configureWebAllMode(int8_t mode, uint16_t frequency)
{
  currentMode = mode;
  currentBFO = 0;
  if(mode == AM)
    rx.setAM(150, 30000, constrain(frequency, 150, 30000), 1);
  else
  {
    rx.setSSB(150, 30000, constrain(frequency, 150, 30000), 0, mode);
    rx.setSSBConfig(getCurrentBandwidth()->idx, 1, 0, 1, 0, 1);
    Band *activeBand = getCurrentBand();
    if(activeBand)
    {
      if(mode == USB) rx.setSSBBfo(-(currentBFO + activeBand->usbCal));
      else rx.setSSBBfo(-(currentBFO + activeBand->lsbCal));
    }
  }
  currentFrequency = frequency;
  Band *activeBand = getCurrentBand();
  if(activeBand) activeBand->currentFreq = frequency;
}

static void restoreWebAllRadio(uint8_t savedBand, int8_t savedMode, uint16_t savedFrequency,
                               int16_t savedBfo, int8_t savedAllBandMode)
{
  bands[1].bandMode = savedAllBandMode;
  if(bandIdx != savedBand) selectBand(savedBand, false);
  currentMode = savedMode;
  currentBFO = savedBfo;
  if(savedMode == FM)
    rx.setFM(6400, 10800, constrain(savedFrequency, 6400, 10800), 10);
  else if(savedMode == AM)
    rx.setAM(150, 30000, constrain(savedFrequency, 150, 30000), 1);
  else
  {
    rx.setSSB(150, 30000, constrain(savedFrequency, 150, 30000), 0, savedMode);
    rx.setSSBConfig(getCurrentBandwidth()->idx, 1, 0, 1, 0, 1);
    Band *activeBand = getCurrentBand();
    if(activeBand)
    {
      if(savedMode == USB) rx.setSSBBfo(-(currentBFO + activeBand->usbCal));
      else rx.setSSBBfo(-(currentBFO + activeBand->lsbCal));
    }
  }
  currentFrequency = savedFrequency;
  Band *activeBand = getCurrentBand();
  if(activeBand) activeBand->currentFreq = savedFrequency;
  clearStationInfo();
  identifyFrequency(currentFrequency + currentBFO / 1000);
}

bool allBandWebScanRun(uint16_t startKHz, uint16_t endKHz, uint16_t stepKHz, int8_t mode)
{
  webAllCount = 0;
  if((mode != AM && mode != USB && mode != LSB) || !stepKHz || endKHz < startKHz)
    return false;
  uint32_t points = ((uint32_t)endKHz - startKHz) / stepKHz + 1;
  if(points < 2 || points > WEB_ALL_MAX_POINTS || startKHz < 150 || endKHz > 30000)
    return false;

  const uint8_t savedBand = bandIdx;
  const int8_t savedMode = currentMode;
  const uint16_t savedFrequency = currentFrequency;
  const int16_t savedBfo = currentBFO;
  const int8_t savedAllBandMode = bands[1].bandMode;
  const uint16_t oldDelay = 30;

  // Use the receiver's ALL band only for this scan; retain the previous band
  // and modulation and restore them on every exit path.
  bands[1].bandMode = mode;
  if(bandIdx != 1) selectBand(1, false);
  configureWebAllMode(mode, startKHz);

  rx.setMaxDelaySetFrequency(60);
  muteOn(MUTE_TEMP, true);
  seekStop = false;
  webAllStart = startKHz;
  webAllStep = stepKHz;
  webAllMode = mode;

  bool aborted = false;
  for(uint16_t i = 0; i < points; i++)
  {
    const uint16_t frequency = startKHz + i * stepKHz;
    rx.getStatus(1, 0);
    rx.setFrequency(frequency);
    const uint32_t tuneStarted = millis();
    while(true)
    {
      delay(10);
      rx.getStatus(0, 0);
      if(rx.getTuneCompleteTriggered() || millis() - tuneStarted >= 350) break;
      if(consumeAbortPending()) { aborted = true; break; }
    }
    if(aborted) break;

    rx.getCurrentReceivedSignalQuality(0);
    webAllData[i].rssi = rx.getCurrentRSSI();
    webAllData[i].snr = rx.getCurrentSNR();
    webAllCount = i + 1;
    if(i + 1 < points && consumeAbortPending()) { aborted = true; break; }
  }

  // Restore receiver configuration, frequency, band, BFO, and audio state.
  restoreWebAllRadio(savedBand, savedMode, savedFrequency, savedBfo, savedAllBandMode);
  muteOn(MUTE_TEMP, false);
  rx.setMaxDelaySetFrequency(oldDelay);
  return webAllCount > 0;
}

uint16_t allBandWebScanCount() { return webAllCount; }
int8_t allBandWebScanMode() { return webAllMode; }

bool allBandWebScanPoint(uint16_t index, uint16_t *frequencyKHz, uint8_t *rssi, uint8_t *snr)
{
  if(index >= webAllCount) return false;
  if(frequencyKHz) *frequencyKHz = webAllStart + webAllStep * index;
  if(rssi) *rssi = webAllData[index].rssi;
  if(snr) *snr = webAllData[index].snr;
  return true;
}
