/*
  Z144SN005 (ST7735S, 128x128) bare LCD test for a 3.3 V Arduino Nano.

  Required libraries (Arduino Library Manager):
    - Adafruit GFX Library
    - Adafruit ST7735 and ST7789 Library

  LCD FPC -> Arduino Nano (3.3 V logic)
    1  NC      -> not connected
    2  GND     -> GND
    3  LED-    -> GND
    4  LED+    -> 3.3 V through a 22 ohm series resistor
    5  GND     -> GND
    6  /RESET  -> D8
    7  A0/DC   -> D9
    8  SDA     -> D11 / MOSI
    9  SCK     -> D13 / SCK
   10  VCC     -> 3.3 V
   11  IOVCC   -> 3.3 V
   12  CS      -> D10
   13  GND     -> GND
   14  NC      -> not connected

  D12/MISO is not used. Do not apply 5 V to any LCD pin.
  This is a bare 0.8 mm-pitch, 14-pin panel; use a suitable PCB/FPC adapter.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

const uint8_t TFT_CS = 10;
const uint8_t TFT_DC = 9;   // LCD pin A0: HIGH=data, LOW=command
const uint8_t TFT_RST = 8;  // LCD reset is active LOW

// The three-argument constructor selects the Nano's hardware SPI port:
// MOSI=D11 and SCK=D13 on the classic ATmega328P Nano.
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

void showSolid(uint16_t color, const __FlashStringHelper *name) {
  Serial.print(F("Solid color: "));
  Serial.println(name);
  tft.fillScreen(color);
  delay(650);
}

void drawColorBars() {
  const uint16_t colors[] = {
    ST77XX_WHITE, ST77XX_YELLOW, ST77XX_CYAN, ST77XX_GREEN,
    ST77XX_MAGENTA, ST77XX_RED, ST77XX_BLUE, ST77XX_BLACK
  };

  const int16_t barWidth = tft.width() / 8;
  for (uint8_t i = 0; i < 8; ++i) {
    const int16_t x = i * barWidth;
    const int16_t width = (i == 7) ? (tft.width() - x) : barWidth;
    tft.fillRect(x, 0, width, tft.height(), colors[i]);
  }
  delay(1200);
}

void drawGeometryTest() {
  tft.fillScreen(ST77XX_BLACK);

  // A one-pixel border and corner blocks expose wrong offsets or clipping.
  tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_WHITE);
  tft.fillRect(0, 0, 5, 5, ST77XX_RED);
  tft.fillRect(tft.width() - 5, 0, 5, 5, ST77XX_GREEN);
  tft.fillRect(0, tft.height() - 5, 5, 5, ST77XX_BLUE);
  tft.fillRect(tft.width() - 5, tft.height() - 5, 5, 5, ST77XX_YELLOW);

  for (int16_t x = 16; x < tft.width(); x += 16) {
    tft.drawFastVLine(x, 0, tft.height(), tft.color565(35, 35, 35));
  }
  for (int16_t y = 16; y < tft.height(); y += 16) {
    tft.drawFastHLine(0, y, tft.width(), tft.color565(35, 35, 35));
  }

  tft.drawLine(0, 0, tft.width() - 1, tft.height() - 1, ST77XX_CYAN);
  tft.drawLine(tft.width() - 1, 0, 0, tft.height() - 1, ST77XX_MAGENTA);
  tft.drawCircle(tft.width() / 2, tft.height() / 2, 28, ST77XX_YELLOW);
  delay(1400);
}

void drawTextTest() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  tft.setCursor(9, 8);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println(F("Z144SN005"));

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(9, 35);
  tft.println(F("ST7735S / SPI"));
  tft.setCursor(9, 48);
  tft.println(F("128 x 128 pixels"));
  tft.setCursor(9, 61);
  tft.println(F("3.3 V logic"));

  tft.fillRect(9, 80, 32, 22, ST77XX_RED);
  tft.fillRect(48, 80, 32, 22, ST77XX_GREEN);
  tft.fillRect(87, 80, 32, 22, ST77XX_BLUE);

  tft.setCursor(18, 110);
  tft.setTextColor(ST77XX_CYAN);
  tft.println(F("DISPLAY TEST OK"));
  delay(2200);
}

void runFullTest() {
  showSolid(ST77XX_BLACK, F("BLACK"));
  showSolid(ST77XX_WHITE, F("WHITE"));
  showSolid(ST77XX_RED, F("RED"));
  showSolid(ST77XX_GREEN, F("GREEN"));
  showSolid(ST77XX_BLUE, F("BLUE"));

  Serial.println(F("Color bars"));
  drawColorBars();

  Serial.println(F("Geometry and edge/offset test"));
  drawGeometryTest();

  Serial.println(F("Text test"));
  drawTextTest();
}

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println(F("\nZ144SN005 test starting"));

  // Correct Adafruit initialization profile for a 1.44-inch 128x128 panel.
  // INITR_BLACKTAB is normally the 128x160 (1.8-inch) profile.
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(0);

  Serial.print(F("Library reports "));
  Serial.print(tft.width());
  Serial.print('x');
  Serial.println(tft.height());

  runFullTest();
  Serial.println(F("Full test complete; animation running."));
}

void loop() {
  // Moving scan line confirms that repeated SPI updates remain stable.
  static int16_t y = 0;
  static int8_t direction = 1;

  tft.drawFastHLine(1, y, tft.width() - 2, ST77XX_BLACK);
  y += direction;

  if (y >= tft.height() - 2) {
    y = tft.height() - 2;
    direction = -1;
  } else if (y <= 1) {
    y = 1;
    direction = 1;
  }

  tft.drawFastHLine(1, y, tft.width() - 2, ST77XX_WHITE);
  delay(20);
}
