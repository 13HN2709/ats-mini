#ifndef SPECTRUM_SCANNER_H
#define SPECTRUM_SCANNER_H
#include <stdint.h>
void spectrumScannerStart();
void spectrumScannerRotate(int16_t delta);
void spectrumScannerSelect();
bool spectrumScannerInputRotate(int16_t delta);
bool spectrumScannerInputClick(bool shortPress);
void spectrumScannerInputCancel();
void drawSpectrumScannerInput();
void drawSpectrumScanner();
#endif
