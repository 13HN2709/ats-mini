#ifndef CW_DECODER_H
#define CW_DECODER_H

#include <Arduino.h>

// Standalone CW Morse decoder core. This class does not acquire audio and
// does not touch the radio, display, encoder, or existing layouts.
// Feed it a tone-present decision from the audio front-end at regular intervals.
class CWDecoder {
public:
  static constexpr size_t TEXT_CAPACITY = 384;

  CWDecoder();
  void reset();
  void setDitDurationMs(uint16_t durationMs);
  void feedToneState(bool tonePresent, uint32_t nowMs);
  const char *text() const;
  void clearText();
  uint16_t ditDurationMs() const;

private:
  void closeMark(uint32_t durationMs);
  void finishCharacter();
  void finishWord();
  void appendChar(char c);
  char decodePattern() const;

  char pattern_[8];
  uint8_t patternLength_;
  bool tonePresent_;
  uint32_t stateSinceMs_;
  uint32_t lastToneEndMs_;
  uint16_t ditMs_;
  char text_[TEXT_CAPACITY];
  size_t textLength_;
};

#endif // CW_DECODER_H
