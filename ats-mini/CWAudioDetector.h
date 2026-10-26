#ifndef CW_AUDIO_DETECTOR_H
#define CW_AUDIO_DETECTOR_H

#include <Arduino.h>

// Sample-rate-independent-in-structure Goertzel tone detector.
// Feed a fixed block of signed/unsigned ADC samples with a known sample rate.
// This class deliberately does not configure pins, Wi-Fi, or ADC hardware.
class CWAudioDetector {
public:
  CWAudioDetector();
  void configure(uint32_t sampleRateHz, uint16_t toneFrequencyHz,
                 uint16_t blockSize = 80, float threshold = 0.12f);
  bool processBlock(const int16_t *samples, size_t count);
  float lastTonePower() const;
  float lastSignalPower() const;
  bool toneDetected() const;

private:
  uint32_t sampleRateHz_;
  uint16_t toneFrequencyHz_;
  uint16_t blockSize_;
  float threshold_;
  float lastTonePower_;
  float lastSignalPower_;
  bool toneDetected_;
};

#endif
