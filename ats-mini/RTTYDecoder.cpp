#include "RTTYDecoder.h"
#include <string.h>
RTTYDecoder::RTTYDecoder() { reset(); }
void RTTYDecoder::reset() {
  memset(bits_, 0, sizeof(bits_)); bitCount_=0; phase_=0; lastBitMs_=0;
  haveBitTime_=false; letters_=true; textLength_=0; text_[0]='\0';
}
void RTTYDecoder::clearText() { textLength_=0; text_[0]='\0'; }
const char *RTTYDecoder::text() const { return text_; }
void RTTYDecoder::appendChar(char c) {
  if(textLength_ >= TEXT_CAPACITY-2) { memmove(text_,text_+1,TEXT_CAPACITY-2); textLength_=TEXT_CAPACITY-3; }
  text_[textLength_++]=c; text_[textLength_]='\0';
}
// ITA2/Baudot: start=space, five data bits LSB-first, stop=mark.
// Call at each detected bit centre from the FSK demodulator.
void RTTYDecoder::feedBit(bool mark, uint32_t nowMs) {
  lastBitMs_=nowMs; haveBitTime_=true;
  if(phase_==0) { if(!mark) { phase_=1; bitCount_=0; } return; }
  if(phase_>=1 && phase_<=5) { bits_[bitCount_++]=mark?1:0; ++phase_; return; }
  if(phase_==6) {
    if(mark && bitCount_==5) {
      uint8_t code=0; for(uint8_t i=0;i<5;i++) if(bits_[i]) code |= (1U<<i);
      decodeCode(code);
    }
    phase_=0; bitCount_=0;
  }
}
void RTTYDecoder::decodeCode(uint8_t c) {
  if(c==31) { letters_=true; return; }
  if(c==27) { letters_=false; return; }
  static const char letters[32] = { '\0','E','\n','A',' ','S','I','U','\r','D','R','J','N','F','C','K','T','Z','L','W','H','Y','P','Q','O','B','G','\0','M','X','V','\0' };
  static const char figures[32] = { '\0','3','\n','-',' ','\'','8','7','\r','
  char out=letters_?letters[c]:figures[c];
  if(out && out!='\7') appendChar(out);
}
,'4','\7',',','!',':','(','5','"',')','2','#','6','0','1','9','?','&','\0','.','/',';','\0' };
  char out=letters_?letters[c]:figures[c];
  if(out && out!='\7') appendChar(out);
}
