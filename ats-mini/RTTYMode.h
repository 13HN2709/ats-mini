#ifndef RTTY_MODE_H
#define RTTY_MODE_H
#include <Arduino.h>
void rttyDecoderEnter();
void rttyDecoderExit();
void drawRTTYDecoderScreen();
bool rttyDecoderTick();
bool rttyDecoderActive();
#endif
