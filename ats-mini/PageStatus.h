#ifndef PAGE_STATUS_H
#define PAGE_STATUS_H

#include <Arduino.h>

static const char pageStatusTitle[] PROGMEM = "ATS-Mini Status";

static const char pageStatus[] PROGMEM = R"HTML(<DIV CLASS="TOP">
  <DIV CLASS="BRAND"><DIV CLASS="LOGO">&#9673;</DIV><DIV><H1>ATS MINI <SPAN STYLE="color:#80a0ff">CONTROLLER</SPAN></H1><DIV CLASS="SUB">Local radio control center</DIV></DIV></DIV>
  <SPAN CLASS="PILL">&#9679; {{connection}}</SPAN>
</DIV>
{{{navigation}}}
<SECTION CLASS="HERO">
  <DIV CLASS="EYEBROW">Now receiving</DIV>
  <DIV CLASS="FREQ LED-FREQ">{{{led_frequency}}}</DIV>
  <DIV CLASS="MODE">{{band}} &nbsp;·&nbsp; {{mode}} {{{stereo}}}</DIV>
</SECTION>
<SECTION CLASS="STAT" STYLE="margin-top:14px">
  <DIV CLASS="EYEBROW">Radio controls</DIV>
  <FORM ID="radioTune" STYLE="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;margin-top:10px">
    <LABEL FOR="radioFrequency" CLASS="LABEL" STYLE="grid-column:1/-1;display:flex;align-items:center;justify-content:space-between;gap:10px">Frequency
      <SELECT ID="radioFrequencyUnit" aria-label="Frequency unit" STYLE="width:auto;min-width:86px"><OPTION VALUE="hz">Hz</OPTION><OPTION VALUE="khz">kHz</OPTION><OPTION VALUE="mhz">MHz</OPTION></SELECT>
    </LABEL>
    <INPUT ID="radioFrequency" NAME="freq" TYPE="number" INPUTMODE="decimal" MIN="0" MAX="300000000" STEP="{{frequency_step_hz}}" VALUE="{{frequency_hz}}" STYLE="width:100%;min-width:0" REQUIRED>
    <BUTTON TYPE="submit">TUNE</BUTTON>
  </FORM>
  <DIV ID="radioControlStatus" CLASS="LABEL" ROLE="status" STYLE="margin-top:8px">Enter a frequency supported by the current band.</DIV>
  <DIV STYLE="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:10px">
    <BUTTON ID="radioStepDown" TYPE="button" aria-label="Tune down one step">− STEP</BUTTON>
    <BUTTON ID="radioStepUp" TYPE="button" aria-label="Tune up one step">+ STEP</BUTTON>
  </DIV>
  <DIV STYLE="margin-top:16px">
    <LABEL FOR="radioVolume" CLASS="LABEL" STYLE="display:flex;justify-content:space-between;gap:10px"><SPAN>Volume</SPAN><STRONG ID="radioVolumeValue" STYLE="color:#80a0ff;font-family:monospace;font-weight:900;font-variant-numeric:tabular-nums">{{volume}}</STRONG></LABEL>
    <INPUT ID="radioVolume" TYPE="range" MIN="0" MAX="63" STEP="1" VALUE="{{volume}}" STYLE="width:100%;accent-color:#80a0ff;margin-top:10px">
    <DIV ID="radioVolumeStatus" CLASS="LABEL" ROLE="status" STYLE="margin-top:6px">Adjust volume · 0–63</DIV>
  </DIV>
</SECTION>
<SECTION CLASS="SPECTRUM-PANEL">
  <DIV CLASS="PANEL-HEAD">
    <DIV><DIV CLASS="EYEBROW">01 / RF ANALYSIS</DIV><H2>SPECTRUM SCAN</H2><DIV CLASS="SUB">All-band receiver scan · selectable modulation</DIV></DIV>
    <DIV CLASS="LIVE-MARK"><SPAN></SPAN> RECEIVER</DIV>
  </DIV>
  <FORM ID="spectrumRange" CLASS="RANGE-FORM">
    <LABEL>START <SPAN ID="spectrumUnitStart">MHz</SPAN><INPUT ID="spectrumStartMHz" TYPE="number" INPUTMODE="decimal" MIN="64" MAX="108" STEP="0.1" VALUE="88.00" REQUIRED></LABEL>
    <LABEL>STOP <SPAN ID="spectrumUnitEnd">MHz</SPAN><INPUT ID="spectrumEndMHz" TYPE="number" INPUTMODE="decimal" MIN="64" MAX="108" STEP="0.1" VALUE="108.00" REQUIRED></LABEL>
    <LABEL STYLE="grid-column:1/-1">MODULATION<DIV ID="spectrumModes" STYLE="display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:5px;margin-top:6px"><BUTTON TYPE="button" CLASS="MOD-BUTTON ACTIVE" DATA-MODE="FM">FM</BUTTON><BUTTON TYPE="button" CLASS="MOD-BUTTON" DATA-MODE="AM">AM</BUTTON><BUTTON TYPE="button" CLASS="MOD-BUTTON" DATA-MODE="USB">USB</BUTTON><BUTTON TYPE="button" CLASS="MOD-BUTTON" DATA-MODE="LSB">LSB</BUTTON></DIV></LABEL>
    <LABEL>STEP<SELECT ID="spectrumStep" STYLE="width:100%;margin-top:6px"><OPTION VALUE="100000" SELECTED>100 kHz</OPTION><OPTION VALUE="1000">1 kHz</OPTION><OPTION VALUE="5000">5 kHz</OPTION><OPTION VALUE="10000">10 kHz</OPTION><OPTION VALUE="25000">25 kHz</OPTION></SELECT></LABEL>
    <BUTTON ID="spectrumStart" TYPE="submit">▶ SCAN RANGE</BUTTON>
  </FORM>
  <DIV CLASS="SPECTRUM-FRAME">
    <DIV CLASS="SPECTRUM-META"><SPAN>FIELD STRENGTH / RSSI</SPAN><SPAN>PEAKS ARE SELECTABLE</SPAN></DIV>
    <CANVAS ID="spectrumCanvas" HEIGHT="230" STYLE="display:block;width:100%;height:230px;cursor:crosshair" aria-label="Clickable all-band spectrum plot"></CANVAS>
  </DIV>
  <DIV ID="spectrumStatus" CLASS="SCAN-STATUS" ROLE="status">Choose a modulation and frequency range, then scan. Tap a red peak to tune the receiver.</DIV>
</SECTION>
<DIV CLASS="GRID">
  <DIV CLASS="STAT"><SPAN CLASS="LABEL">Signal strength</SPAN><SPAN CLASS="VALUE">{{rssi}} <SMALL STYLE="color:#8ea2ae;font-size:.72rem">dBµV</SMALL></SPAN></DIV>
  <DIV CLASS="STAT"><SPAN CLASS="LABEL">Signal / noise</SPAN><SPAN CLASS="VALUE">{{snr}} <SMALL STYLE="color:#8ea2ae;font-size:.72rem">dB</SMALL></SPAN></DIV>
  <DIV CLASS="STAT"><SPAN CLASS="LABEL">Battery</SPAN><SPAN CLASS="VALUE">{{battery}} V</SPAN></DIV>
  <DIV CLASS="STAT"><SPAN CLASS="LABEL">Receiver clock</SPAN><SPAN CLASS="VALUE">{{time}}</SPAN></DIV>
</DIV>
<DIV CLASS="SECTION">Radio tools</DIV>
<DIV CLASS="ACTIONS">
  <A CLASS="ACTION" HREF="/memory"><SPAN CLASS="ICON">&#9776;</SPAN><SPAN><STRONG>Memories</STRONG><SMALL>Manage and organize slots</SMALL></SPAN></A>
  <A CLASS="ACTION" HREF="/config"><SPAN CLASS="ICON">&#9881;</SPAN><SPAN><STRONG>Configuration</STRONG><SMALL>Network and display settings</SMALL></SPAN></A>
  <A CLASS="ACTION" HREF="/update"><SPAN CLASS="ICON">&#8593;</SPAN><SPAN><STRONG>Firmware</STRONG><SMALL>Update and status</SMALL></SPAN></A>
  <A CLASS="ACTION" HREF="/patches"><SPAN CLASS="ICON">&#9638;</SPAN><SPAN><STRONG>Patch sets</STRONG><SMALL>Manage radio patches</SMALL></SPAN></A>
</DIV>
<DIV CLASS="SECTION">Connection</DIV>
<DIV CLASS="STAT">
  <SPAN CLASS="LABEL">Network</SPAN><SPAN CLASS="VALUE">{{ssid}}</SPAN>
  <SPAN CLASS="LABEL" STYLE="margin-top:10px">Open controller</SPAN><SPAN CLASS="VALUE"><A HREF="http://{{ip}}">{{ip}}</A></SPAN>
  <SPAN CLASS="LABEL" STYLE="margin-top:10px">MAC address</SPAN><SPAN CLASS="VALUE">{{mac}}</SPAN>
  <SPAN CLASS="LABEL" STYLE="margin-top:10px">Firmware version</SPAN><SPAN CLASS="VALUE">{{version}}</SPAN>
</DIV>
<P CLASS="SUB" STYLE="text-align:center;margin:16px 0 0">Offline-ready interface · Served directly by your ATS Mini</P>
<SCRIPT>
(()=>{
 const tuneForm=document.getElementById('radioTune');
 const tuneInput=document.getElementById('radioFrequency');
 const tuneUnit=document.getElementById('radioFrequencyUnit');
 const controlStatus=document.getElementById('radioControlStatus');
 const frequencyStepHz=Number(tuneInput.step)||1000;
 const unitScale=unit=>unit==='mhz'?1000000:unit==='khz'?1000:1;
 let activeFrequencyUnit=tuneUnit.value;
 function inputFrequencyHz(){return Number(tuneInput.value)*unitScale(activeFrequencyUnit);}
 function setFrequencyUnit(nextUnit){
   const hz=inputFrequencyHz();
   tuneUnit.value=nextUnit;
   activeFrequencyUnit=nextUnit;
   tuneInput.value=Number.isFinite(hz)?(hz/unitScale(nextUnit)).toFixed(nextUnit==='mhz'?4:nextUnit==='khz'?3:0):'';
   tuneInput.step=String(Math.max(1,frequencyStepHz/unitScale(nextUnit)));
 }
 const initialHz=Number(tuneInput.value);
 setFrequencyUnit(initialHz>=64000000?'mhz':'khz');
 tuneUnit.addEventListener('change',()=>setFrequencyUnit(tuneUnit.value));
 const stepDown=document.getElementById('radioStepDown');
 const stepUp=document.getElementById('radioStepUp');
 const volumeInput=document.getElementById('radioVolume');
 const volumeValue=document.getElementById('radioVolumeValue');
 const volumeStatus=document.getElementById('radioVolumeStatus');
 async function postRadioCommand(path,params,statusElement){
   try{
     const response=await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(params),cache:'no-store'});
     const data=await response.json().catch(()=>({}));
     if(!response.ok) throw new Error(data.message||data.error||'Radio command failed.');
     statusElement.textContent=data.message||'Command accepted.';
     return true;
   }catch(error){
     statusElement.textContent=error.message||'Radio command failed.';
     return false;
   }
 }
 tuneForm.addEventListener('submit',async event=>{
   event.preventDefault();
   const frequency=inputFrequencyHz();
   if(!Number.isFinite(frequency)||frequency<100000||frequency>300000000){
     controlStatus.textContent='Enter a valid frequency between 100 kHz and 300 MHz.';
     return;
   }
   const button=tuneForm.querySelector('button[type=submit]');
   button.disabled=true;
   const ok=await postRadioCommand('/radio/tune',{freq:String(Math.round(frequency))},controlStatus);
   button.disabled=false;
   if(ok) setTimeout(()=>location.reload(),900);
 });
 async function stepRadio(direction,button){
   button.disabled=true;
   const ok=await postRadioCommand('/radio/step',{direction},controlStatus);
   if(ok) setTimeout(()=>location.reload(),700);
   else button.disabled=false;
 }
 stepDown.addEventListener('click',()=>stepRadio('down',stepDown));
 stepUp.addEventListener('click',()=>stepRadio('up',stepUp));
 let volumeTimer=0;
 volumeInput.addEventListener('input',()=>{
   volumeValue.textContent=volumeInput.value;
   volumeStatus.textContent='Setting volume to '+volumeInput.value+'…';
   clearTimeout(volumeTimer);
   volumeTimer=setTimeout(async()=>{
     volumeInput.disabled=true;
     await postRadioCommand('/radio/volume',{volume:volumeInput.value},volumeStatus);
     volumeInput.disabled=false;
   },180);
 });
 const canvas=document.getElementById('spectrumCanvas');
 const ctx=canvas.getContext('2d');
 const spectrumForm=document.getElementById('spectrumRange');
 const startInput=document.getElementById('spectrumStartMHz');
 const endInput=document.getElementById('spectrumEndMHz');
 const modeInput={value:'FM'};
 const modeButtons=Array.from(document.querySelectorAll('[data-mode]'));
 const stepInput=document.getElementById('spectrumStep');
 const unitStart=document.getElementById('spectrumUnitStart');
 const unitEnd=document.getElementById('spectrumUnitEnd');
 const button=document.getElementById('spectrumStart');
 const status=document.getElementById('spectrumStatus');
 let lastSamples=[];
 let peakPoints=[];
 let fmMode=false;
 let lastSpectrumMode='FM';
 function draw(samples,fm=fmMode){
   fmMode=fm;
   lastSamples=samples||[];
   peakPoints=[];
   const dpr=window.devicePixelRatio||1;
   const w=Math.max(260,canvas.clientWidth), h=190;
   canvas.width=Math.round(w*dpr); canvas.height=Math.round(h*dpr);
   ctx.setTransform(dpr,0,0,dpr,0,0);
   ctx.fillStyle='#050810';ctx.fillRect(0,0,w,h);
   const left=34,right=10,top=12,bottom=28,plotW=w-left-right,plotH=h-top-bottom;
   ctx.strokeStyle='#293958';ctx.lineWidth=1;
   ctx.fillStyle='#a5b9e1';ctx.font='10px system-ui';
   if(samples.length<2){
     for(let i=0;i<5;i++){const y=top+plotH*i/4;ctx.beginPath();ctx.moveTo(left,y);ctx.lineTo(w-right,y);ctx.stroke();}
     ctx.fillStyle='#8ea2ae';ctx.font='12px system-ui';ctx.fillText('No sweep data yet',left+8,top+22);return;
   }
   const minF=samples[0].f,maxF=samples[samples.length-1].f;
   const values=samples.map(p=>p.r);
   const rawMin=Math.min(...values),rawMax=Math.max(...values);
   const padding=Math.max(4,(rawMax-rawMin)*.15);
   const minR=Math.max(0,rawMin-padding),maxR=rawMax+padding,span=Math.max(10,maxR-minR);
   for(let i=0;i<5;i++){
     const y=top+plotH*i/4;
     ctx.beginPath();ctx.moveTo(left,y);ctx.lineTo(w-right,y);ctx.stroke();
     ctx.fillText(String(Math.round(maxR-(maxR-minR)*i/4)),3,y+3);
   }
   ctx.fillText('RSSI',4,9);
   const pts=samples.map((p,i)=>({x:left+i*plotW/(samples.length-1),y:top+plotH-(p.r-minR)/span*plotH,index:i}));
   ctx.beginPath();ctx.moveTo(pts[0].x,top+plotH);pts.forEach(p=>ctx.lineTo(p.x,p.y));ctx.lineTo(pts[pts.length-1].x,top+plotH);ctx.closePath();
   ctx.fillStyle='rgba(0,65,255,.22)';ctx.fill();
   ctx.beginPath();pts.forEach((p,i)=>i?ctx.lineTo(p.x,p.y):ctx.moveTo(p.x,p.y));
   ctx.strokeStyle='#167bff';ctx.lineWidth=2.5;ctx.shadowColor='#087cff';ctx.shadowBlur=8;ctx.stroke();ctx.shadowBlur=0;
   peakPoints=[];
   const threshold=rawMin+(rawMax-rawMin)*0.22;
   samples.forEach((sample,i)=>{
     if(i===0||i===samples.length-1)return;
     if(sample.r>=samples[i-1].r&&sample.r>samples[i+1].r&&sample.r>=threshold){
       const p=pts[i];peakPoints.push(p);
       ctx.beginPath();ctx.arc(p.x,p.y,4,0,Math.PI*2);ctx.fillStyle='#ff4c50';ctx.shadowColor='#ff334b';ctx.shadowBlur=12;ctx.fill();ctx.shadowBlur=0;
       ctx.strokeStyle='#ffd4dc';ctx.lineWidth=1;ctx.stroke();
     }
   });
   ctx.fillStyle='#8ea2ae';ctx.font='10px monospace';
   const fmt=f=>fmMode?(f/100).toFixed(2)+' MHz':f+' kHz';
   ctx.textAlign='left';ctx.fillText(fmt(minF),left,h-8);
   ctx.textAlign='right';ctx.fillText(fmt(maxF),w-right,h-8);
   ctx.textAlign='center';ctx.fillStyle='#167bff';ctx.fillText('TAP RED PEAK TO TUNE',w/2, h-8);
 }
 async function start(event){
   event.preventDefault();
   const selectedMode=modeInput.value;
   const startValue=Number(startInput.value),endValue=Number(endInput.value);
   const fm=selectedMode==='FM';
   const minValue=fm?64:150,maxValue=fm?108:30000;
   const stepHz=Number(stepInput.value);
   const startHz=Math.round(startValue*(fm?1000000:1000));
   const endHz=Math.round(endValue*(fm?1000000:1000));
   if(!Number.isFinite(startValue)||!Number.isFinite(endValue)||startValue<minValue||endValue>maxValue||endValue<=startValue){
     status.textContent=fm?'Enter a valid FM range: 64–108 MHz.':'Enter a valid AM/SSB range: 150–30000 kHz.';
     return;
   }
   if(Math.floor((endHz-startHz)/stepHz)+1>256){
     status.textContent='Range is too wide for 256 samples at this step. Choose a larger step or narrower range.';
     return;
   }
   button.disabled=true;
   const tuneButton=tuneForm.querySelector('button[type=submit]');
   tuneButton.disabled=true;stepDown.disabled=true;stepUp.disabled=true;volumeInput.disabled=true;
   startInput.disabled=true;endInput.disabled=true;stepInput.disabled=true;modeButtons.forEach(item=>item.disabled=true);
   status.textContent='Starting receiver sweep…';
   try{
     const params=new URLSearchParams({start_hz:String(startHz),end_hz:String(endHz),mode:selectedMode,step_hz:String(stepHz)});
     const start=await fetch('/spectrum/start',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:params,cache:'no-store'});
     if(!start.ok){const e=await start.json().catch(()=>({}));throw new Error(e.message||e.error||'Scan could not be started.');}
     for(let i=0;i<45;i++){
       await new Promise(resolve=>setTimeout(resolve,1000));
       const response=await fetch('/spectrum.json',{cache:'no-store'});
       const data=await response.json();
       if(data.ready&&data.samples&&data.samples.length){
         draw(data.samples,!!data.fm);
         lastSpectrumMode=data.mode||selectedMode;
         try{sessionStorage.setItem('atsSpectrumSnapshot',JSON.stringify({fm:!!data.fm,mode:lastSpectrumMode,samples:data.samples}));}catch(_){}
         status.textContent='Sweep complete · '+data.samples.length+' real RSSI samples. Run again to refresh.';
         return;
       }
       status.textContent='Measuring receiver signal… '+(i+1)+' s';
     }
     throw new Error('Sweep timed out. Try again.');
   }catch(error){status.textContent=error.message||'Spectrum request failed.';}
   finally{button.disabled=false;tuneButton.disabled=false;stepDown.disabled=false;stepUp.disabled=false;volumeInput.disabled=false;startInput.disabled=false;endInput.disabled=false;stepInput.disabled=false;modeButtons.forEach(item=>item.disabled=false);}
 }
 function updateSpectrumMode(){
   const fm=modeInput.value==='FM';
   unitStart.textContent=fm?'MHz':'kHz';unitEnd.textContent=fm?'MHz':'kHz';
   startInput.min=fm?'64':'150';endInput.min=fm?'64':'150';
   startInput.max=fm?'108':'30000';endInput.max=fm?'108':'30000';
   startInput.step=fm?'0.1':'1';endInput.step=fm?'0.1':'1';
   if(fm){startInput.value='88.00';endInput.value='108.00';stepInput.value='100000';}
   else {startInput.value='3000';endInput.value='3600';stepInput.value='5000';}
   Array.from(stepInput.options).forEach(option=>{option.disabled=(fm&&option.value!=='100000')||(!fm&&option.value==='100000');});
 }
 modeButtons.forEach(button=>button.addEventListener('click',()=>{
   modeInput.value=button.dataset.mode;
   modeButtons.forEach(item=>item.classList.toggle('ACTIVE',item===button));
   updateSpectrumMode();
 }));
 updateSpectrumMode();
 spectrumForm.addEventListener('submit',start);
 canvas.addEventListener('click',async event=>{
   if(!peakPoints.length||!lastSamples.length)return;
   const rect=canvas.getBoundingClientRect();
   const scaleX=canvas.width/(window.devicePixelRatio||1)/rect.width;
   const x=(event.clientX-rect.left)*scaleX;
   let nearest=null,distance=Infinity;
   peakPoints.forEach(p=>{const d=Math.abs(p.x-x);if(d<distance){distance=d;nearest=p;}});
   if(!nearest||distance>22){status.textContent='Tap one of the red peaks to tune to that station.';return;}
   const sample=lastSamples[nearest.index];
   const frequencyHz=Math.round(sample.f*(fmMode?10000:1000));
   status.textContent='Tuning to '+(fmMode?(sample.f/100).toFixed(2)+' MHz':sample.f+' kHz')+'…';
   const ok=await postRadioCommand('/radio/tune',{freq:String(frequencyHz),mode:lastSpectrumMode},status);
   if(ok) status.textContent='Tuned to '+(fmMode?(sample.f/100).toFixed(2)+' MHz':sample.f+' kHz')+'. Spectrum retained; scan again whenever you want fresh measurements.';
 });
 let restored=false;
 try{
   const saved=JSON.parse(sessionStorage.getItem('atsSpectrumSnapshot')||'null');
   if(saved&&Array.isArray(saved.samples)&&saved.samples.length>1){
     draw(saved.samples,!!saved.fm);
     lastSpectrumMode=saved.mode||(saved.fm?'FM':'AM');
     status.textContent='Previous sweep restored · '+saved.samples.length+' samples. Scan again whenever you want fresh measurements.';
     restored=true;
   }
 }catch(_){}
 if(!restored)draw([]);
 window.addEventListener('resize',()=>draw(lastSamples));
})();
</SCRIPT>
)HTML";

#endif // PAGE_STATUS_H
