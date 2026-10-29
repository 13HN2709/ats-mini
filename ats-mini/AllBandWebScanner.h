#ifndef ALL_BAND_WEB_SCANNER_H
#define ALL_BAND_WEB_SCANNER_H

#include <stdint.h>

bool allBandWebScanRun(uint16_t startKHz, uint16_t endKHz, uint16_t stepKHz, int8_t mode);
uint16_t allBandWebScanCount();
bool allBandWebScanPoint(uint16_t index, uint16_t *frequencyKHz, uint8_t *rssi, uint8_t *snr);
int8_t allBandWebScanMode();

#endif
