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
    :root { color-scheme: dark; --bg:#080d12; --panel:#111a22; --panel2:#17232d; --line:#263642; --text:#e8f1f5; --muted:#8ea2ae; --green:#56f29a; --cyan:#45d9ee; }
    * { box-sizing:border-box; }
    body { margin:0; padding:0; background:radial-gradient(ellipse at top,#14242d 0%,var(--bg) 58%); color:var(--text); font:16px/1.45 system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif; }
    a { color:var(--cyan); text-decoration:none; }
    a:hover { color:var(--green); }
    .APP { max-width:920px; margin:0 auto; padding:18px 14px 28px; }
    .TOP { display:flex; align-items:center; justify-content:space-between; gap:12px; margin:4px 0 18px; }
    .BRAND { display:flex; align-items:center; gap:12px; }
    .LOGO { width:44px; height:44px; display:grid; place-items:center; border-radius:14px; background:#102b27; color:var(--green); border:1px solid #245943; font-size:22px; }
    .BRAND h1 { margin:0; font-size:1.12rem; letter-spacing:.06em; }
    .SUB { color:var(--muted); font-size:.78rem; margin-top:2px; }
    .PILL { border:1px solid #285541; background:#10271f; color:var(--green); border-radius:999px; padding:6px 10px; font-size:.76rem; white-space:nowrap; }
    .NAV { display:flex; flex-wrap:wrap; gap:8px; margin:0 0 16px; }
    .NAV a { flex:1 1 auto; text-align:center; padding:10px 13px; border:1px solid var(--line); border-radius:12px; background:#101820; color:#c6d6df; font-size:.88rem; }
    .NAV a.ACTIVE { border-color:#286347; background:#10271f; color:var(--green); }
    .HERO { background:linear-gradient(140deg,#101d25,#080d12 75%); border:1px solid #28404a; border-radius:22px; padding:20px; box-shadow:0 18px 44px #0005; margin-bottom:14px; }
    .EYEBROW { color:var(--green); font-size:.74rem; font-weight:750; letter-spacing:.16em; text-transform:uppercase; }
    .FREQ { font-size:clamp(2.35rem,10vw,4.5rem); line-height:1.05; font-weight:800; letter-spacing:-.055em; font-variant-numeric:tabular-nums; color:var(--green); margin:12px 0 8px; overflow-wrap:anywhere; }
    .MODE { color:#d5e7ed; font-size:1rem; font-weight:650; }
    .GRID { display:grid; grid-template-columns:repeat(2,minmax(0,1fr)); gap:10px; margin:14px 0; }
    .STAT { min-width:0; padding:14px; border:1px solid var(--line); border-radius:15px; background:#111a22; }
    .STAT .LABEL { display:block; color:var(--muted); font-size:.76rem; margin-bottom:5px; }
    .STAT .VALUE { display:block; font-size:1.08rem; font-weight:700; overflow-wrap:anywhere; font-variant-numeric:tabular-nums; }
    .SECTION { margin:20px 0 10px; font-size:.78rem; color:var(--muted); font-weight:750; letter-spacing:.14em; text-transform:uppercase; }
    .ACTIONS { display:grid; grid-template-columns:repeat(2,minmax(0,1fr)); gap:10px; }
    .ACTION { display:flex; align-items:center; gap:12px; min-height:72px; padding:14px; border:1px solid var(--line); border-radius:16px; background:linear-gradient(135deg,#17242c,#10171e); color:var(--text); }
    .ACTION:hover { border-color:#3d8866; color:var(--text); transform:translateY(-1px); }
    .ICON { width:38px; height:38px; flex:0 0 38px; display:grid; place-items:center; border-radius:12px; background:#20312f; color:var(--green); font-size:1.1rem; }
    .ACTION strong { display:block; font-size:.92rem; }
    .ACTION small { display:block; margin-top:3px; color:var(--muted); font-size:.75rem; }
    TABLE { width:100%; max-width:768px; border:0; margin-left:auto; margin-right:auto; }
    TH, TD { padding:.5em; }
    .HEADING { background:#80a0ff; column-span:all; text-align:center; }
    TD.LABEL { text-align:right; }
    INPUT[type=text], INPUT[type=password], SELECT { width:95%; padding:.65em; border:1px solid var(--line); border-radius:9px; background:#0b1218; color:var(--text); }
    INPUT[type=submit], BUTTON { padding:.7em 1em; border:1px solid #32694e; border-radius:10px; background:#153528; color:var(--green); font:inherit; cursor:pointer; }
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
    if(i) out += "&nbsp;|&nbsp;";
    if(!strcmp(activePage, pages[i].path)) pageAppend(out, R"HTML(<SPAN CLASS="ACTIVE" STYLE="flex:1 1 auto;text-align:center;padding:10px 13px;border:1px solid #286347;border-radius:12px;background:#10271f;color:#56f29a;font-size:.88rem">{{label}}</SPAN>)HTML", {{"label", pages[i].name}});
    else pageAppend(out, R"HTML(<A HREF="{{path}}">{{label}}</A>)HTML", {{"path", pages[i].path}, {"label", pages[i].name}});
  }
  out += "</NAV>";
}

#endif // PAGE_COMMON_H
