#ifndef RTTY_DECODER_H
#define RTTY_DECODER_H
#include <Arduino.h>
class RTTYDecoder {
public:
  static constexpr size_t TEXT_CAPACITY = 384;
  RTTYDecoder();
  void reset();
  void feedBit(bool mark, uint32_t nowMs);
  const char *text() const;
  void clearText();
private:
  void decodeCode(uint8_t code);
  void appendChar(char c);
  uint8_t bits_[5];
  uint8_t bitCount_;
  uint8_t phase_;
  uint32_t lastBitMs_;
  bool haveBitTime_;
  bool letters_;
  char text_[TEXT_CAPACITY];
  size_t textLength_;
};
#endif
