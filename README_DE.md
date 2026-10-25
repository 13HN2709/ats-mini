# ATS Mini V4 – Firmware von 13HN2709

<p align="center">
  <img src="docs/source/_static/20261023_043409.jpg" alt="ATS Mini V4 mit der Firmware von 13HN2709" width="360">
</p>

<p align="center">
  <strong>Eigene Firmware- und Oberflächenentwicklung für den ATS Mini V4 (SI4732 / ESP32-S3)</strong><br>
  Von <a href="https://github.com/13HN2709">13HN2709</a> · Rufzeichen: 13HN2709
</p>

Dieses Repository enthält meinen Entwicklungs-Fork der ATS-Mini-Firmware für den SI4732-basierten ATS Mini V4. Es verbindet die ursprüngliche Empfänger-Firmware mit eigenen Display-Layouts, Bedienungsverbesserungen und Web-Control-Funktionen.

## Downloads

**Firmware-Releases dieses Forks:**  
➡️ **[Zu den Releases von 13HN2709](https://github.com/13HN2709/ats-mini/releases)**

Auf der Releases-Seite findest du die veröffentlichten Firmware-Pakete. Wähle die passende Flash-Konfiguration deines Geräts (OSPI oder QSPI) und die Sprache (DE oder EN). OTA-Dateien sind für unterstützte Over-the-Air-Updates vorgesehen. ZIP-Pakete enthalten die jeweils bereitgestellten Release-/Quelldateien.

**Aktuell aufgeführtes Release:** [v1.22](https://github.com/13HN2709/ats-mini/releases/tag/v1.22). Bitte prüfe vor der Installation die Release-Hinweise und verfügbaren Dateien. Neue Funktionen aus der Entwicklung müssen noch nicht in einem veröffentlichten Release enthalten sein.

## Projekt-Highlights

- **Mehrere Display-Layouts**, darunter Clean und ein vom TEF6686-Design inspiriertes Layout.
- **Frequenz-Keypad und Verbesserungen beim Abstimmen** für eine direkte Frequenzeingabe.
- **Bandplan-Assistent** in unterstützten Layouts mit Hinweisen auf typische Betriebsarten und Aktivitäten.
- **Integrierte lokale Websteuerung**, die im lokalen Netzwerk des Empfängers ohne Internetzugang funktioniert.
- **Spectrum-Scan-Funktionen** und Erweiterungen der Empfängersteuerung, abhängig von der Firmware-Version.
- **Deutsche und englische Benutzeroberfläche** in unterstützten Menüs und Bedienelementen.

## Eigene Erweiterungen von 13HN2709

Die folgenden Funktionen und Oberflächenanpassungen sind Teil meiner eigenen Entwicklungsarbeit an diesem Fork und erweitern das ursprüngliche Projekt. Die genaue Verfügbarkeit kann je nach Firmware-Version variieren:

- **Frequency Keypad** – Zielfrequenz direkt über ein Tastenfeld eingeben.
- **VFO A/B** – Frequenzen zwischen VFO A und VFO B übernehmen beziehungsweise zwischen ihnen wechseln, sofern im Build unterstützt.
- **Frequency Step Control im Web Control Center** – die Frequenzschrittweite direkt über die Weboberfläche ändern.
- **Zweisprachige Oberfläche (Deutsch/Englisch)** – übersetzte Menüs und kurze Beschreibungen wichtiger Einstellungen.
- **Integrierter BFO** – Feinabstimmung für Empfangsarten mit Beat Frequency Oscillator, insbesondere SSB.
- **Erweiterter Spectrum Scanner** – einen Frequenzbereich mit auswählbaren Schrittweiten und Betriebsarten absuchen, erkannte Peaks auswählen und auf die Frequenz abstimmen; je nach Version auch mit Speicherfunktionen.
- **Speichersteuerung über das Webinterface** – gespeicherte Speicherplätze anzeigen und auswählen sowie unterstützte Speicherfunktionen nutzen.
- **Eigenständiges lokales Web Control Center** – Empfängersteuerung über das lokale Gerätenetzwerk, ohne Internetverbindung.
- **Eigene Display-Layouts** – darunter ein separates Clean Layout für Entwicklung und Tests sowie ein TEF6686-inspiriertes Design.
- **Bandplan-Assistent** – frequenzabhängige Betriebshinweise in unterstützten Layouts, mit kurzen englischen Anzeigen auf dem Display.
- **Dual RX und Verbesserungen der Bedienung** – zusätzliche Scan-/Steuerungsabläufe und Display-Anpassungen in entsprechenden Builds.

Diese Liste beschreibt Erweiterungen dieses Forks und bedeutet nicht, dass jede Funktion in jedem veröffentlichten Release enthalten ist. Bitte prüfe die Hinweise und Dateien der jeweiligen Version.

## Entwicklungsstatus

- `main` ist der stabile Release-Branch.
- `development` wird für laufende Entwicklung und Tests verwendet.
- Temporäre Feature-Branches werden nach erfolgreicher Übernahme der getesteten Änderungen wieder entfernt.

Den aktuellen Stand, die Branch-Strategie und die nächsten Schritte findest du in [PROJECT_STATUS.md](PROJECT_STATUS.md).

## Dokumentation

Allgemeine Informationen zur Hardware und die Dokumentation des ursprünglichen Projekts findest du in der [ATS-Mini-Dokumentation](https://esp32-si4732.github.io/ats-mini/).

## Dank an die Entwickler und Upstream-Projekte

Diese Firmware baut auf dem ATS-Mini-/SI4732-Empfänger-Ökosystem und der Arbeit der ursprünglichen Projektbetreuer sowie Open-Source-Mitwirkenden auf. Vielen Dank an alle Entwickler, deren Arbeit dieses Projekt möglich macht.

Verwandte Projekte und Bibliotheken:

- [ATS Mini – ursprüngliches Upstream-Projekt](https://github.com/esp32-si4732/ats-mini)
- [Max Arnold – ATS-Mini-Firmware-Arbeiten](https://github.com/G8PTN/ATS_MINI)
- [PU2CLR SI4735 Library](https://github.com/pu2clr/SI4735)
- [Volos Projects – TEmbedFMRadio](https://github.com/VolosR/TEmbedFMRadio)
- [Ralph Xavier – SI4735](https://github.com/ralphxavier/SI4735)
- [Goshante – ATS20/ATS-EX](https://github.com/goshante/ats20_ats_ex)

Dies ist ein Community-Firmware-Fork. Die ursprünglichen Lizenzen und erforderlichen Namensnennungen des Upstream-Codes und der verwendeten Bibliotheken müssen erhalten bleiben.

## Kontakt und Projekt

- **Maintainer:** [13HN2709 auf GitHub](https://github.com/13HN2709)
- **Firmware-Releases:** [13HN2709/ats-mini/releases](https://github.com/13HN2709/ats-mini/releases)
- **Quellcode:** [13HN2709/ats-mini](https://github.com/13HN2709/ats-mini)
