#ifndef PAGE_STATUS_H
#define PAGE_STATUS_H

#include <Arduino.h>

static const char pageStatusTitle[] PROGMEM = "ATS-Mini Status";

static const char pageStatus[] PROGMEM = R"HTML(<DIV CLASS="TOP">
  <DIV CLASS="BRAND"><DIV CLASS="LOGO">&#9673;</DIV><DIV><H1>ATS MINI <SPAN STYLE="color:#56f29a">CONTROLLER</SPAN></H1><DIV CLASS="SUB">Local radio control center</DIV></DIV></DIV>
  <SPAN CLASS="PILL">&#9679; {{connection}}</SPAN>
</DIV>
{{{navigation}}}
<SECTION CLASS="HERO">
  <DIV CLASS="EYEBROW">Now receiving</DIV>
  <DIV CLASS="FREQ">{{frequency}}</DIV>
  <DIV CLASS="MODE">{{band}} &nbsp;·&nbsp; {{mode}}</DIV>
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
<SCRIPT>setTimeout(()=>location.reload(),10000);</SCRIPT>
)HTML";

#endif // PAGE_STATUS_H
