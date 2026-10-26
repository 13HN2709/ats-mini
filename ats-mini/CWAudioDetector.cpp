#include "CWAudioDetector.h"
#include <math.h>

CWAudioDetector::CWAudioDetector()
  : sampleRateHz_(4000),
    toneFrequencyHz_(700),
    blockSize_(80),
    threshold_(0.12f),
    lastTonePower_(0.0f),
    lastSignalPower_(0.0f),
    toneDetected_(false)
{
}

void CWAudioDetector::configure(uint32_t sampleRateHz, uint16_t toneFrequencyHz,
                                uint16_t blockSize, float threshold)
{
  sampleRateHz_ = sampleRateHz ? sampleRateHz : 4000;
  toneFrequencyHz_ = toneFrequencyHz;
  blockSize_ = blockSize ? blockSize : 80;
  threshold_ = threshold < 0.01f ? 0.01f : (threshold > 0.95f ? 0.95f : threshold);
}

bool CWAudioDetector::processBlock(const int16_t *samples, size_t count)
{
  if (!samples || count < 8 || !sampleRateHz_ ||
      toneFrequencyHz_ == 0 || toneFrequencyHz_ >= sampleRateHz_ / 2) {
    toneDetected_ = false;
    lastTonePower_ = lastSignalPower_ = 0.0f;
    return false;
  }

  // Remove DC bias before calculating the target-frequency energy.
  double mean = 0.0;
  for (size_t i = 0; i < count; ++i) mean += samples[i];
  mean /= (double)count;

  const double omega = 2.0 * PI * (double)toneFrequencyHz_ / (double)sampleRateHz_;
  const double coeff = 2.0 * cos(omega);
  double q0, q1 = 0.0, q2 = 0.0;
  double total = 0.0;

  for (size_t i = 0; i < count; ++i) {
    const double value = (double)samples[i] - mean;
    total += value * value;
    q0 = value + coeff * q1 - q2;
    q2 = q1;
    q1 = q0;
  }

  const double real = q1 - q2 * cos(omega);
  const double imag = q2 * sin(omega);
  const double targetEnergy = real * real + imag * imag;
  const double signalEnergy = total * (double)count;

  lastSignalPower_ = total > 0.0 ? (float)(total / (double)count) : 0.0f;
  lastTonePower_ = signalEnergy > 0.0 ? (float)(targetEnergy / signalEnergy) : 0.0f;

  // Goertzel energy ratio approaches 1 for a clean in-band tone. A relative
  // threshold helps accommodate different ADC gain and audio volume levels.
  toneDetected_ = lastTonePower_ >= threshold_;
  return toneDetected_;
}

float CWAudioDetector::lastTonePower() const { return lastTonePower_; }
float CWAudioDetector::lastSignalPower() const { return lastSignalPower_; }
bool CWAudioDetector::toneDetected() const { return toneDetected_; }
