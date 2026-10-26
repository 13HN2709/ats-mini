#ifndef RTTY_MODE_H
#define RTTY_MODE_H
#include <Arduino.h>
void rttyDecoderEnter(uint16_t decoderCommand);
void rttyDecoderExit();
void drawRTTYDecoderScreen();
bool rttyDecoderTick();
bool rttyDecoderActive();
#endif
