#ifndef SPECTRUM_SCANNER_H
#define SPECTRUM_SCANNER_H

#include <stdint.h>

void spectrumScannerStart();
void spectrumScannerRotate(int16_t delta);
void spectrumScannerSelect();
void drawSpectrumScanner();

#endif
