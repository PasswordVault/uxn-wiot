#include "config.h"
#ifdef USE_TFT_ESPI

#include <TFT_eSPI.h>
extern "C" {
    #include <uxn.h>
    #include <devices/screen.h>
}

extern void error(const char *msg, const char *err);

TFT_eSPI tft = TFT_eSPI();
#ifdef USE_TFT_DMA
Uint16 *line;
#else
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
TFT_eSprite display = TFT_eSprite(&tft);
#endif

int 
devscreen_init() {
  tft.begin();
#ifdef USE_TFT_DMA
  tft.initDMA();
#endif
  //tft.setRotation(3);
  //tft.fillScreen(TFT_BLACK);
  //tft.setTextColor(TFT_GREEN);
  //tft.setCursor(0, 0);
#ifdef USE_TFT_DMA
  line = (Uint16*)heap_caps_malloc(tft.width() * sizeof(Uint16), MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
  if(line == NULL)
    error("devscreen_init", "not enough memory");
#else
  display.setColorDepth(4);
  display.createSprite(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display.fillSprite(TFT_BLACK);
#endif
  screen_resize(&uxn_screen, tft.width(), tft.height());
  if(uxn_screen.pixels == NULL)
    error("devscreen_init", "not enough memory");
  return 1;
}

int 
devscreen_redraw() {
  Uint32 x, y, w = uxn_screen.width, h = uxn_screen.height;
  Uint16 palette[16], c;
  Uint8 *pixels = uxn_screen.pixels, p, p1, p2;
  int i;

  unsigned long timestamp = micros();

	for(i = 0; i < 16; i++) {
    c = uxn_screen.palette[(i >> 2) ? (i >> 2) : (i & 3)];
    palette[i] = c >> 8 | c << 8; /* We swap bytes for the tft screen */
  }

#ifdef USE_TFT_DMA
  tft.startWrite();
  tft.setAddrWindow(0, 0, w, h);

  /* TODO: write  more efficient loop */
  /* but it is not the bottleneck (the SPI port IS) */
  for(y=0; y< h; y++) {
    for(x = 0; x < w; x+=2) {
      p = pixels[(y*w+x)/2];
      p1 = p >> 4;
      p2 = p & 0xf;
      line[x] = palette[p1];
      line[x+1] = palette[p2];
    }
    /*Serial.printf("Busy ? %s\n", tft.dmaBusy() ? "true" : "false");*/
    tft.pushPixelsDMA(line, w);
  }
  tft.endWrite();
#else
  for(y=0; y < h; y += 1) {
    for(x = 0; x < w; x += 2) {
      p = pixels[(y*w+x)/2];
      p1 = p >> 4;
      p2 = p & 0xf;
      display.drawPixel(x, y, palette[p1]);
      display.drawPixel(x+1, y, palette[p2]);
    }
  }
  display.pushSprite(0, 0);
#endif
  uxn_screen.changed = 0;
  /* Serial.printf("\n%ld us\n", micros() - timestamp); */
  return 1;
}

#endif