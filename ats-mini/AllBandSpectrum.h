#ifndef ALL_BAND_SPECTRUM_H
#define ALL_BAND_SPECTRUM_H
#include <stdint.h>
void allBandSpectrumStart();
void allBandSpectrumCancel();
bool allBandSpectrumInputRotate(int16_t delta);
bool allBandSpectrumInputClick(bool shortPress);
void allBandSpectrumRotate(int16_t delta);
void allBandSpectrumSelect();
void drawAllBandSpectrumInput();
void drawAllBandSpectrum();
#endif
