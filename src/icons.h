#ifndef ICONS_H
#define ICONS_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

extern Adafruit_ST7789 tft;

// 1. Sonne (Gelber Kreis mit 8 Strahlen)
inline void drawSunIcon32(int x, int y) {
  uint16_t yellow = ST77XX_YELLOW;
  tft.drawFastVLine(x + 16, y + 2,  5, yellow);
  tft.drawFastVLine(x + 16, y + 25, 5, yellow);
  tft.drawFastHLine(x + 2,  y + 16, 5, yellow);
  tft.drawFastHLine(x + 25, y + 16, 5, yellow);
  tft.drawLine(x + 6,  y + 6,  x + 9,  y + 9,  yellow);
  tft.drawLine(x + 25, y + 6,  x + 22, y + 9,  yellow);
  tft.drawLine(x + 6,  y + 25, x + 9,  y + 22, yellow);
  tft.drawLine(x + 25, y + 25, x + 22, y + 22, yellow);
  tft.fillCircle(x + 16, y + 16, 7, yellow);
}

// 2. Wolke (Graue Rundungen mit flacher Basis)
inline void drawCloudIcon32(int x, int y, uint16_t color = 0xAD55) {
  tft.fillCircle(x + 12, y + 18, 7, color);
  tft.fillCircle(x + 20, y + 15, 9, color);
  tft.fillCircle(x + 25, y + 20, 6, color);
  tft.fillRect(x + 11, y + 19, 15, 8, color);
}

// 3. Sonne hinter Wolke
inline void drawSunCloudIcon32(int x, int y) {
  tft.fillCircle(x + 22, y + 10, 6, ST77XX_YELLOW);
  tft.drawFastVLine(x + 22, y + 2, 3, ST77XX_YELLOW);
  tft.drawFastHLine(x + 29, y + 10, 3, ST77XX_YELLOW);
  drawCloudIcon32(x, y, 0xCE79);
}

// 4. Regen (Dunklere Wolke + 3 blaue Tropfen)
inline void drawRainIcon32(int x, int y) {
  drawCloudIcon32(x, y - 2, 0x8410);
  uint16_t blue = tft.color565(0, 180, 255);
  tft.drawLine(x + 12, y + 25, x + 10, y + 29, blue);
  tft.drawLine(x + 18, y + 25, x + 16, y + 29, blue);
  tft.drawLine(x + 24, y + 25, x + 22, y + 29, blue);
}

// 5. Gewitter (Wolke + gelber Zickzack-Blitz)
inline void drawThunderIcon32(int x, int y) {
  drawCloudIcon32(x, y - 2, 0x52AA);
  uint16_t yellow = ST77XX_YELLOW;
  tft.drawLine(x + 18, y + 21, x + 14, y + 26, yellow);
  tft.drawLine(x + 14, y + 26, x + 17, y + 26, yellow);
  tft.drawLine(x + 17, y + 26, x + 13, y + 31, yellow);
}

// 6. Schnee (Wolke + weiße Flockenpunkte)
inline void drawSnowIcon32(int x, int y) {
  drawCloudIcon32(x, y - 2, 0xAD55);
  uint16_t white = ST77XX_WHITE;
  tft.drawPixel(x + 13, y + 26, white);
  tft.drawPixel(x + 19, y + 28, white);
  tft.drawPixel(x + 24, y + 25, white);
}

// 7. Icon-Router anhand der OpenWeather-Condition-ID
inline void drawWeatherIconById32(int id, int x, int y) {
  if (id >= 200 && id < 300) {
    drawThunderIcon32(x, y);
  } else if (id >= 300 && id < 600) {
    drawRainIcon32(x, y);
  } else if (id >= 600 && id < 700) {
    drawSnowIcon32(x, y);
  } else if (id == 800) {
    drawSunIcon32(x, y);
  } else if (id == 801 || id == 802) {
    drawSunCloudIcon32(x, y);
  } else {
    drawCloudIcon32(x, y); // 803, 804 (Bedeckt)
  }
}

#endif // ICONS_H