#include "Common.h"
#include "Menu.h"
#include "Themes.h"
#include "Draw.h"
#include "Utils.h"
#include "AllBandSpectrum.h"
#include <stdlib.h>
#include <string.h>

static const uint16_t MIN_KHZ=100, MAX_KHZ=30000, MAX_WIDTH_KHZ=1000, STEP_KHZ=5, MAX_POINTS=201;
static const char *keys[16]={"1","2","3","BS","4","5","6","0","7","8","9",".","OK","MODE","","CLOSE"};
static char input[2][12]={{0},{0}};
static uint8_t stage=0,keyIndex=5,modeIndex=0;
static const uint8_t modes[3]={AM,USB,LSB};
static const char *modeNames[3]={"AM","USB","LSB"};
static bool inputError=false,hasData=false,saved=false;
static uint16_t startKHz=7100,endKHz=7600,count=0,selected=0;
static uint8_t rssi[MAX_POINTS],snr[MAX_POINTS];
static uint8_t oldBand=0,oldMode=AM,oldAllBandMode=AM;
static uint16_t oldFreq=0;
static int16_t oldBfo=0;

void allBandSpectrumStart(){input[0][0]=input[1][0]=0;stage=0;keyIndex=5;modeIndex=0;inputError=false;hasData=false;count=0;saved=false;currentCmd=CMD_ALL_SPECTRUM_INPUT;}
bool allBandSpectrumInputRotate(int16_t delta){if(!delta)return false;int d=(scrollDirection*delta>0)?1:-1;int n=(int)keyIndex+d;if(n<0)n=15;if(n>15)n=0;if(n==14){n+=d;if(n<0)n=15;if(n>15)n=0;}keyIndex=n;return true;}
void allBandSpectrumCancel(){inputError=false;if(saved){selectBand(oldBand,false);currentMode=oldMode;updateFrequency(oldFreq,false);if(isSSB())updateBFO(oldBfo,true);bands[1].bandMode=oldAllBandMode;saved=false;}currentCmd=CMD_MENU;}
static bool parseKHz(uint16_t *out){const char *s=input[stage];if(!*s)return false;char *e=nullptr;long v=strtol(s,&e,10);if(e==s||*e||v<MIN_KHZ||v>MAX_KHZ)return false;*out=(uint16_t)v;return true;}
static void restoreRadio(bool clearSaved=true){if(!saved)return;selectBand(oldBand,false);currentMode=oldMode;updateFrequency(oldFreq,false);if(isSSB())updateBFO(oldBfo,true);bands[1].bandMode=oldAllBandMode;if(clearSaved)saved=false;}
static void tuneAllBandPoint(uint16_t freq){if(bandIdx!=1||currentMode!=modes[modeIndex]){bands[1].bandMode=modes[modeIndex];selectBand(1,false);}currentBFO=0;updateFrequency(freq,false);}
static void runSweep(){
 count=0;rx.setMaxDelaySetFrequency(60);muteOn(MUTE_TEMP,true);const uint16_t restoreFreq=currentFrequency;
 const uint16_t points=(endKHz-startKHz)/STEP_KHZ+1;
 for(uint16_t i=0;i<points;i++){uint16_t f=startKHz+i*STEP_KHZ;rx.getStatus(1,0);rx.setFrequency(f);uint32_t t=millis();while(!rx.getTuneCompleteTriggered()&&millis()-t<350){delay(10);rx.getStatus(0,0);}rx.getCurrentReceivedSignalQuality(0);rssi[i]=rx.getCurrentRSSI();snr[i]=rx.getCurrentSNR();count=i+1;}
 rx.setFrequency(restoreFreq);muteOn(MUTE_TEMP,false);rx.setMaxDelaySetFrequency(30);hasData=count>0;selected=count/2;
}
bool allBandSpectrumInputClick(bool shortPress){if(!shortPress){allBandSpectrumCancel();return true;}const char*k=keys[keyIndex];char*v=input[stage];size_t n=strlen(v);inputError=false;
 if(!strcmp(k,"CLOSE")){allBandSpectrumCancel();return true;}if(!strcmp(k,"BS")){if(n)v[n-1]=0;return true;}
 if(!strcmp(k,"MODE")){modeIndex=(modeIndex+1)%3;return true;}
 if(!strcmp(k,"."))return true;
 if(!strcmp(k,"OK")){uint16_t f;if(!parseKHz(&f)){inputError=true;return true;}if(stage==0){startKHz=f;stage=1;input[1][0]=0;keyIndex=5;return true;}endKHz=f;if(endKHz<=startKHz||endKHz-startKHz>MAX_WIDTH_KHZ||(endKHz-startKHz)%STEP_KHZ!=0||((endKHz-startKHz)/STEP_KHZ+1)>MAX_POINTS){inputError=true;return true;}
 // ALL band is the broad AM-capable tuner configuration; configure it only after validation.
 oldBand=bandIdx;oldFreq=currentFrequency;oldMode=currentMode;oldBfo=currentBFO;oldAllBandMode=bands[1].bandMode;saved=true;
 bands[1].bandMode=modes[modeIndex];selectBand(1,false);currentBFO=0;updateFrequency(startKHz,false);currentCmd=CMD_ALL_SPECTRUM;runSweep();restoreRadio(false);return true;}
 if(n<sizeof(input[0])-1&&k[0]>='0'&&k[0]<='9'){v[n]=k[0];v[n+1]=0;}return true;}
void allBandSpectrumRotate(int16_t delta){
 if(!hasData||count<3||!delta)return;
 uint8_t mn=255,mx=0;for(uint16_t i=0;i<count;i++){if(rssi[i]<mn)mn=rssi[i];if(rssi[i]>mx)mx=rssi[i];}
 uint8_t threshold=(uint8_t)(mn+((uint16_t)(mx-mn)*5)/100);int dir=delta>0?1:-1;int candidate=selected;
 for(uint16_t n=0;n<count;n++){candidate+=dir;if(candidate<0)candidate=count-1;if(candidate>=count)candidate=0;if(candidate==0||candidate==count-1)continue;
   if(rssi[candidate]>=threshold&&rssi[candidate]>=rssi[candidate-1]&&rssi[candidate]>rssi[candidate+1]){selected=candidate;tuneAllBandPoint(startKHz+selected*STEP_KHZ);return;}}
 int best=-1;uint8_t bestR=0;candidate=selected;
 for(uint16_t n=0;n<count-1;n++){candidate+=dir;if(candidate<0)candidate=count-1;if(candidate>=count)candidate=0;if(candidate==selected)break;if(candidate==0||candidate==count-1)continue;if(best<0||rssi[candidate]>bestR){best=candidate;bestR=rssi[candidate];}}
 if(best>=0){selected=best;tuneAllBandPoint(startKHz+selected*STEP_KHZ);}
}
void allBandSpectrumSelect(){if(hasData){tuneAllBandPoint(startKHz+selected*STEP_KHZ);saved=false;}else restoreRadio();currentCmd=CMD_NONE;}
void drawAllBandSpectrumInput(){const uint16_t cyan=0x07FF,blue=0x001F,dark=0x0841,red=0xF800;spr.fillSprite(0);spr.fillRoundRect(6,2,308,166,4,0);spr.drawRoundRect(6,2,308,166,4,blue);spr.drawRoundRect(7,3,306,164,3,blue);
 spr.setTextDatum(TC_DATUM);spr.setTextColor(cyan);spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);spr.drawString(stage==0?"ALL BAND: START (kHz)":"ALL BAND: END (kHz)",160,5);
 char b[32];snprintf(b,sizeof(b),"%s kHz",input[stage][0]?input[stage]:"---");spr.setTextDatum(MC_DATUM);spr.setTextColor(inputError?red:cyan);spr.drawRoundRect(14,28,210,23,2,inputError?red:blue);spr.drawString(b,119,39);
 spr.fillRoundRect(230,28,76,23,2,dark);spr.drawRoundRect(230,28,76,23,2,blue);spr.setTextColor(cyan);spr.drawString(modeNames[modeIndex],268,39);
 const int sx=14,sy=55,w=68,h=27,gx=5,gy=2;
 for(int i=0;i<16;i++){if(i==14)continue;int x=sx+(i%4)*(w+gx),y=sy+(i/4)*(h+gy);bool sel=i==keyIndex;spr.fillRoundRect(x,y,w,h,3,sel?cyan:dark);spr.drawRoundRect(x,y,w,h,3,sel?0xFFFF:blue);spr.setTextDatum(MC_DATUM);spr.setTextColor(sel?0:cyan);spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);spr.drawString(keys[i],x+w/2,y+h/2);}
 spr.setTextDatum(TC_DATUM);spr.setTextColor(inputError?red:0xBDF7);spr.setFont(FONT_SMALL);spr.drawString(inputError?"Invalid Input / Scan Width > 1000 kHz":"MODE cycles AM / USB / LSB",160,158); }
void drawAllBandSpectrum(){const uint16_t cyan=0x07FF,blue=0x001F,green=0x07E0;spr.fillSprite(0);spr.fillRoundRect(3,3,314,164,5,0x0008);spr.drawRoundRect(3,3,314,164,5,blue);spr.setTextDatum(TC_DATUM);spr.setTextColor(cyan);spr.setFont(&lgfx::fonts::FreeSansBold9pt7b);spr.drawString("ALL BAND SPECTRUM",160,7);char b[32];snprintf(b,sizeof(b),"%u kHz  %s",startKHz+selected*STEP_KHZ,modeNames[modeIndex]);spr.setTextColor(green);spr.drawString(b,160,25);
 const int x0=12,y0=48,w=296,h=91;spr.drawRect(x0,y0,w,h,blue);if(hasData&&count>1){uint8_t mn=255,mx=0;for(int i=0;i<count;i++){if(rssi[i]<mn)mn=rssi[i];if(rssi[i]>mx)mx=rssi[i];}if(mx<=mn)mx=mn+1;int px=x0,py=y0+h-2;for(int i=0;i<count;i++){int x=x0+1+(long)i*(w-3)/(count-1);int y=y0+h-2-(long)(rssi[i]-mn)*(h-4)/(mx-mn);spr.drawLine(px,py,x,y,cyan);if(i==selected)spr.fillCircle(x,y,3,0xF800);px=x;py=y;}}
 spr.setTextColor(cyan);spr.setFont(FONT_SMALL);spr.setTextDatum(TL_DATUM);spr.drawString("Rotate: peak/frequency  Click: accept",12,147);spr.drawString("Max scan width: 1000 kHz",12,158);}
