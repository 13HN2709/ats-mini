#ifndef PAGE_STATUS_H
#define PAGE_STATUS_H

#include <Arduino.h>

static const char pageStatusTitle[] PROGMEM = "ATS-Mini Status";

static const char pageStatus[] PROGMEM = R"HTML(<DIV CLASS="TOP">
  <DIV CLASS="BRAND"><DIV CLASS="LOGO">&#9673;</DIV><DIV><H1>ATS MINI <SPAN STYLE="color:#80a0ff">CONTROLLER</SPAN></H1><DIV CLASS="SUB">Local radio control center</DIV></DIV></DIV>
  <SPAN CLASS="PILL">&#9679; {{connection}}</SPAN>
</DIV>
{{{navigation}}}
<SECTION CLASS="HERO" STYLE="padding:20px 14px 22px">
  <DIV ID="rdsStation" CLASS="FREQ LED-FREQ" STYLE="line-height:1.2;min-height:1.2em;overflow-wrap:anywhere;text-align:center"> </DIV>
  <DIV CLASS="EYEBROW" STYLE="margin:4px 0 8px">Now receiving</DIV>
  <DIV CLASS="FREQ LED-FREQ">{{{led_frequency}}}</DIV>
  <DIV CLASS="MODE">{{band}} &nbsp;·&nbsp; {{mode}} {{{stereo}}}</DIV>
  <DIV ID="rdsText" CLASS="FREQ LED-FREQ" STYLE="line-height:1.2;margin-top:12px;min-height:1.2em;overflow-wrap:anywhere;text-align:center"></DIV>
  <DIV ID="rdsPtyRow" STYLE="display:flex;justify-content:center;align-items:center;gap:8px;margin-top:8px;min-height:1.4em;color:#80a0ff;font-family:monospace;font-size:1rem;font-weight:700;letter-spacing:.06em;text-align:center;overflow-wrap:anywhere">
    <SPAN STYLE="opacity:.75">PTY</SPAN><SPAN ID="rdsPty"></SPAN>
  </DIV>
</SECTION>
<SECTION CLASS="STAT" ID="frequencyKeypadPanel" STYLE="margin-top:14px;padding:14px">
  <DIV CLASS="EYEBROW" STYLE="margin-bottom:12px">FREQUENCY KEYPAD</DIV>
  <FORM ID="manualTuneForm" STYLE="display:grid;grid-template-columns:minmax(0,1fr) minmax(92px,.55fr);gap:8px;margin:0">
    <LABEL STYLE="grid-column:1/-1">Frequency / Band</LABEL>
    <INPUT ID="manualTuneFrequency" TYPE="text" INPUTMODE="decimal" AUTOCOMPLETE="off" PLACEHOLDER="e.g. 14100 or 88.50" REQUIRED STYLE="width:100%;min-width:0">
    <SELECT ID="manualTuneMode" aria-label="Band and modulation" STYLE="width:100%;min-width:0">
      <OPTION VALUE="FM" SELECTED>FM · MHz</OPTION>
      <OPTION VALUE="AM">AM · kHz</OPTION>
      <OPTION VALUE="USB">USB · kHz</OPTION>
      <OPTION VALUE="LSB">LSB · kHz</OPTION>
    </SELECT>
    <BUTTON ID="manualTuneButton" TYPE="submit" STYLE="grid-column:1/-1">TUNE FREQUENCY</BUTTON>
    <DIV ID="manualTuneStatus" CLASS="LABEL" ROLE="status" STYLE="grid-column:1/-1">Enter a frequency and select its band / modulation.</DIV>
  </FORM>
</SECTION>
<SECTION CLASS="STAT" ID="memoryQuickPanel" STYLE="margin-top:14px;padding:0;overflow:hidden">
  <BUTTON ID="memoryQuickToggle" TYPE="button" aria-expanded="false" aria-controls="memoryQuickList" STYLE="display:flex;align-items:center;justify-content:space-between;width:100%;padding:14px 16px;background:transparent;border:0;color:inherit;text-align:left;cursor:pointer">
    <SPAN STYLE="display:flex;align-items:center;gap:10px"><SPAN STYLE="color:#80a0ff;font-size:1.15em">&#9835;</SPAN><SPAN CLASS="EYEBROW" STYLE="margin:0">MEMORIES</SPAN><SMALL ID="memoryQuickCount" CLASS="LABEL">Loading…</SMALL></SPAN>
    <SPAN ID="memoryQuickArrow" STYLE="color:#80a0ff;font-size:1.1em">&#9662;</SPAN>
  </BUTTON>
  <DIV ID="memoryQuickList" hidden STYLE="padding:0 12px 12px">
    <DIV ID="memoryQuickItems" STYLE="display:grid;grid-template-columns:repeat(auto-fit,minmax(125px,1fr));gap:7px"></DIV>
    <DIV ID="memoryQuickStatus" CLASS="LABEL" ROLE="status" STYLE="margin-top:8px">Loading saved stations…</DIV>
  </DIV>
</SECTION>
<SECTION CLASS="STAT" ID="frequencyGuidePanel" STYLE="margin-top:14px;padding:0;overflow:hidden">
  <BUTTON ID="frequencyGuideToggle" TYPE="button" aria-expanded="false" aria-controls="frequencyGuideList" STYLE="display:flex;align-items:center;justify-content:space-between;width:100%;padding:14px 16px;background:transparent;border:0;color:inherit;text-align:left;cursor:pointer">
    <SPAN STYLE="display:flex;align-items:center;gap:10px"><SPAN STYLE="color:#35e8ff;font-size:1.15em">&#128225;</SPAN><SPAN CLASS="EYEBROW" STYLE="margin:0">FREQUENCY GUIDE</SPAN><SMALL ID="frequencyGuideCount" CLASS="LABEL">World DX</SMALL></SPAN>
    <SPAN ID="frequencyGuideArrow" STYLE="color:#35e8ff;font-size:1.1em">&#9662;</SPAN>
  </BUTTON>
  <DIV ID="frequencyGuideList" hidden STYLE="padding:0 12px 12px">
    <DIV CLASS="LABEL" STYLE="margin:0 4px 10px">Worldwide reference frequencies · Tap an entry to tune. Reception and activity depend on propagation and schedule.</DIV>
    <DIV ID="frequencyGuideGroups" STYLE="display:grid;gap:12px"></DIV>
    <DIV ID="frequencyGuideStatus" CLASS="LABEL" ROLE="status" STYLE="margin-top:10px">Select a frequency to tune the receiver.</DIV>
  </DIV>
</SECTION>
<SECTION CLASS="STAT" STYLE="margin-top:14px">
  <DIV CLASS="EYEBROW">Radio controls</DIV>
  <FORM ID="radioTune" STYLE="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;margin-top:10px">
    <LABEL FOR="radioFrequency" CLASS="LABEL" STYLE="grid-column:1/-1;display:flex;align-items:center;justify-content:space-between;gap:10px">Frequency
      <SELECT ID="radioFrequencyUnit" aria-label="Frequency unit" STYLE="width:auto;min-width:86px"><OPTION VALUE="khz" SELECTED>kHz</OPTION><OPTION VALUE="mhz">MHz</OPTION></SELECT>
    </LABEL>
    <INPUT ID="radioFrequency" NAME="freq" TYPE="text" INPUTMODE="decimal" AUTOCOMPLETE="off" VALUE="{{frequency_hz}}" STYLE="width:100%;min-width:0" REQUIRED>
    <BUTTON TYPE="submit">TUNE</BUTTON>
  </FORM>
  <DIV ID="radioControlStatus" CLASS="LABEL" ROLE="status" STYLE="margin-top:8px">Enter a frequency supported by the current band.</DIV>
  <DIV STYLE="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:10px">
    <BUTTON ID="radioStepDown" TYPE="button" aria-label="Tune down one step">− STEP</BUTTON>
    <BUTTON ID="radioStepUp" TYPE="button" aria-label="Tune up one step">+ STEP</BUTTON>
  </DIV>
  <DIV STYLE="margin-top:16px;border:1px solid #223b68;border-radius:8px;overflow:hidden">
    <BUTTON ID="radioVolumeToggle" TYPE="button" aria-expanded="false" aria-controls="radioVolumePanel" STYLE="display:flex;align-items:center;justify-content:space-between;gap:10px;width:100%;padding:12px;background:#091326;color:#eaf0ff;border:0;text-align:left;cursor:pointer">
      <SPAN STYLE="display:flex;align-items:center;gap:9px"><SPAN STYLE="color:#80a0ff;font-size:1.1em">&#128266;</SPAN><SPAN CLASS="LABEL" STYLE="margin:0">Volume</SPAN></SPAN>
      <SPAN STYLE="display:flex;align-items:center;gap:12px"><STRONG ID="radioVolumeValue" STYLE="color:#80a0ff;font-family:monospace;font-weight:900;font-variant-numeric:tabular-nums">{{volume}}</STRONG><SPAN ID="radioVolumeArrow" STYLE="color:#80a0ff">&#9662;</SPAN></SPAN>
    </BUTTON>
    <DIV ID="radioVolumePanel" hidden STYLE="padding:12px;background:#060d1b">
      <INPUT ID="radioVolume" TYPE="range" MIN="0" MAX="63" STEP="1" VALUE="{{volume}}" STYLE="width:100%;accent-color:#167bff;margin:2px 0 8px">
      <DIV ID="radioVolumeStatus" CLASS="LABEL" ROLE="status">Adjust volume · 0–63</DIV>
    </DIV>
  </DIV>
</SECTION>
<SECTION CLASS="SPECTRUM-PANEL">
  <DIV CLASS="PANEL-HEAD">
    <DIV><DIV CLASS="EYEBROW">01 / RF ANALYSIS</DIV><H2>SPECTRUM SCAN</H2><DIV CLASS="SUB">FM spectrum scan · 64–108 MHz</DIV></DIV>
    <DIV CLASS="LIVE-MARK"><SPAN></SPAN> RECEIVER</DIV>
  </DIV>
    <FORM ID="spectrumRange" CLASS="RANGE-FORM">
    <LABEL>START <SPAN ID="spectrumUnitStart">MHz</SPAN><INPUT ID="spectrumStartMHz" TYPE="number" INPUTMODE="decimal" MIN="64" MAX="108" STEP="0.1" VALUE="88.00" REQUIRED></LABEL>
    <LABEL>STOP <SPAN ID="spectrumUnitEnd">MHz</SPAN><INPUT ID="spectrumEndMHz" TYPE="number" INPUTMODE="decimal" MIN="64" MAX="108" STEP="0.1" VALUE="108.00" REQUIRED></LABEL>
    <LABEL>MODULATION<DIV STYLE="margin-top:6px;padding:10px 12px;border:1px solid #167bff;background:#102a55;color:#eaf0ff;font-weight:800;text-align:center">FM ONLY</DIV></LABEL>
    <LABEL>STEP<SELECT ID="spectrumStep" STYLE="width:100%;margin-top:6px"><OPTION VALUE="100000" SELECTED>100 kHz</OPTION></SELECT></LABEL>
    <BUTTON ID="spectrumStart" TYPE="submit">▶ SCAN RANGE</BUTTON>
  </FORM>
  <DIV CLASS="SPECTRUM-FRAME">
    <DIV CLASS="SPECTRUM-META"><SPAN>FIELD STRENGTH / RSSI</SPAN><SPAN>PEAKS ARE SELECTABLE</SPAN></DIV>
    <CANVAS ID="spectrumCanvas" HEIGHT="230" STYLE="display:block;width:100%;height:230px;cursor:crosshair" aria-label="Clickable all-band spectrum plot"></CANVAS>
  </DIV>
  <DIV ID="spectrumStatus" CLASS="SCAN-STATUS" ROLE="status">Choose a modulation and frequency range, then scan. Tap a red peak to tune the receiver.</DIV>
</SECTION>
<SECTION CLASS="SPECTRUM-PANEL" ID="allBandScannerPanel" STYLE="margin-top:14px">
  <DIV CLASS="PANEL-HEAD">
    <DIV><DIV CLASS="EYEBROW">02 / HF &amp; SHORTWAVE</DIV><H2>ALL BAND SCANNER</H2><DIV CLASS="SUB">AM / USB / LSB · 150–30000 kHz</DIV></DIV>
    <DIV CLASS="LIVE-MARK"><SPAN></SPAN> RECEIVER</DIV>
  </DIV>
  <FORM ID="allBandSpectrumForm" CLASS="RANGE-FORM">
    <LABEL>START <SPAN>kHz</SPAN><INPUT ID="allBandStart" TYPE="number" MIN="150" MAX="29995" STEP="1" VALUE="" PLACEHOLDER="Start kHz" REQUIRED></LABEL>
    <LABEL>STOP <SPAN>kHz</SPAN><INPUT ID="allBandEnd" TYPE="number" MIN="151" MAX="30000" STEP="1" VALUE="" PLACEHOLDER="Stop kHz" REQUIRED></LABEL>
    <LABEL>MODULATION<SELECT ID="allBandMode" STYLE="width:100%;margin-top:6px"><OPTION VALUE="AM" SELECTED>AM</OPTION><OPTION VALUE="USB">USB</OPTION><OPTION VALUE="LSB">LSB</OPTION></SELECT></LABEL>
    <LABEL>STEP<SELECT ID="allBandStep" STYLE="width:100%;margin-top:6px"><OPTION VALUE="5" SELECTED>5 kHz</OPTION><OPTION VALUE="10">10 kHz</OPTION><OPTION VALUE="25">25 kHz</OPTION><OPTION VALUE="50">50 kHz</OPTION><OPTION VALUE="100">100 kHz</OPTION><OPTION VALUE="250">250 kHz</OPTION><OPTION VALUE="500">500 kHz</OPTION><OPTION VALUE="1000">1000 kHz</OPTION></SELECT></LABEL>
    <BUTTON ID="allBandStartScan" TYPE="submit">▶ SCAN ALL BAND</BUTTON>
  </FORM>
  <DIV CLASS="SPECTRUM-FRAME">
    <DIV CLASS="SPECTRUM-META"><SPAN>FIELD STRENGTH / RSSI</SPAN><SPAN>PEAKS ARE SELECTABLE</SPAN></DIV>
    <CANVAS ID="allBandCanvas" HEIGHT="230" STYLE="display:block;width:100%;height:230px;cursor:crosshair" aria-label="Clickable ALL Band spectrum plot"></CANVAS>
  </DIV>
  <DIV ID="allBandStatus" CLASS="SCAN-STATUS" ROLE="status">Choose a shortwave range and modulation, then scan. Tap a red peak to tune the receiver.</DIV>
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
 const LOG_KEY='atsControlDebugLog';
 window.atsDebugLog=function(type,message){
   try{const entries=JSON.parse(localStorage.getItem(LOG_KEY)||'[]');entries.push({time:new Date().toISOString(),type:String(type),message:String(message)});while(entries.length>250)entries.shift();localStorage.setItem(LOG_KEY,JSON.stringify(entries));}catch(_){}
 };
 window.addEventListener('error',e=>window.atsDebugLog('JS ERROR',e.message+' @ '+e.filename+':'+e.lineno));
 window.addEventListener('unhandledrejection',e=>window.atsDebugLog('PROMISE ERROR',e.reason&&e.reason.message||e.reason||'unknown'));
 window.atsDebugLog('PAGE','Radio page loaded');
})();
(()=>{
 const memoryToggle=document.getElementById('memoryQuickToggle');
 const memoryList=document.getElementById('memoryQuickList');
 const memoryItems=document.getElementById('memoryQuickItems');
 const memoryCount=document.getElementById('memoryQuickCount');
 const memoryStatus=document.getElementById('memoryQuickStatus');
 const memoryArrow=document.getElementById('memoryQuickArrow');
 // Persist accordion state across reloads and browser sessions until the user closes it.
 const memoryOpenKey='atsQuickMemoriesOpen';
 const guideOpenKey='atsFrequencyGuideOpen';
 const readAccordionState=key=>{try{return localStorage.getItem(key)==='true';}catch(_){return false;}};
 const writeAccordionState=(key,isOpen)=>{try{localStorage.setItem(key,String(isOpen));}catch(_){}};
 const memoryInitiallyOpen=readAccordionState(memoryOpenKey);
 if(memoryInitiallyOpen){
   memoryList.hidden=false;
   memoryToggle.setAttribute('aria-expanded','true');
   memoryArrow.innerHTML='&#9652;';
 }
 memoryToggle.addEventListener('click',()=>{
   const expanded=memoryToggle.getAttribute('aria-expanded')==='true';
   memoryToggle.setAttribute('aria-expanded',String(!expanded));
   memoryList.hidden=expanded;
   memoryArrow.innerHTML=expanded?'&#9662;':'&#9652;';
   writeAccordionState(memoryOpenKey,!expanded);
 });
 async function refreshQuickMemories(){
   try{
     const response=await fetch('/memories.json',{cache:'no-store'});
     if(!response.ok)throw Error('Could not load memories.');
     const data=await response.json();
     memoryItems.replaceChildren();
     const saved=Array.isArray(data.memories)?data.memories:[];
     memoryCount.textContent=saved.length+' saved';
     if(!saved.length){memoryStatus.textContent='No named stations saved yet.';return;}
     memoryStatus.textContent='Tap a station to tune the receiver.';
     saved.forEach(memory=>{
       const button=document.createElement('button');
       button.type='button';
       button.style.cssText='display:flex;align-items:center;gap:8px;min-width:0;text-align:left;padding:10px;border:1px solid #344762;border-radius:8px;background:#101a2b;color:#eaf0ff;font:inherit;cursor:pointer';
       const icon=document.createElement('span');
       icon.textContent='♫';
       icon.style.cssText='color:#80a0ff;font-size:1.15em;flex-shrink:0';
       const label=document.createElement('span');
       label.textContent=memory.name;
       label.style.cssText='overflow:hidden;text-overflow:ellipsis;white-space:nowrap;font-weight:700';
       button.append(icon,label);
       button.title=memory.name+' · '+memory.displayFrequency+' · '+memory.mode;
       button.addEventListener('click',async()=>{
         memoryStatus.textContent='Tuning to '+memory.name+'…';
         memoryItems.querySelectorAll('button').forEach(item=>item.disabled=true);
         const ok=await postRadioCommand('/radio/tune',{freq:String(memory.freq),mode:memory.mode},memoryStatus);
         if(ok){memoryStatus.textContent='Tuned to '+memory.name+'. Updating receiver display…';setTimeout(()=>location.reload(),700);}
         else memoryItems.querySelectorAll('button').forEach(item=>item.disabled=false);
       });
       memoryItems.append(button);
     });
   }catch(error){memoryCount.textContent='Unavailable';memoryStatus.textContent=error.message||'Could not load memories.';}
 }
 refreshQuickMemories();

 // Built-in reference guide: static firmware data, independent of personal memories.
 const guideToggle=document.getElementById('frequencyGuideToggle');
 const guideList=document.getElementById('frequencyGuideList');
 const guideArrow=document.getElementById('frequencyGuideArrow');
 const guideGroups=document.getElementById('frequencyGuideGroups');
 const guideStatus=document.getElementById('frequencyGuideStatus');
 const guideInitiallyOpen=readAccordionState(guideOpenKey);
 if(guideInitiallyOpen){
   guideList.hidden=false;
   guideToggle.setAttribute('aria-expanded','true');
   guideArrow.innerHTML='&#9652;';
 }
 const guideEntries=[
   {group:'DX BEACONS · NCDXF / IARU',items:[
     {name:'20 m international beacon network',freq:14100000,mode:'USB',detail:'14.100 MHz · one of five coordinated beacon bands'},
     {name:'17 m international beacon network',freq:18110000,mode:'USB',detail:'18.110 MHz · propagation beacon'},
     {name:'15 m international beacon network',freq:21150000,mode:'USB',detail:'21.150 MHz · propagation beacon'},
     {name:'12 m international beacon network',freq:24930000,mode:'USB',detail:'24.930 MHz · propagation beacon'},
     {name:'10 m international beacon network',freq:28200000,mode:'USB',detail:'28.200 MHz · propagation beacon'}
   ]},
   {group:'DWD PINNEBERG · WEATHER / DATA',items:[
     {name:'DWD RTTY / weather data',freq:3855000,mode:'USB',detail:'3.855 MHz · data signal; external decoder needed'},
     {name:'DWD RTTY / weather data',freq:4583000,mode:'USB',detail:'4.583 MHz · data signal; external decoder needed'},
     {name:'DWD RTTY / weather data',freq:7646000,mode:'USB',detail:'7.646 MHz · data signal; external decoder needed'},
     {name:'DWD weather fax',freq:7880000,mode:'USB',detail:'7.880 MHz · scheduled weather fax; decoder needed'},
     {name:'DWD RTTY / weather data',freq:10100800,mode:'USB',detail:'10.1008 MHz · data signal; external decoder needed'},
     {name:'DWD RTTY / weather data',freq:11039000,mode:'USB',detail:'11.039 MHz · data signal; external decoder needed'},
     {name:'DWD weather fax',freq:13882500,mode:'USB',detail:'13.8825 MHz · scheduled weather fax; decoder needed'},
     {name:'DWD RTTY / weather data',freq:14467300,mode:'USB',detail:'14.4673 MHz · data signal; external decoder needed'}
   ]},
   {group:'TIME SIGNALS · AM',items:[
     {name:'WWV / WWVH',freq:2500000,mode:'AM',detail:'2.500 MHz · international time and frequency signal'},
     {name:'WWV / WWVH',freq:5000000,mode:'AM',detail:'5.000 MHz · international time and frequency signal'},
     {name:'WWV / WWVH',freq:10000000,mode:'AM',detail:'10.000 MHz · international time and frequency signal'},
     {name:'WWV / WWVH',freq:15000000,mode:'AM',detail:'15.000 MHz · international time and frequency signal'},
     {name:'WWV',freq:20000000,mode:'AM',detail:'20.000 MHz · US time and frequency signal'},
     {name:'WWV experimental',freq:25000000,mode:'AM',detail:'25.000 MHz · experimental; availability may change'}
   ]},
   {group:'SHORTWAVE · BROADCAST / DX',items:[
     {name:'Shortwave broadcast band',freq:3950000,mode:'AM',detail:'3.950 MHz · evening listening; activity varies'},
     {name:'60 m broadcast / utility region',freq:5000000,mode:'AM',detail:'5.000 MHz · signals vary by region and allocation'},
     {name:'49 m broadcast band',freq:6000000,mode:'AM',detail:'6.000 MHz · often useful after sunset'},
     {name:'41 m broadcast band',freq:7300000,mode:'AM',detail:'7.300 MHz · international broadcast region'},
     {name:'31 m broadcast band',freq:9500000,mode:'AM',detail:'9.500 MHz · international shortwave broadcasts'},
     {name:'25 m broadcast band',freq:11800000,mode:'AM',detail:'11.800 MHz · often useful in daytime'},
     {name:'19 m broadcast band',freq:15100000,mode:'AM',detail:'15.100 MHz · daytime DX opportunities'},
     {name:'16 m broadcast band',freq:17500000,mode:'AM',detail:'17.500 MHz · daytime DX opportunities'}
   ]},
   {group:'AMATEUR RADIO · DX / BEACONS',items:[
     {name:'80 m amateur band (SSB activity)',freq:3700000,mode:'LSB',detail:'3.700 MHz · regional contacts, mainly evening'},
     {name:'40 m amateur band (SSB activity)',freq:7100000,mode:'LSB',detail:'7.100 MHz · regional and long-distance contacts'},
     {name:'20 m amateur band (SSB activity)',freq:14200000,mode:'USB',detail:'14.200 MHz · international activity'},
     {name:'17 m amateur band (SSB activity)',freq:18130000,mode:'USB',detail:'18.130 MHz · long-distance contacts'},
     {name:'15 m amateur band (SSB activity)',freq:21285000,mode:'USB',detail:'21.285 MHz · DX when the band is open'},
     {name:'12 m amateur band (SSB activity)',freq:24950000,mode:'USB',detail:'24.950 MHz · DX when the band is open'},
     {name:'10 m amateur band (SSB activity)',freq:28400000,mode:'USB',detail:'28.400 MHz · highly dependent on propagation'}
   ]},
   {group:'MYSTERY SIGNALS · NUMBERS STATIONS',items:[
     {name:'The Buzzer · UVB-76 / UZB-76',freq:4625000,mode:'USB',detail:'4.625 MHz · Russian military channel marker; voice messages sometimes interrupt the buzzing'},
     {name:'V32 · Persian numbers station',freq:7910000,mode:'USB',detail:'7.910 MHz · Persian number groups; reported daily at 02:00 and 18:00 UTC in 2026'},
     {name:'V32 · alternate / interference frequency',freq:7842000,mode:'USB',detail:'7.842 MHz · reported alternate frequency after jamming; activity may vary'}
   ]},
   {group:'SIGNAL HUNTING · IDENTIFY BEFORE ASSUMING',items:[
     {name:'HF data / buzzer hunting',freq:10150000,mode:'USB',detail:'10.150 MHz · listening spot only; no activity guaranteed'},
     {name:'HF data / buzzer hunting',freq:14070000,mode:'USB',detail:'14.070 MHz · common digital-mode activity nearby; decoder needed'},
     {name:'HF data / buzzer hunting',freq:21070000,mode:'USB',detail:'21.070 MHz · common digital-mode activity nearby; decoder needed'}
   ]}
 ];
 guideToggle.addEventListener('click',()=>{
   const expanded=guideToggle.getAttribute('aria-expanded')==='true';
   guideToggle.setAttribute('aria-expanded',String(!expanded));
   guideList.hidden=expanded;
   guideArrow.innerHTML=expanded?'&#9662;':'&#9652;';
   writeAccordionState(guideOpenKey,!expanded);
 });
 guideEntries.forEach(group=>{
   const section=document.createElement('div');
   section.style.cssText='border:1px solid #223b68;border-radius:8px;overflow:hidden';
   const heading=document.createElement('div');
   heading.textContent=group.group;
   heading.style.cssText='padding:9px 10px;background:#091326;color:#80a0ff;font-size:.72rem;font-weight:800;letter-spacing:.08em';
   const items=document.createElement('div');
   items.style.cssText='display:grid;grid-template-columns:repeat(auto-fit,minmax(210px,1fr));gap:6px;padding:8px';
   group.items.forEach(entry=>{
     const button=document.createElement('button');
     button.type='button';
     button.style.cssText='display:flex;flex-direction:column;align-items:flex-start;gap:3px;min-width:0;text-align:left;padding:9px;border:1px solid #344762;border-radius:6px;background:#101a2b;color:#eaf0ff;font:inherit;cursor:pointer';
     const name=document.createElement('strong');name.textContent=entry.name;name.style.cssText='font-size:.82rem;line-height:1.25';
     const freq=document.createElement('span');freq.textContent=(entry.freq>=1000000?(entry.freq/1000000).toFixed(entry.freq%1000?4:3)+' MHz':(entry.freq/1000).toFixed(1)+' kHz')+' · '+entry.mode;
     freq.style.cssText='color:#35e8ff;font:700 .78rem monospace';
     const detail=document.createElement('span');detail.textContent=entry.detail;detail.style.cssText='color:#8ea2ae;font-size:.7rem;line-height:1.3';
     button.append(name,freq,detail);
     button.title=entry.name+' · '+entry.detail;
     button.addEventListener('click',async()=>{
       guideStatus.textContent='Tuning to '+(entry.freq>=1000000?(entry.freq/1000000).toFixed(4)+' MHz':(entry.freq/1000).toFixed(1)+' kHz')+' ('+entry.mode+')…';
       guideGroups.querySelectorAll('button').forEach(item=>item.disabled=true);
       const ok=await postRadioCommand('/radio/tune',{freq:String(entry.freq),mode:entry.mode},guideStatus);
       if(ok){guideStatus.textContent='Tune command queued. Receiver display will update…';setTimeout(()=>location.reload(),900);}
       else guideGroups.querySelectorAll('button').forEach(item=>item.disabled=false);
     });
     items.append(button);
   });
   section.append(heading,items);guideGroups.append(section);
 });
 guideToggle.querySelector('#frequencyGuideCount').textContent=guideEntries.reduce((sum,group)=>sum+group.items.length,0)+' frequencies';

 const tuneForm=document.getElementById('radioTune');
 const tuneInput=document.getElementById('radioFrequency');
 const tuneUnit=document.getElementById('radioFrequencyUnit');
 const controlStatus=document.getElementById('radioControlStatus');
 const frequencyStepHz=Number(tuneInput.step)||1000;
 const unitScale=unit=>unit==='mhz'?1000000:unit==='khz'?1000:1;
 let activeFrequencyUnit='hz'; // The template provides the canonical frequency in Hz.
 function inputFrequencyHz(){const raw=String(tuneInput.value).trim().replace(',','.');return raw!==''&&/^[0-9]+(?:\.[0-9]*)?$/.test(raw)?Number(raw)*unitScale(activeFrequencyUnit):NaN;}
 function setFrequencyUnit(nextUnit){
   const hz=inputFrequencyHz();
   tuneUnit.value=nextUnit;
   activeFrequencyUnit=nextUnit;
   tuneInput.value=Number.isFinite(hz)?(hz/unitScale(nextUnit)).toFixed(nextUnit==='mhz'?2:nextUnit==='khz'?3:0):'';
   tuneInput.dataset.step=String(Math.max(1,frequencyStepHz/unitScale(nextUnit)));
 }
 const initialHz=Number(tuneInput.value);
 setFrequencyUnit(initialHz>=64000000?'mhz':'khz');
 tuneUnit.addEventListener('change',()=>setFrequencyUnit(tuneUnit.value));
 const stepDown=document.getElementById('radioStepDown');
 const stepUp=document.getElementById('radioStepUp');
 const volumeToggle=document.getElementById('radioVolumeToggle');
 const volumePanel=document.getElementById('radioVolumePanel');
 const volumeArrow=document.getElementById('radioVolumeArrow');
 volumeToggle.addEventListener('click',()=>{
   const expanded=volumeToggle.getAttribute('aria-expanded')==='true';
   volumeToggle.setAttribute('aria-expanded',String(!expanded));
   volumePanel.hidden=expanded;
   volumeArrow.innerHTML=expanded?'&#9662;':'&#9652;';
 });
 const volumeInput=document.getElementById('radioVolume');
 const volumeValue=document.getElementById('radioVolumeValue');
 const volumeStatus=document.getElementById('radioVolumeStatus');
 async function postRadioCommand(path,params,statusElement){
   try{
     if(window.atsDebugLog) window.atsDebugLog('REQUEST',path+' '+JSON.stringify(params));
     const response=await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(params),cache:'no-store'});
     const data=await response.json().catch(()=>({}));
     if(!response.ok) throw new Error((data.message||data.error||'Radio command failed.')+' [HTTP '+response.status+']');
     if(window.atsDebugLog) window.atsDebugLog('RESPONSE',path+' HTTP '+response.status+' '+JSON.stringify(data));
     statusElement.textContent=data.message||'Command accepted.';
     if(path==='/radio/tune' && Number.isFinite(Number(data.request_id))){
       const requestId=Number(data.request_id);
       statusElement.textContent='Command accepted; waiting for receiver confirmation…';
       const deadline=Date.now()+6000;
       let lastState=null;
       const phaseNames={0:'idle',1:'queued',2:'processing',3:'switching band',4:'configuring receiver',5:'reading receiver',6:'identifying station',7:'complete',8:'selectBand: mute on',9:'selectBand: load DSP patch',10:'selectBand: configure receiver',11:'selectBand: bandwidth and station info',12:'selectBand: unmute'};
       while(Date.now()<deadline){
         await new Promise(resolve=>setTimeout(resolve,250));
         const stateResponse=await fetch('/radio/tune-status.json?ts='+Date.now(),{cache:'no-store'});
         if(!stateResponse.ok) continue;
         const state=await stateResponse.json();
         lastState=state;
         if(Number(state.completed_id)!==requestId) continue;
         const modeNames={0:'FM',1:'LSB',2:'USB',3:'AM'};
         const actualMode=modeNames[Number(state.actual_mode)]||'unknown mode';
         const actualHz=Number(state.actual_hz);
         const targetHz=Number(state.target_hz);
         const fmt=hz=>hz>=64000000?(hz/1000000).toFixed(2)+' MHz':(hz/1000).toFixed(0)+' kHz';
         if(Number(state.result)===1){
           statusElement.textContent='Confirmed: '+fmt(actualHz)+' '+actualMode+'.';
           if(window.atsDebugLog) window.atsDebugLog('CONFIRMED','/radio/tune '+JSON.stringify(state));
           return true;
         }
         statusElement.textContent='Receiver mismatch: requested '+fmt(targetHz)+', receiver reports '+fmt(actualHz)+' '+actualMode+'.';
         if(window.atsDebugLog) window.atsDebugLog('TUNE MISMATCH','/radio/tune '+JSON.stringify(state));
         return false;
       }
       const lastPhase=lastState?phaseNames[Number(lastState.phase)]||'unknown':'status unavailable';
       statusElement.textContent='Tune confirmation timed out. Last phase: '+lastPhase+'. Check the debug log.';
       if(window.atsDebugLog) window.atsDebugLog('TUNE TIMEOUT','/radio/tune request_id='+requestId+' last_state='+JSON.stringify(lastState));
       return false;
     }
     return true;
   }catch(error){
     if(window.atsDebugLog) window.atsDebugLog('REQUEST ERROR',path+' '+(error.message||error));
     statusElement.textContent=error.message||'Radio command failed.';
     return false;
   }
 }
 tuneForm.addEventListener('submit',async event=>{
   event.preventDefault();
   const frequency=inputFrequencyHz();
   const modeText=(document.querySelector('.HERO .MODE')?.textContent||'').trim();
   const currentModeMatch=modeText.match(/(?:^|[·\\s])(FM|AM|USB|LSB)(?:$|[·\\s])/);
   const currentMode=currentModeMatch?currentModeMatch[1]:'FM';
   const minFrequency=currentMode==='FM'?64000000:150000;
   const maxFrequency=currentMode==='FM'?108000000:30000000;
   if(!Number.isFinite(frequency)||frequency<minFrequency||frequency>maxFrequency){
     controlStatus.textContent=currentMode==='FM'
       ?'Invalid frequency for FM (64–108 MHz). Receiver frequency was not changed.'
       :'Invalid frequency for '+currentMode+' (150 kHz–30 MHz). Receiver frequency was not changed.';
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
 const manualTuneForm=document.getElementById('manualTuneForm');
  const manualTuneFrequency=document.getElementById('manualTuneFrequency');
  const manualTuneMode=document.getElementById('manualTuneMode');
  const manualTuneButton=document.getElementById('manualTuneButton');
  const manualTuneStatus=document.getElementById('manualTuneStatus');
  manualTuneMode.addEventListener('change',()=>{
    manualTuneFrequency.placeholder=manualTuneMode.value==='FM'?'e.g. 88.50 (MHz)':'e.g. 14100 (kHz)';
  });
  manualTuneForm.addEventListener('submit',async event=>{
    event.preventDefault();
    const mode=manualTuneMode.value;
    const raw=String(manualTuneFrequency.value).trim().replace(',','.');
    const value=Number(raw);
    if(!raw||!Number.isFinite(value)||value<=0){
      manualTuneStatus.textContent='Enter a valid frequency.';
      return;
    }
    const hz=Math.round(value*(mode==='FM'?1000000:1000));
    const minHz=mode==='FM'?64000000:150000;
    const maxHz=mode==='FM'?108000000:30000000;
    if(hz<minHz||hz>maxHz){
      manualTuneStatus.textContent=mode==='FM'
        ?'FM frequency must be between 64 and 108 MHz.'
        :'Frequency must be between 150 kHz and 30000 kHz for AM/USB/LSB.';
      return;
    }
    manualTuneButton.disabled=true;
    manualTuneStatus.textContent='Tuning to '+(mode==='FM'?(hz/1000000).toFixed(2)+' MHz':(hz/1000).toFixed(1)+' kHz')+' '+mode+'…';
    const ok=await postRadioCommand('/radio/tune',{freq:String(hz),mode},manualTuneStatus);
    manualTuneButton.disabled=false;
    if(ok){
      manualTuneStatus.textContent='Tuned to '+(mode==='FM'?(hz/1000000).toFixed(2)+' MHz':(hz/1000).toFixed(1)+' kHz')+' '+mode+'.';
      setTimeout(()=>location.reload(),700);
    }
  });
  const canvas=document.getElementById('spectrumCanvas');
 const ctx=canvas.getContext('2d');
 const spectrumForm=document.getElementById('spectrumRange');
 const startInput=document.getElementById('spectrumStartMHz');
 const endInput=document.getElementById('spectrumEndMHz');
 const modeInput={value:'FM'};
 const modeButtons=[]; // Spectrum scanning is intentionally FM-only.
 const stepInput=document.getElementById('spectrumStep');
 const unitStart=document.getElementById('spectrumUnitStart');
 const unitEnd=document.getElementById('spectrumUnitEnd');
 const button=document.getElementById('spectrumStart');
 const status=document.getElementById('spectrumStatus');
 const rdsStation=document.getElementById('rdsStation');
 const rdsText=document.getElementById('rdsText');
 const rdsPty=document.getElementById('rdsPty');
 const rdsPtyRow=document.getElementById('rdsPtyRow');
 async function refreshRds(){
   try{
     const response=await fetch('/rds.json',{cache:'no-store'});
     if(!response.ok)return;
     const data=await response.json();
     if(rdsStation)rdsStation.textContent=data.station||'';
     if(rdsText)rdsText.textContent=data.text||'';
     if(rdsPty)rdsPty.textContent=data.pty||'';
     if(rdsPtyRow)rdsPtyRow.style.display=data.pty?'flex':'none';
   }catch(_){}
 }
 refreshRds();
 setInterval(refreshRds,2000);
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
   saveSpectrumRange();
   if(window.atsDebugLog) window.atsDebugLog('SCAN','Start requested: mode='+selectedMode+', range='+startInput.value+'–'+endInput.value+', step='+stepInput.value);
   lastSamples=[];peakPoints=[];lastSpectrumMode=selectedMode;
   try{sessionStorage.removeItem('atsSpectrumSnapshotV2');}catch(_){}
   draw([]);
   const startValue=Number(startInput.value),endValue=Number(endInput.value);
   const fm=true;
   const minValue=64,maxValue=108;
   const stepHz=Number(stepInput.value);
   const startHz=Math.round(startValue*1000000);
   const endHz=Math.round(endValue*1000000);
   if(!Number.isFinite(startValue)||!Number.isFinite(endValue)||startValue<minValue||endValue>maxValue||endValue<=startValue){
     status.textContent='Enter a valid FM range: 64–108 MHz.';
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
   status.textContent='⏳ FM SPECTRUM SCAN RUNNING — PLEASE WAIT · Initializing receiver…';
   try{
     const params=new URLSearchParams({start_hz:String(startHz),end_hz:String(endHz),mode:selectedMode,step_hz:String(stepHz)});
     const start=await fetch('/spectrum/start',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:params,cache:'no-store'});
     if(window.atsDebugLog) window.atsDebugLog('SCAN RESPONSE','/spectrum/start HTTP '+start.status);
     if(!start.ok){const e=await start.json().catch(()=>({}));throw new Error((e.message||e.error||'Scan could not be started.')+' [HTTP '+start.status+']');}
     for(let i=0;i<45;i++){
       await new Promise(resolve=>setTimeout(resolve,1000));
       const response=await fetch('/spectrum.json',{cache:'no-store'});
       const data=await response.json();
       if(data.ready&&data.samples&&data.samples.length){
         draw(data.samples,!!data.fm);
         lastSpectrumMode=data.mode||selectedMode;
         try{sessionStorage.setItem('atsSpectrumSnapshotV2',JSON.stringify({fm:!!data.fm,mode:lastSpectrumMode,samples:data.samples}));}catch(_){}
         status.textContent='Sweep complete · '+data.samples.length+' real RSSI samples. Run again to refresh.';
         if(window.atsDebugLog) window.atsDebugLog('SCAN COMPLETE','mode='+lastSpectrumMode+', samples='+data.samples.length);
         return;
       }
       status.textContent='⏳ FM SPECTRUM SCAN RUNNING — PLEASE WAIT · Measuring signal… '+(i+1)+' s';
     }
     throw new Error('Sweep timed out. Try again.');
   }catch(error){if(window.atsDebugLog) window.atsDebugLog('SCAN ERROR',error.message||error);status.textContent=error.message||'Spectrum request failed.';}
   finally{button.disabled=false;tuneButton.disabled=false;stepDown.disabled=false;stepUp.disabled=false;volumeInput.disabled=false;startInput.disabled=false;endInput.disabled=false;stepInput.disabled=false;modeButtons.forEach(item=>item.disabled=false);}
 }
 function saveSpectrumRange(){
   try{sessionStorage.setItem('atsSpectrumRangeV1',JSON.stringify({mode:modeInput.value,start:startInput.value,end:endInput.value,step:stepInput.value}));}catch(_){}
 }
 function updateSpectrumMode(restoreSaved=false){
   const fm=true;
   unitStart.textContent='MHz';unitEnd.textContent='MHz';
   startInput.min='64';endInput.min='64';
   startInput.max='108';endInput.max='108';
   startInput.step='0.1';endInput.step='0.1';
   let restoredRange=null;
   if(restoreSaved){try{restoredRange=JSON.parse(sessionStorage.getItem('atsSpectrumRangeV1')||'null');}catch(_){}}
   const savedMatches=restoredRange&&restoredRange.mode===modeInput.value;
   const startValue=savedMatches?Number(restoredRange.start):Number(startInput.value);
   const endValue=savedMatches?Number(restoredRange.end):Number(endInput.value);
   const minValue=64,maxValue=108;
   const valid=Number.isFinite(startValue)&&Number.isFinite(endValue)&&startValue>=minValue&&endValue<=maxValue&&endValue>startValue;
   if(valid){startInput.value=String(startValue);endInput.value=String(endValue);}
   else if(fm){startInput.value='88.00';endInput.value='108.00';}
   else {startInput.value='88.00';endInput.value='108.00';}
   if(savedMatches&&restoredRange.step)stepInput.value=restoredRange.step;
   else if(!valid)stepInput.value=fm?'100000':'5000';
   stepInput.value='100000';
 }
 // Restore the selected modulation after the page reloads so the user can
 // switch the receiver mode immediately and still keep the matching scan range.
 let savedScanMode='FM';
 modeInput.value='FM';
 try{sessionStorage.setItem('atsSpectrumMode','FM');}catch(_){}
 modeButtons.forEach(item=>item.classList.toggle('ACTIVE',item.dataset.mode===modeInput.value));
 updateSpectrumMode(true);

 let modeSwitchBusy=false;
 modeButtons.forEach(button=>button.addEventListener('click',async()=>{
   const previousMode=modeInput.value;
   const selectedMode=button.dataset.mode;
   modeInput.value=selectedMode;
   modeButtons.forEach(item=>item.classList.toggle('ACTIVE',item===button));
   // Preserve a user-entered range when switching modulation; only use defaults if it is invalid for the new mode.
   updateSpectrumMode();
   saveSpectrumRange();
   if(previousMode!==selectedMode){
     lastSamples=[];peakPoints=[];lastSpectrumMode=selectedMode;
     try{sessionStorage.removeItem('atsSpectrumSnapshotV2');}catch(_){}
     draw([]);
     status.textContent='Spectrum reset for '+selectedMode+'. Set the range and scan again.';
     if(window.atsDebugLog) window.atsDebugLog('SPECTRUM RESET','mode changed '+previousMode+' -> '+selectedMode);
   }
   try{sessionStorage.setItem('atsSpectrumMode',selectedMode);}catch(_){}
   if(modeSwitchBusy) return;
   modeSwitchBusy=true;
   modeButtons.forEach(item=>item.disabled=true);
   status.textContent='Switching receiver to '+selectedMode+'…';
   // Non-FM modes use a safe shortwave starting frequency (3000 kHz).
   // FM keeps the receiver at 88 MHz until the scan is explicitly started.
   const rangeStart=Number(startInput.value);
   const targetHz=Math.round(rangeStart*(selectedMode==='FM'?1000000:1000));
   const ok=await postRadioCommand('/radio/tune',{freq:String(targetHz),mode:selectedMode},status);
   if(ok){
     saveSpectrumRange();
     status.textContent='Receiver switched to '+selectedMode+' and tuned to '+startInput.value+' '+(selectedMode==='FM'?'MHz':'kHz')+'. Press SCAN RANGE to scan this range.';
     setTimeout(()=>location.reload(),800);
   }else{
     modeSwitchBusy=false;
     modeButtons.forEach(item=>item.disabled=false);
   }
 }));
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
   if(ok){status.textContent='Tuned to '+(fmMode?(sample.f/100).toFixed(2)+' MHz':sample.f+' kHz')+'. Updating receiver display…';setTimeout(()=>location.reload(),700);}
 });
 
// Separate ALL Band scanner UI and state. It uses its own API and result buffer.
(function initAllBandScanner(){
 const form=document.getElementById('allBandSpectrumForm');
 const startInput=document.getElementById('allBandStart');
 const endInput=document.getElementById('allBandEnd');
 const modeInput=document.getElementById('allBandMode');
 const stepInput=document.getElementById('allBandStep');
 const button=document.getElementById('allBandStartScan');
 const status=document.getElementById('allBandStatus');
 const canvas=document.getElementById('allBandCanvas');
 if(!form||!canvas)return;
 const ctx=canvas.getContext('2d');
 let samples=[],peaks=[],scanBusy=false,plotMode='AM';
 function draw(data){
   samples=data||[];peaks=[];
   const dpr=window.devicePixelRatio||1,w=Math.max(260,canvas.clientWidth),h=230;
   canvas.width=Math.round(w*dpr);canvas.height=Math.round(h*dpr);ctx.setTransform(dpr,0,0,dpr,0,0);
   ctx.fillStyle='#050810';ctx.fillRect(0,0,w,h);
   const left=36,right=10,top=14,bottom=30,pw=w-left-right,ph=h-top-bottom;
   ctx.strokeStyle='#293958';ctx.lineWidth=1;ctx.fillStyle='#a5b9e1';ctx.font='10px system-ui';
   for(let i=0;i<5;i++){const y=top+ph*i/4;ctx.beginPath();ctx.moveTo(left,y);ctx.lineTo(w-right,y);ctx.stroke();}
   if(samples.length<2){ctx.fillStyle='#8ea2ae';ctx.font='12px system-ui';ctx.fillText('No ALL Band sweep data yet',left+8,top+22);return;}
   const vals=samples.map(p=>p.r),lo=Math.min(...vals),hi=Math.max(...vals),pad=Math.max(4,(hi-lo)*.15),minR=Math.max(0,lo-pad),maxR=hi+pad,span=Math.max(10,maxR-minR);
   for(let i=0;i<5;i++){const y=top+ph*i/4;ctx.fillStyle='#a5b9e1';ctx.fillText(String(Math.round(maxR-(maxR-minR)*i/4)),3,y+3);}
   ctx.fillStyle='#8ea2ae';ctx.fillText('RSSI',4,10);
   const pts=samples.map((p,i)=>({x:left+i*pw/(samples.length-1),y:top+ph-(p.r-minR)/span*ph,index:i}));
   ctx.beginPath();ctx.moveTo(pts[0].x,top+ph);pts.forEach(p=>ctx.lineTo(p.x,p.y));ctx.lineTo(pts[pts.length-1].x,top+ph);ctx.closePath();ctx.fillStyle='rgba(0,65,255,.22)';ctx.fill();
   ctx.beginPath();pts.forEach((p,i)=>i?ctx.lineTo(p.x,p.y):ctx.moveTo(p.x,p.y));ctx.strokeStyle='#167bff';ctx.lineWidth=2.5;ctx.shadowColor='#087cff';ctx.shadowBlur=8;ctx.stroke();ctx.shadowBlur=0;
   const threshold=lo+(hi-lo)*.22;
   samples.forEach((p,i)=>{if(i===0||i===samples.length-1||p.r<samples[i-1].r||p.r<=samples[i+1].r||p.r<threshold)return;const q=pts[i];peaks.push(q);ctx.beginPath();ctx.arc(q.x,q.y,4,0,Math.PI*2);ctx.fillStyle='#ff4c50';ctx.shadowColor='#ff334b';ctx.shadowBlur=12;ctx.fill();ctx.shadowBlur=0;ctx.strokeStyle='#ffd4dc';ctx.lineWidth=1;ctx.stroke();});
   ctx.fillStyle='#8ea2ae';ctx.font='10px monospace';ctx.textAlign='left';ctx.fillText(samples[0].f+' kHz',left,h-8);ctx.textAlign='right';ctx.fillText(samples[samples.length-1].f+' kHz',w-right,h-8);ctx.textAlign='center';ctx.fillStyle='#167bff';ctx.fillText('TAP RED PEAK TO TUNE',w/2,h-8);ctx.textAlign='left';
 }
 // Keep the receiver synchronized with the current ALL Band settings.
 // Empty frequency fields on page load and after every completed/failed scan
 // force a fresh, explicit range entry for the next sweep.
 let preTuneKey='';
 let tuneTimer=null;
 async function syncAllBandReceiver(){
   if(scanBusy)return false;
   const start=Number(startInput.value),end=Number(endInput.value),mode=modeInput.value;
   if(!startInput.value.trim()||!Number.isInteger(start)||start<150||start>30000)return false;
   const key=String(start)+'|'+mode;
   if(key===preTuneKey)return true;
   preTuneKey=key;
   status.textContent='Preparing receiver: '+start+' kHz '+mode+'…';
   if(window.atsDebugLog)window.atsDebugLog('ALL BAND PRE-TUNE','Settings changed: '+start+' kHz '+mode);
   try{
     const ok=await postRadioCommand('/radio/tune',{freq:String(start*1000),mode},status);
     if(ok)status.textContent='Receiver ready at '+start+' kHz '+mode+'.';
     else preTuneKey='';
     return !!ok;
   }catch(_){preTuneKey='';status.textContent='Could not prepare receiver at '+start+' kHz '+mode+'.';return false;}
 }
 function scheduleAllBandTune(){
   if(tuneTimer)clearTimeout(tuneTimer);
   preTuneKey='';
   tuneTimer=setTimeout(()=>{syncAllBandReceiver();},250);
 }
 startInput.addEventListener('change',scheduleAllBandTune);
 endInput.addEventListener('change',scheduleAllBandTune);
 modeInput.addEventListener('change',scheduleAllBandTune);
 stepInput.addEventListener('change',scheduleAllBandTune);
 form.addEventListener('submit',async event=>{
   event.preventDefault();if(scanBusy)return;
   const start=Number(startInput.value),end=Number(endInput.value),step=Number(stepInput.value),mode=modeInput.value;
   const points=Math.floor((end-start)/step)+1;
   if(!Number.isInteger(start)||!Number.isInteger(end)||start<150||end>30000||end<=start){status.textContent='Enter a valid range between 150 and 30000 kHz.';return;}
   if(end-start>1000||(end-start)%step!==0||points<6||points>201){status.textContent=points<6?'Scan range too small. Choose at least 5 measurement steps (6 points).':'Range must be at most 1000 kHz wide and align exactly to the selected step (maximum 201 points).';return;}
   status.textContent='Preparing receiver at '+start+' kHz '+mode+' before scan…';
   const tuned=await syncAllBandReceiver();
   if(!tuned){status.textContent='Could not tune receiver to '+start+' kHz '+mode+'. Scan not started.';return;}
   scanBusy=true;button.disabled=true;button.textContent='⏳ SCAN RUNNING…';startInput.disabled=endInput.disabled=modeInput.disabled=stepInput.disabled=true;draw([]);status.textContent='⏳ ALL BAND SCAN RUNNING — PLEASE WAIT · Initializing receiver…';
   try{
     const body=new URLSearchParams({start_khz:String(start),end_khz:String(end),step_khz:String(step),mode});
     const response=await fetch('/all-spectrum/start',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body,cache:'no-store'});
     const result=await response.json().catch(()=>({}));
     if(!response.ok)throw new Error(result.error||'ALL Band scan could not be started.');
     let completed=false;
     for(let i=0;i<100;i++){
       await new Promise(resolve=>setTimeout(resolve,1000));
       const res=await fetch('/all-spectrum.json',{cache:'no-store'});const data=await res.json();
       if(data.ready&&Array.isArray(data.samples)&&data.samples.length){plotMode=data.mode||mode;draw(data.samples);status.textContent='ALL Band sweep complete · '+data.samples.length+' samples · '+plotMode+'. Tap a red peak to tune.';completed=true;break;}
       status.textContent='⏳ ALL BAND SCAN RUNNING — PLEASE WAIT · Measuring signal… '+(i+1)+' s';
     }
     if(!completed)throw new Error('ALL Band sweep timed out. Try a narrower range or larger step.');
   }catch(error){status.textContent=error.message||'ALL Band scan failed.';}
   finally{scanBusy=false;button.disabled=false;button.textContent='▶ SCAN ALL BAND';startInput.disabled=endInput.disabled=modeInput.disabled=stepInput.disabled=false;startInput.value='';endInput.value='';preTuneKey='';}
 });
 canvas.addEventListener('click',async event=>{
   if(!peaks.length||!samples.length||scanBusy)return;
   const rect=canvas.getBoundingClientRect(),scaleX=canvas.width/(window.devicePixelRatio||1)/rect.width,x=(event.clientX-rect.left)*scaleX;
   let near=null,distance=Infinity;peaks.forEach(p=>{const d=Math.abs(p.x-x);if(d<distance){distance=d;near=p;}});
   if(!near||distance>22){status.textContent='Tap one of the red peaks to tune to that frequency.';return;}
   const p=samples[near.index];status.textContent='Tuning to '+p.f+' kHz '+plotMode+'…';
   try{const ok=await postRadioCommand('/radio/tune',{freq:String(Math.round(p.f*1000)),mode:plotMode},status);if(ok){status.textContent='Tuned to '+p.f+' kHz '+plotMode+'. Spectrum peaks remain visible; scan again when you want fresh measurements.';}}
   catch(_){status.textContent='Could not tune to selected peak.';}
 });
 draw([]);window.addEventListener('resize',()=>draw(samples));
})();

 let restored=false;
 try{
   const saved=JSON.parse(sessionStorage.getItem('atsSpectrumSnapshotV2')||'null');
   if(saved&&saved.fm===true&&Array.isArray(saved.samples)&&saved.samples.length>1){
     draw(saved.samples,true);
     lastSpectrumMode='FM';
     status.textContent='Previous FM sweep restored · '+saved.samples.length+' samples. Scan again whenever you want fresh measurements.';
     restored=true;
   }
 }catch(_){}
 if(!restored)draw([]);
 window.addEventListener('resize',()=>draw(lastSamples));
})();
</SCRIPT>
)HTML";

#endif // PAGE_STATUS_H
