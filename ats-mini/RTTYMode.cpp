#include "Common.h"
#include "Menu.h"
#include "Draw.h"
#include "RTTYMode.h"
#include "RTTYDecoder.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static RTTYDecoder rtty;
static bool active=false;
static uint32_t lastUi=0;
static bool hasSignal=false;
static uint32_t lastSignal=0;
static constexpr uint8_t AUDIO_PIN=11;
static constexpr uint32_t SAMPLE_RATE=4000;
static uint16_t decoderCommand=CMD_RTTY_DECODER;
static constexpr uint16_t BLOCK=80;
static int16_t samples[BLOCK];
static uint32_t lastBlock=0;
static uint32_t bitStart=0;
static uint16_t bitSamples=0;
static int markScore=0, spaceScore=0;
static bool markBit=true;
extern uint8_t wifiModeIdx;
#ifndef NET_OFF
#define NET_OFF 0
#endif
bool rttyDecoderActive(){return active;}
void rttyDecoderEnter(uint16_t command){rtty.reset(); decoderCommand=command; active=(wifiModeIdx==NET_OFF); currentCmd=command; pinMode(AUDIO_PIN,INPUT); lastUi=lastBlock=millis(); bitStart=millis(); bitSamples=0; markScore=spaceScore=0; hasSignal=false; lastSignal=0;}
void rttyDecoderExit(){active=false; currentCmd=CMD_NONE;}
bool rttyDecoderTick(){
 if(!active) return false;
 uint32_t now=millis();
 if(now-lastBlock>=20){
  uint32_t us=micros(); double mean=0;
  for(uint16_t i=0;i<BLOCK;i++){samples[i]=(int16_t)analogRead(AUDIO_PIN); mean+=samples[i]; uint32_t target=us+((uint32_t)(i+1)*1000000UL/SAMPLE_RATE); while((int32_t)(micros()-target)<0){}}
  mean/=BLOCK; double eMark=0,eSpace=0;
  // Goertzel-like single-frequency correlations, tuned for common 170 Hz-shift RTTY audio.
  double m1=0,m2=0,s1=0,s2=0; const double markHz=decoderCommand==CMD_RTTY50_DECODER?1275.0:1275.0;
  const double spaceHz=decoderCommand==CMD_RTTY50_DECODER?1725.0:1445.0;
  const double cm=2.0*cos(2.0*PI*markHz/SAMPLE_RATE), cs=2.0*cos(2.0*PI*spaceHz/SAMPLE_RATE);
  for(uint16_t i=0;i<BLOCK;i++){double v=samples[i]-mean; double q=v+cm*m1-m2; m2=m1;m1=q; q=v+cs*s1-s2;s2=s1;s1=q;}
  eMark=m1*m1+m2*m2-cm*m1*m2; eSpace=s1*s1+s2*s2-cs*s1*s2;
  bool mark=eMark>=eSpace; hasSignal=(eMark+eSpace)>100000.0; if(hasSignal)lastSignal=now;
  // Nominal 45.45 baud. Sample at each bit centre; timing recovery is deliberately simple for first hardware test.
  if(bitSamples==0){bitStart=now;markBit=mark;}
  ++bitSamples;
  const uint32_t bitPeriod=decoderCommand==CMD_RTTY50_DECODER?20:22;
  if(now-bitStart>=bitPeriod){rtty.feedBit(markBit,now);bitStart=now;bitSamples=0;}
  lastBlock=now;
 }
 if(now-lastUi>=100){lastUi=now;return true;} return false;
}
void drawRTTYDecoderScreen(){
 const uint16_t navy=0x0008, blueA=0x0359, blueB=0x047D, green=0x07E0, red=0xF800;
 spr.fillSprite(navy);
 spr.fillRect(8,5,304,31,blueA);
 spr.drawRect(8,5,304,31,red);

 // Keep frequency, decoder mode and signal status in separate zones.
 char freq[32];
 if(currentMode==FM) snprintf(freq,sizeof(freq),"%.2f MHz",currentFrequency/100.0f);
 else snprintf(freq,sizeof(freq),"%u kHz",currentFrequency);
 spr.setTextDatum(ML_DATUM);
 spr.setTextColor(0xFFFF);
 spr.drawString(freq,16,13,FONT_SMALL);

 spr.setTextDatum(MC_DATUM);
 spr.setTextColor(green);
 spr.drawString(decoderCommand==CMD_RTTY50_DECODER?"RTTY50 / 50 BAUD":"RTTY / 45.45 BAUD",174,13,FONT_SMALL);

 spr.setTextDatum(MR_DATUM);
 uint16_t led=(lastSignal&&millis()-lastSignal<500)?green:red;
 spr.setTextColor(led);
 spr.drawString((lastSignal&&millis()-lastSignal<500)?"SIGNAL":"NO SIG",302,13,FONT_SMALL);
 const int py[]={40,69,98,127}; const int chars=34; const char *txt=rtty.text(); size_t len=strlen(txt);
 for(int row=0;row<4;row++){spr.fillRect(8,py[row],304,25,(row&1)?blueB:blueA);spr.drawRect(8,py[row],304,25,0x04BF);size_t visible=len<(size_t)(4*chars)?len:(size_t)(4*chars); size_t start=len-visible+(size_t)(row*chars); size_t n=visible>(size_t)(row*chars)?visible-(size_t)(row*chars):0;if(n>(size_t)chars)n=chars;if(n){char line[chars+1];memcpy(line,txt+start,n);line[n]=0;spr.setTextDatum(ML_DATUM);spr.setTextColor(0xFFFF);spr.setFont(&fonts::Font2); spr.drawString(line,14,py[row]+4);}}
 if((millis()/350)&1) spr.fillCircle(296,27,3,led); else spr.drawCircle(296,27,3,led);spr.setTextDatum(MC_DATUM);spr.setTextColor(0xBDF7);spr.drawString("CLICK: EXIT  |  ROTATE: TUNE/BFO",160,163,FONT_SMALL);
}
