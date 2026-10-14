#ifndef FREQ_KEYPAD_H
#define FREQ_KEYPAD_H

#include <Arduino.h>

void freqKeypadInit();
void freqKeypadReset();
bool doFreqKeypad(int16_t enc);
bool clickFreqKeypad(bool shortPress);
void drawFreqKeypad();

#endif

