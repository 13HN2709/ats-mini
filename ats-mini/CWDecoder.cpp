#include "CWDecoder.h"
#include <string.h>

CWDecoder::CWDecoder()
  : patternLength_(0),
    initialized_(false),
    tonePresent_(false),
    stateSinceMs_(0),
    lastToneEndMs_(0),
    ditMs_(100),
    textLength_(0)
{
  pattern_[0] = '\0';
  text_[0] = '\0';
}

void CWDecoder::reset()
{
  patternLength_ = 0;
  pattern_[0] = '\0';
  initialized_ = false;
  tonePresent_ = false;
  stateSinceMs_ = 0;
  lastToneEndMs_ = 0;
  textLength_ = 0;
  text_[0] = '\0';
}

void CWDecoder::setDitDurationMs(uint16_t durationMs)
{
  if (durationMs < 25) durationMs = 25;
  if (durationMs > 500) durationMs = 500;
  ditMs_ = durationMs;
}

uint16_t CWDecoder::ditDurationMs() const
{
  return ditMs_;
}

void CWDecoder::feedToneState(bool tonePresent, uint32_t nowMs)
{
  if (!initialized_) {
    stateSinceMs_ = nowMs;
    tonePresent_ = tonePresent;
    initialized_ = true;
    return;
  }

  if (tonePresent == tonePresent_) {
    tick(nowMs);
    return;
  }

  const uint32_t duration = nowMs - stateSinceMs_;
  if (tonePresent_) {
    closeMark(duration);
    lastToneEndMs_ = nowMs;
  } else {
    // The next tone edge may arrive before tick() was called often enough.
    // Account for the full preceding silence here as a fallback.
    if (duration >= (uint32_t)ditMs_ * 6U) {
      finishCharacter();
      finishWord();
    } else if (duration >= (uint32_t)ditMs_ * 2U) {
      finishCharacter();
    }
  }

  tonePresent_ = tonePresent;
  stateSinceMs_ = nowMs;
}

void CWDecoder::tick(uint32_t nowMs)
{
  if (!initialized_ || tonePresent_) return;

  const uint32_t silenceMs = nowMs - stateSinceMs_;
  if (silenceMs >= (uint32_t)ditMs_ * 6U) {
    finishCharacter();
    finishWord();
  } else if (silenceMs >= (uint32_t)ditMs_ * 2U) {
    finishCharacter();
  }
}

void CWDecoder::closeMark(uint32_t durationMs)
{
  if (patternLength_ >= sizeof(pattern_) - 1) return;
  pattern_[patternLength_++] = (durationMs < (uint32_t)ditMs_ * 2U) ? '.' : '-';
  pattern_[patternLength_] = '\0';
}

void CWDecoder::finishCharacter()
{
  if (!patternLength_) return;
  appendChar(decodePattern());
  patternLength_ = 0;
  pattern_[0] = '\0';
}

void CWDecoder::finishWord()
{
  if (!textLength_ || text_[textLength_ - 1] == ' ') return;
  appendChar(' ');
}

void CWDecoder::appendChar(char c)
{
  if (textLength_ >= TEXT_CAPACITY - 2) {
    memmove(text_, text_ + 1, TEXT_CAPACITY - 2);
    textLength_ = TEXT_CAPACITY - 3;
  }
  text_[textLength_++] = c;
  text_[textLength_] = '\0';
}

const char *CWDecoder::text() const
{
  return text_;
}

void CWDecoder::clearText()
{
  textLength_ = 0;
  text_[0] = '\0';
}

char CWDecoder::decodePattern() const
{
  // ITU Morse alphabet, numerals, and common punctuation.
  struct Entry { const char *code; char value; };
  static const Entry table[] = {
    {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'},
    {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'}, {"..", 'I'}, {".---", 'J'},
    {"-.-", 'K'}, {".-..", 'L'}, {"--", 'M'}, {"-.", 'N'}, {"---", 'O'},
    {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
    {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'}, {"-.--", 'Y'},
    {"--..", 'Z'},
    {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'},
    {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'},
    {"---..", '8'}, {"----.", '9'},
    {".-.-.-", '.'}, {"--..--", ','}, {"..--..", '?'}, {".----.", '\''},
    {"-.-.--", '!'}, {"-..-.", '/'}, {"-.--.", '('}, {"-.--.-", ')'},
    {".-...", '&'}, {"---...", ':'}, {"-.-.-.", ';'}, {"-...-", '='},
    {".-.-.", '+'}, {"-....-", '-'}, {"..--.-", '_'}, {".-..-.", '"'},
    {".--.-.", '@'}, {"...-..-", '$'}
  };

  for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
    if (strcmp(pattern_, table[i].code) == 0) return table[i].value;
  }
  return '?';
}
