
  // === KASTEN 3: Unten (Wasserfall mit weißer Trennlinie oben & 5px rotem Rand) ===
  int wfX = 10;
  int wfY = 121;
  int wfW = 300;
  int wfH = 44;
  
  spr.drawRect(wfX, wfY, wfW, wfH, 0xFFFF);

  // Weiße Trennlinie direkt bündig am inneren oberen Rand des Wasserfalls
  spr.drawFastHLine(wfX + 1, wfY + 1, wfW - 2, 0xFFFF);

  int wfInnerW = wfW - 2;
  int wfInnerH = wfH - 3; 

  if (!wfInitialized || wfSpr.width() != wfInnerW || wfSpr.height() != wfInnerH) {
    if (wfInitialized) {
      wfSpr.deleteSprite();
    }
    wfSpr.setPsram(true);
    wfSpr.setColorDepth(16);
    wfSpr.createSprite(wfInnerW, wfInnerH);
    wfSpr.fillScreen(0x001F); // Dunkles SDR-Blau als Basis
    wfInitialized = true;
  }

  // Wasserfall fließend nach unten scrollen
  wfSpr.scroll(0, 1);

  // Hintergrund der neuen Zeile mit dunklem Blau füllen
  wfSpr.fillRect(0, 0, wfInnerW, 1, 0x001F);

  // Signalstärke auslesen und Farbprofil mit 5px rotem Rand zeichnen
  uint8_t currentRssi = rx.getCurrentRSSI();
  int center = wfInnerW / 2;
  int halfWidth = map(currentRssi, 0, 45, 6, 35);
  halfWidth = constrain(halfWidth, 6, wfInnerW / 2 - 12);

  // 1. Grüner Kern in der Mitte
  int coreW = max(2, halfWidth);
  wfSpr.fillRect(center - coreW / 2, 0, coreW, 1, 0x07E0);

  // 2. Gelber Zwischenbereich daneben
  int yellowW = max(2, halfWidth / 2);
  wfSpr.fillRect(center - coreW / 2 - yellowW, 0, yellowW, 1, 0xFFE0);
  wfSpr.fillRect(center + coreW / 2, 0, yellowW, 1, 0xFFE0);

  // 3. Kräftiger, 5 Pixel dicker roter Rand außen
  int redW = 5;
  int leftRedX = center - coreW / 2 - yellowW - redW;
  int rightRedX = center + coreW / 2 + yellowW;

  if (leftRedX >= 0) {
    wfSpr.fillRect(leftRedX, 0, redW, 1, 0xF800);
  }
  if (rightRedX + redW <= wfInnerW) {
    wfSpr.fillRect(rightRedX, 0, redW, 1, 0xF800);
  }

  // Den Wasserfall unter der weißen Trennlinie ins Haupt-Sprite pushen
  wfSpr.pushSprite(&spr, wfX + 1, wfY + 2);

  if(currentCmd == CMD_SCAN)
  {
    drawScanGraphs(isSSB()? (currentFrequency + currentBFO/1000) : currentFrequency);
  }
}
