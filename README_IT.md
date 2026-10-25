# ATS Mini V4 – Firmware di 13HN2709

<p align="center">
  <img src="docs/source/_static/20261023_043409.jpg" alt="ATS Mini V4 con il firmware di 13HN2709" width="360">
</p>

<p align="center">
  <strong>Sviluppo personalizzato del firmware e dell'interfaccia per ATS Mini V4 (SI4732 / ESP32-S3)</strong><br>
  A cura di <a href="https://github.com/13HN2709">13HN2709</a> · Nominativo radioamatoriale: 13HN2709
</p>

Questo repository contiene il mio fork di sviluppo del firmware ATS Mini per il ricevitore ATS Mini V4 basato su SI4732. Il progetto combina il firmware originale del ricevitore con layout personalizzati per il display, miglioramenti nell'utilizzo e funzioni di controllo via web.

## Download

**Release firmware di questo fork:**  
➡️ **[Apri le release di 13HN2709](https://github.com/13HN2709/ats-mini/releases)**

Nella pagina Releases sono disponibili i pacchetti firmware pubblicati. Scegli la configurazione della memoria flash adatta al tuo dispositivo (OSPI o QSPI) e la lingua (DE o EN). I file OTA sono destinati alle procedure di aggiornamento over-the-air supportate. I pacchetti ZIP contengono i file di release/codice sorgente forniti per quella build.

**Ultima release attualmente elencata:** [v1.22](https://github.com/13HN2709/ats-mini/releases/tag/v1.22). Prima dell'installazione, controlla le note della release e i file disponibili. Le nuove funzioni ancora in sviluppo potrebbero non essere incluse in una release pubblicata.

## Caratteristiche principali

- **Diversi layout del display**, tra cui Clean e un design ispirato al TEF6686.
- **Tastierino per la frequenza e miglioramenti della sintonia** per inserire direttamente una frequenza.
- **Assistente bandplan** nei layout supportati, con indicazioni sulle attività e modalità operative più comuni.
- **Controllo web locale integrato**, utilizzabile sulla rete locale del ricevitore senza connessione Internet.
- **Funzioni Spectrum Scanner** e miglioramenti dei controlli del ricevitore, in base alla versione del firmware.
- **Interfaccia in tedesco e inglese** nei menu e nei controlli supportati.

## Funzioni aggiuntive sviluppate da 13HN2709

Le seguenti funzioni e personalizzazioni dell'interfaccia fanno parte del mio lavoro di sviluppo su questo fork e ampliano il progetto originale. La disponibilità esatta può variare in base alla versione del firmware:

- **Frequency Keypad** — inserimento diretto della frequenza desiderata tramite tastierino.
- **VFO A/B** — trasferimento delle frequenze tra VFO A e VFO B o passaggio da uno all'altro, dove supportato dalla build.
- **Frequency Step Control nel Web Control Center** — modifica del passo di sintonia direttamente dall'interfaccia web.
- **Interfaccia bilingue (tedesco/inglese)** — menu tradotti e brevi descrizioni delle impostazioni principali.
- **BFO integrato** — regolazione fine della frequenza per le modalità di ricezione che utilizzano il beat frequency oscillator, in particolare SSB.
- **Spectrum Scanner ampliato** — scansione di un intervallo di frequenze con passi e modalità selezionabili, selezione dei picchi rilevati e sintonia sulla frequenza scelta; alcune versioni includono anche funzioni di memoria.
- **Gestione delle memorie via web** — visualizzazione e selezione delle memorie salvate e utilizzo delle funzioni di salvataggio supportate.
- **Web Control Center locale indipendente** — controllo del ricevitore tramite la rete locale del dispositivo, senza bisogno di Internet.
- **Layout personalizzati del display** — tra cui un Clean Layout separato per sviluppo e test e un design ispirato al TEF6686.
- **Assistente bandplan** — suggerimenti operativi in base alla frequenza nei layout supportati, con etichette brevi in inglese sul display.
- **Dual RX e miglioramenti dei controlli** — ulteriori comportamenti di scansione/controllo e modifiche al display nelle build che li includono.

Questo elenco descrive le aggiunte specifiche di questo fork e non significa che ogni funzione sia presente in tutte le release pubblicate. Controlla le note e i file della versione scelta.

## Stato dello sviluppo

- `main` è il branch stabile delle release.
- `development` è usato per lo sviluppo e i test in corso.
- I branch temporanei dedicati alle funzioni vengono eliminati dopo l'integrazione delle modifiche testate.

Per lo stato aggiornato, la strategia dei branch e i prossimi passi, consulta [PROJECT_STATUS.md](PROJECT_STATUS.md).

## Documentazione

Per informazioni generali sull'hardware e la documentazione del progetto originale, consulta la [documentazione ATS Mini](https://esp32-si4732.github.io/ats-mini/).

## Ringraziamenti agli sviluppatori e ai progetti originali

Questo firmware si basa sull'ecosistema del ricevitore ATS Mini / SI4732 e sul lavoro dei manutentori del progetto originale e dei collaboratori open source. Un grande grazie a tutti gli sviluppatori il cui lavoro rende possibile questo progetto.

Progetti e librerie correlati:

- [ATS Mini – progetto upstream originale](https://github.com/esp32-si4732/ats-mini)
- [Max Arnold – sviluppo del firmware ATS Mini](https://github.com/G8PTN/ATS_MINI)
- [Libreria PU2CLR SI4735](https://github.com/pu2clr/SI4735)
- [Volos Projects – TEmbedFMRadio](https://github.com/VolosR/TEmbedFMRadio)
- [Ralph Xavier – SI4735](https://github.com/ralphxavier/SI4735)
- [Goshante – ATS20/ATS-EX](https://github.com/goshante/ats20_ats_ex)

Questo è un fork comunitario del firmware. Le licenze originali e le attribuzioni richieste per il codice upstream e le librerie di terze parti devono essere mantenute.

## Contatti e progetto

- **Maintainer:** [13HN2709 su GitHub](https://github.com/13HN2709)
- **Release firmware:** [13HN2709/ats-mini/releases](https://github.com/13HN2709/ats-mini/releases)
- **Codice sorgente:** [13HN2709/ats-mini](https://github.com/13HN2709/ats-mini)
