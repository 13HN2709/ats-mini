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
  <DIV CLASS="FREQ">{{frequency}}</DIV>
  <DIV CLASS="MODE">{{band}} &nbsp;·&nbsp; {{mode}} {{{stereo}}}</DIV>
</SECTION>
<SECTION CLASS="STAT" STYLE="margin-top:14px">
  <DIV CLASS="EYEBROW">Radio controls</DIV>
  <FORM ID="radioTune" STYLE="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;margin-top:10px">
    <LABEL FOR="radioFrequency" CLASS="LABEL" STYLE="grid-column:1/-1">Frequency (Hz)</LABEL>
    <INPUT ID="radioFrequency" NAME="freq" TYPE="number" INPUTMODE="decimal" MIN="100000" MAX="300000000" STEP="100" VALUE="{{frequency_hz}}" REQUIRED>
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
<SECTION CLASS="STAT" STYLE="margin-top:14px">
  <DIV STYLE="display:flex;align-items:center;justify-content:space-between;gap:12px;flex-wrap:wrap">
    <DIV><DIV CLASS="LABEL">LIVE RECEIVER SWEEP</DIV><DIV CLASS="VALUE" STYLE="font-size:1.05rem;color:#80a0ff">Spectrum analyzer</DIV></DIV>
    <BUTTON ID="spectrumStart" TYPE="button">RUN SPECTRUM</BUTTON>
  </DIV>
  <DIV STYLE="margin-top:12px;background:#050810;border:1px solid #40527b;border-radius:12px;padding:8px">
    <CANVAS ID="spectrumCanvas" HEIGHT="190" STYLE="display:block;width:100%;height:190px"></CANVAS>
  </DIV>
  <DIV ID="spectrumStatus" CLASS="LABEL" STYLE="margin-top:10px">Press RUN SPECTRUM to measure real RSSI values across the current band. Audio is briefly muted during the sweep.</DIV>
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
 const controlStatus=document.getElementById('radioControlStatus');
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
   const frequency=Number(tuneInput.value);
   if(!Number.isFinite(frequency)||frequency<100000||frequency>300000000){
     controlStatus.textContent='Enter a valid frequency in Hz.';
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
 const button=document.getElementById('spectrumStart');
 const status=document.getElementById('spectrumStatus');
 let lastSamples=[];
 let fmMode=false;
 function draw(samples,fm=fmMode){
   fmMode=fm;
   lastSamples=samples||[];
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
   const pts=samples.map((p,i)=>({x:left+i*plotW/(samples.length-1),y:top+plotH-(p.r-minR)/span*plotH}));
   ctx.beginPath();ctx.moveTo(pts[0].x,top+plotH);pts.forEach(p=>ctx.lineTo(p.x,p.y));ctx.lineTo(pts[pts.length-1].x,top+plotH);ctx.closePath();
   ctx.fillStyle='rgba(95,135,255,.16)';ctx.fill();
   ctx.beginPath();pts.forEach((p,i)=>i?ctx.lineTo(p.x,p.y):ctx.moveTo(p.x,p.y));
   ctx.strokeStyle='#80a0ff';ctx.lineWidth=2;ctx.stroke();
   ctx.fillStyle='#8ea2ae';ctx.font='10px system-ui';
   const fmt=f=>fmMode?(f/100).toFixed(2)+' MHz':f+' kHz';
   ctx.textAlign='left';ctx.fillText(fmt(minF),left,h-8);
   ctx.textAlign='right';ctx.fillText(fmt(maxF),w-right,h-8);
   ctx.textAlign='center';ctx.fillStyle='#80a0ff';ctx.fillText('RSSI',w/2, h-8);
 }
 async function start(){
   button.disabled=true;
   const tuneButton=tuneForm.querySelector('button[type=submit]');
   tuneButton.disabled=true;stepDown.disabled=true;stepUp.disabled=true;
   status.textContent='Starting receiver sweep…';
   try{
     const start=await fetch('/spectrum/start',{method:'POST',cache:'no-store'});
     if(!start.ok){const e=await start.json().catch(()=>({}));throw new Error(e.message||e.error||'Scan could not be started.');}
     for(let i=0;i<45;i++){
       await new Promise(resolve=>setTimeout(resolve,1000));
       const response=await fetch('/spectrum.json',{cache:'no-store'});
       const data=await response.json();
       if(data.ready&&data.samples&&data.samples.length){
         draw(data.samples,!!data.fm);
         status.textContent='Sweep complete · '+data.samples.length+' real RSSI samples. Run again to refresh.';
         return;
       }
       status.textContent='Measuring receiver signal… '+(i+1)+' s';
     }
     throw new Error('Sweep timed out. Try again.');
   }catch(error){status.textContent=error.message||'Spectrum request failed.';}
   finally{button.disabled=false;tuneButton.disabled=false;stepDown.disabled=false;stepUp.disabled=false;}
 }
 button.addEventListener('click',start);
 draw([]);
 window.addEventListener('resize',()=>draw(lastSamples));
})();
</SCRIPT>
)HTML";

#endif // PAGE_STATUS_H
