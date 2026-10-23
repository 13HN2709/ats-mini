#ifndef PAGE_COMMON_H
#define PAGE_COMMON_H

#include "PageTemplate.h"

static const char pageStart[] PROGMEM = R"HTML(<!DOCTYPE HTML>
<HTML>
<HEAD>
  <META CHARSET="UTF-8">
  <META NAME="viewport" CONTENT="width=device-width, initial-scale=1.0">
  <META NAME="theme-color" CONTENT="#080d12">
  <TITLE>{{title}}</TITLE>
  <STYLE>
    :root { color-scheme:dark; --bg:#020611; --panel:#030d27; --panel2:#06163b; --line:#123a78; --text:#e7f1ff; --muted:#7192c6; --blue:#167bff; --green:#35e8ff; --cyan:#45dfff; }
    * { box-sizing:border-box; }
    body { margin:0; padding:0; background:linear-gradient(180deg,#020611 0%,#01030a 70%,#000 100%); color:var(--text); font:16px/1.45 "Arial Narrow", "Roboto Condensed", system-ui, sans-serif; letter-spacing:.015em; }
    a { color:var(--blue); text-decoration:none; }
    a:hover { color:var(--cyan); }
    .APP { max-width:1040px; margin:0 auto; padding:16px 12px 28px; }
    .TOP { display:flex; align-items:center; justify-content:space-between; gap:12px; margin:4px 0 18px; }
    .BRAND { display:flex; align-items:center; gap:12px; }
    .LOGO { width:44px; height:44px; display:grid; place-items:center; border-radius:0; background:#020a1c; color:var(--blue); border:1px solid #1454ad; box-shadow:inset 0 0 16px #004dff22; font-size:22px; }
    .BRAND h1 { margin:0; font-size:1.12rem; letter-spacing:.06em; }
    .SUB { color:var(--muted); font-size:.78rem; margin-top:2px; }
    .PILL { border:1px solid #16458f; background:#030d25; color:var(--cyan); border-radius:0; padding:6px 10px; font-size:.76rem; white-space:nowrap; }
    .NAV { display:grid; grid-template-columns:repeat(3,minmax(0,1fr)); gap:8px; margin:0 0 16px; }
    .NAV .ACTIVE { font-weight:800; text-shadow:0 0 10px #80a0ff77; }
    .NAV a { flex:1 1 auto; text-align:center; padding:10px 13px; border:1px solid var(--line); border-radius:0; background:#030b1d; color:#dce5ff; font-size:.88rem; }
    .NAV a.ACTIVE { border-color:var(--blue); background:#061d4b; color:var(--cyan); box-shadow:inset 0 -2px 0 var(--blue); }
    .HERO { background:linear-gradient(115deg,#04112d,#020713 72%); border:1px solid #1557b5; border-radius:0; padding:22px 18px; box-shadow:inset 0 0 28px #004dff18,0 12px 30px #000; margin-bottom:14px; position:relative; overflow:hidden; }
    .EYEBROW { color:var(--blue); font-size:.74rem; font-weight:750; letter-spacing:.16em; text-transform:uppercase; }
    .STEREO { color:#ff453e; font-weight:900; letter-spacing:.16em; text-shadow:0 0 10px #ff453e99,0 0 22px #ff453e55; }
    .FREQ { font-family:"DSEG7 Classic","Digital-7","DS-Digital",monospace; font-size:clamp(2.55rem,11vw,5.1rem); line-height:1.08; font-weight:700; letter-spacing:.025em; font-variant-numeric:tabular-nums; color:#2585ff; text-shadow:0 0 4px #167bff,0 0 18px #075dff99,0 0 34px #075dff55; background:#01040b; border:1px solid #0b2d6a; padding:12px 10px; margin:14px 0 10px; margin:12px 0 8px; overflow-wrap:anywhere; }
    .STEREO { display:inline-block; margin-left:8px; padding:2px 8px; color:#ff4a45; border:1px solid #a52636; background:#19040a; border-radius:0; font-family:monospace; font-weight:900; letter-spacing:.12em; text-shadow:0 0 8px #ff302b88; }
    .MODE { color:#d5e7ed; font-size:1rem; font-weight:650; }
    .GRID { display:grid; grid-template-columns:repeat(2,minmax(0,1fr)); gap:10px; margin:14px 0; }
    .STAT { min-width:0; padding:14px; border:1px solid #10356e; border-radius:0; background:linear-gradient(145deg,#04112b,#020712); box-shadow:inset 0 0 18px #003b9b12; }
    .STAT .LABEL { display:block; color:var(--muted); font-size:.76rem; margin-bottom:5px; }
    .STAT .VALUE { display:block; font-size:1.08rem; font-weight:700; overflow-wrap:anywhere; font-variant-numeric:tabular-nums; }
    .SECTION { margin:20px 0 10px; font-size:.78rem; color:var(--muted); font-weight:750; letter-spacing:.14em; text-transform:uppercase; }
    .ACTIONS { display:grid; grid-template-columns:repeat(2,minmax(0,1fr)); gap:10px; }
    .ACTION { display:flex; align-items:center; gap:12px; min-height:72px; padding:14px; border:1px solid #123b7a; border-radius:0; background:linear-gradient(110deg,#061638,#020712); color:var(--text); }
    .ACTION:hover { border-color:var(--blue); color:var(--text); transform:translateY(-1px); }
    .ICON { width:38px; height:38px; flex:0 0 38px; display:grid; place-items:center; border-radius:0; background:#041b48; color:var(--blue); border:1px solid #16458f; font-size:1.1rem; }
    .ACTION strong { display:block; font-size:.92rem; }
    .ACTION small { display:block; margin-top:3px; color:var(--muted); font-size:.75rem; }
    TABLE { width:100%; max-width:768px; border:0; margin-left:auto; margin-right:auto; }
    TH, TD { padding:.5em; }
    .HEADING { background:#182b4c; border:1px solid #80a0ff; color:#a9c3ff; column-span:all; text-align:center; font-weight:800; letter-spacing:.08em; text-shadow:0 0 10px #80a0ff77; }
    TD.LABEL { text-align:right; }
    INPUT[type=text], INPUT[type=password], INPUT[type=file], INPUT[type=number], SELECT, TEXTAREA { max-width:100%; padding:.65em; border:1px solid #16458f; border-radius:0; background:#010713; color:var(--text); font:inherit; } INPUT[type=text], INPUT[type=password], SELECT, TEXTAREA { width:95%; } INPUT:focus, SELECT:focus, TEXTAREA:focus { outline:1px solid var(--blue); border-color:var(--blue); }
    .LED-FREQ { display:flex; align-items:flex-end; justify-content:center; gap:clamp(2px,1vw,8px); min-height:92px; }
    .LED-DIGIT { display:block; width:clamp(22px,8vw,58px); height:auto; flex:0 1 auto; filter:drop-shadow(0 0 5px #075dff66); }
    .LED-DOT { display:block; width:clamp(7px,2vw,14px); height:auto; flex:0 0 auto; margin:0 clamp(0px,.2vw,2px) 0 0; }
    .LED-UNIT { margin:0 0 5px 4px; color:#3b8fff; font:700 clamp(.7rem,2vw,1rem) monospace; letter-spacing:.08em; text-shadow:0 0 10px #075dff88; }
    .HERO:before { content:""; position:absolute; left:0; top:0; width:4px; height:100%; background:var(--blue); box-shadow:0 0 18px var(--blue); }
    .TOP { border-bottom:1px solid #10356e; padding-bottom:12px; }
    .NAV { grid-template-columns:repeat(5,minmax(0,1fr)); gap:0; border:1px solid #10356e; }
    .NAV a, .NAV .ACTIVE { border:0; border-right:1px solid #10356e; padding:12px 4px; }
    .NAV a:last-child { border-right:0; }
    .STAT .LABEL { letter-spacing:.12em; text-transform:uppercase; }
    .SPECTRUM-PANEL { margin:16px 0; padding:16px; border:1px solid #1454ad; background:linear-gradient(180deg,#030d25,#01040b); box-shadow:inset 0 0 28px #004dff12; }
    .PANEL-HEAD { display:flex; align-items:center; justify-content:space-between; gap:12px; flex-wrap:wrap; border-bottom:1px solid #10356e; padding-bottom:12px; }
    .PANEL-HEAD H2 { margin:4px 0; color:#dbeaff; font-size:1.2rem; letter-spacing:.12em; }
    .LIVE-MARK { color:#ff5968; font:700 .7rem monospace; letter-spacing:.12em; }
    .LIVE-MARK SPAN { display:inline-block; width:7px; height:7px; background:#ff4057; border-radius:50%; box-shadow:0 0 10px #ff4057; margin-right:5px; }
    .RANGE-FORM { display:grid; grid-template-columns:1fr 1fr; gap:9px; align-items:end; margin:14px 0; }
    .RANGE-FORM BUTTON { grid-column:1/-1; }
    .RANGE-FORM SELECT { font-size:.85rem; }
    .RANGE-FORM LABEL { display:block; color:var(--muted); font:700 .68rem monospace; letter-spacing:.12em; }
    .RANGE-FORM INPUT { display:block; width:100%; min-width:0; margin-top:6px; font-family:monospace; font-size:1rem; color:#7bb5ff; }
    .RANGE-FORM BUTTON { min-height:42px; white-space:nowrap; }
    .SPECTRUM-FRAME { border:1px solid #123a78; background:#00040c; padding:8px; }
    .SPECTRUM-META { display:flex; justify-content:space-between; gap:8px; color:#4c84d5; font: .6rem monospace; letter-spacing:.08em; }
    .SCAN-STATUS { color:#8ba9d8; font-size:.78rem; margin-top:10px; }
    @media (max-width:480px) { .RANGE-FORM { grid-template-columns:1fr 1fr; } .RANGE-FORM BUTTON { grid-column:1/-1; } .SPECTRUM-PANEL { padding:10px; } .NAV a,.NAV .ACTIVE { font-size:.72rem; } }
    .CONFIGFORM TABLE { border-collapse:separate; border-spacing:0 5px; }
    .CONFIGFORM TH, .CONFIGFORM TD { padding:.55em .6em; }
    .CONFIGFORM TD.LABEL { width:42%; color:var(--muted); }
    .CONFIGFORM INPUT[type=text], .CONFIGFORM INPUT[type=password], .CONFIGFORM INPUT[type=file], .CONFIGFORM SELECT, .CONFIGFORM TEXTAREA { width:100%; }
    .CONFIGFORM INPUT[type=submit] { width:min(100%,320px); }
    @media (max-width:480px) { .CONFIGFORM TD.LABEL { width:38%; } .CONFIGFORM TH, .CONFIGFORM TD { padding:.45em .3em; } }
    INPUT[type=submit], BUTTON { padding:.7em 1em; border:1px solid #167bff; border-radius:0; background:linear-gradient(90deg,#07337f,#04102d); color:var(--blue); font:inherit; font-weight:700; letter-spacing:.055em; cursor:pointer; }
    INPUT[type=submit]:hover, BUTTON:hover { background:#263d67; border-color:var(--cyan); }
    BUTTON:focus-visible, INPUT[type=submit]:focus-visible, A:focus-visible { outline:2px solid var(--cyan); outline-offset:2px; }
    BUTTON:disabled { opacity:.55; cursor:wait; }
    INPUT[type=checkbox], INPUT[type=radio], INPUT[type=range] { accent-color:var(--blue); }
    INPUT[type=checkbox], INPUT[type=radio] { width:1.15em; height:1.15em; vertical-align:middle; }
    INPUT[type=submit] { width:50%; }
    .CENTER { text-align:center; }
    .HEAP { color:#71838d; font-size:.72em; text-align:center; margin:2em .5em 0; }
    @media (min-width:640px) { .GRID { grid-template-columns:repeat(4,minmax(0,1fr)); } .ACTIONS { grid-template-columns:repeat(3,minmax(0,1fr)); } .HERO { padding:28px; } }
    @media (max-width:380px) { .APP { padding:12px 10px 22px; } .HERO { padding:16px; } .GRID,.ACTIONS { gap:8px; } .STAT { padding:11px; } .ACTION { padding:11px; gap:8px; } .PILL { font-size:.68rem; } }
  </STYLE>
</HEAD>
<BODY>
<DIV CLASS="APP">
)HTML";

static const char pageEnd[] PROGMEM = R"HTML(
<FOOTER CLASS="HEAP">
  Internal heap: {{heap_used}} KiB used / {{heap_free}} KiB free &middot;
  PSRAM: {{psram_used}} KiB used / {{psram_free}} KiB free (at page render)
</FOOTER>
</DIV>
</BODY>
</HTML>
)HTML";

static void webNavigation(String &out, const char *activePage)
{
  static const struct { const char *name; const char *path; } pages[] =
  {
    {"Radio", "/"},
    {"Memory", "/memory"},
    {"Config", "/config"},
    {"Update", "/update"},
    {"Patches", "/patches"},
  };
  out += R"HTML(<NAV CLASS="NAV">)HTML";
  for(size_t i = 0; i < sizeof(pages) / sizeof(pages[0]); i++)
  {
    if(i) out += "";
    if(!strcmp(activePage, pages[i].path)) pageAppend(out, R"HTML(<SPAN CLASS="ACTIVE" STYLE="flex:1 1 auto;text-align:center;padding:10px 13px;border:1px solid #80a0ff;border-radius:8px;background:#1a2946;color:#80a0ff;font-size:.88rem;font-weight:800">{{label}}</SPAN>)HTML", {{"label", pages[i].name}});
    else pageAppend(out, R"HTML(<A HREF="{{path}}">{{label}}</A>)HTML", {{"path", pages[i].path}, {"label", pages[i].name}});
  }
  out += "</NAV>";
}

#endif // PAGE_COMMON_H
