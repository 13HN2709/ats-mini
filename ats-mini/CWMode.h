#ifndef CW_MODE_H
#define CW_MODE_H

#include <Arduino.h>

void cwDecoderEnter();
void cwDecoderExit();
void drawCWDecoderScreen();
void cwDecoderTick();
bool cwDecoderActive();
bool cwDecoderWifiBlocked();

#endif
