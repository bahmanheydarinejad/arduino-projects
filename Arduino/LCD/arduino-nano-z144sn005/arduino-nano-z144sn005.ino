/*
  One-second ultrasonic history display
  Arduino Nano (3.3 V GPIO) + HC-SR04 + Z144SN005/ST7735S 128x128 LCD

  Required libraries:
    - Adafruit GFX Library
    - Adafruit ST7735 and ST7789 Library

  LCD FPC -> Nano
    1  NC       not connected
    2  GND      GND
    3  LED-     GND
    4  LED+     3.3 V through 22 ohm resistor
    5  GND      GND
    6  /RESET   D8
    7  A0/DC    D9
    8  SDA      D11/MOSI
    9  SCK      D13/SCK
   10  VCC      3.3 V
   11  IOVCC    3.3 V
   12  CS       D10
   13  GND      GND
   14  NC       not connected

  HC-SR04 -> Nano
    VCC          5 V (standard HC-SR04 normally needs 5 V)
    GND          common GND
    TRIG         D2
    ECHO         1 kohm -> D3, plus 2 kohm from D3 to GND

  WARNING: HC-SR04 ECHO is approximately 5 V. Do not connect it directly to
  a 3.3 V Nano input. The 1 kohm/2 kohm divider reduces it to about 3.3 V.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

const uint8_t TFT_CS = 10;
const uint8_t TFT_DC = 9;
const uint8_t TFT_RST = 8;

const uint8_t HC_TRIG = 2;
const uint8_t HC_ECHO = 3;

const uint16_t HISTORY_MS = 1000;
const uint16_t SAMPLE_INTERVAL_MS = 60;
const uint32_t ECHO_TIMEOUT_US = 25000UL;

// Change this if you need a different vertical range.
const uint16_t GRAPH_MAX_CM = 200;
const uint16_t GRAPH_MAX_MM = GRAPH_MAX_CM * 10U;

const uint8_t MAX_SAMPLES = 20;
const uint16_t INVALID_DISTANCE = 0xFFFF;

const int16_t PLOT_LEFT = 1;
const int16_t PLOT_RIGHT = 126;
const int16_t PLOT_TOP = 30;
const int16_t PLOT_BOTTOM = 112;

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

uint32_t sampleTime[MAX_SAMPLES];
uint16_t sampleDistanceMm[MAX_SAMPLES];
uint8_t sampleCount = 0;
uint32_t lastSampleMs = 0;

uint16_t readDistanceMm() {
  digitalWrite(HC_TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(HC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(HC_TRIG, LOW);

  const uint32_t echoUs = pulseIn(HC_ECHO, HIGH, ECHO_TIMEOUT_US);
  if (echoUs == 0) {
    return INVALID_DISTANCE;
  }

  // Speed of sound is approximately 0.343 mm/us. Divide by two because
  // the pulse travels to the object and back. Integer calculation is rounded.
  const uint32_t distanceMm = (echoUs * 343UL + 1000UL) / 2000UL;
  if (distanceMm > 65534UL) {
    return INVALID_DISTANCE;
  }
  return (uint16_t)distanceMm;
}

void addSample(uint32_t now, uint16_t distanceMm) {
  if (sampleCount < MAX_SAMPLES) {
    sampleTime[sampleCount] = now;
    sampleDistanceMm[sampleCount] = distanceMm;
    ++sampleCount;
    return;
  }

  // The array is small (about 17 useful readings per second), so shifting it
  // is simple and uses less RAM than a more complicated display buffer.
  for (uint8_t i = 1; i < MAX_SAMPLES; ++i) {
    sampleTime[i - 1] = sampleTime[i];
    sampleDistanceMm[i - 1] = sampleDistanceMm[i];
  }
  sampleTime[MAX_SAMPLES - 1] = now;
  sampleDistanceMm[MAX_SAMPLES - 1] = distanceMm;
}

int16_t distanceToY(uint16_t distanceMm) {
  const uint16_t clipped = min(distanceMm, GRAPH_MAX_MM);
  const uint16_t plotHeight = PLOT_BOTTOM - PLOT_TOP;
  return PLOT_BOTTOM - ((uint32_t)clipped * plotHeight / GRAPH_MAX_MM);
}

uint16_t pointColor(uint16_t distanceMm) {
  if (distanceMm < 200) {       // closer than 20 cm
    return ST77XX_RED;
  }
  if (distanceMm < 500) {       // 20 to 50 cm
    return ST77XX_YELLOW;
  }
  return ST77XX_GREEN;
}

void drawHeader(uint16_t newestDistanceMm) {
  tft.fillRect(0, 0, tft.width(), PLOT_TOP - 1, ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.setTextSize(2);
  tft.setCursor(2, 2);

  if (newestDistanceMm == INVALID_DISTANCE) {
    tft.setTextColor(ST77XX_RED);
    tft.print(F("NO ECHO"));
  } else {
    tft.setTextColor(pointColor(newestDistanceMm));
    tft.print(newestDistanceMm / 10);
    tft.print('.');
    tft.print(newestDistanceMm % 10);
    tft.print(F(" cm"));
  }

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(2, 20);
  tft.print(F("History: last 1 second"));
}

void drawPlot(uint32_t now) {
  const uint16_t gridColor = tft.color565(45, 45, 45);

  tft.fillRect(PLOT_LEFT, PLOT_TOP,
               PLOT_RIGHT - PLOT_LEFT + 1,
               PLOT_BOTTOM - PLOT_TOP + 1,
               ST77XX_BLACK);

  // 250 ms vertical grid lines and 50 cm horizontal grid lines.
  for (uint8_t i = 1; i < 4; ++i) {
    const int16_t x = PLOT_LEFT +
                      (int32_t)i * (PLOT_RIGHT - PLOT_LEFT) / 4;
    tft.drawFastVLine(x, PLOT_TOP, PLOT_BOTTOM - PLOT_TOP + 1, gridColor);
  }
  for (uint8_t i = 1; i < 4; ++i) {
    const int16_t y = PLOT_TOP +
                      (int32_t)i * (PLOT_BOTTOM - PLOT_TOP) / 4;
    tft.drawFastHLine(PLOT_LEFT, y, PLOT_RIGHT - PLOT_LEFT + 1, gridColor);
  }

  tft.drawRect(PLOT_LEFT, PLOT_TOP,
               PLOT_RIGHT - PLOT_LEFT + 1,
               PLOT_BOTTOM - PLOT_TOP + 1,
               ST77XX_WHITE);

  bool havePreviousPoint = false;
  int16_t previousX = 0;
  int16_t previousY = 0;

  for (uint8_t i = 0; i < sampleCount; ++i) {
    const uint32_t age = now - sampleTime[i];
    if (age > HISTORY_MS || sampleDistanceMm[i] == INVALID_DISTANCE) {
      havePreviousPoint = false;
      continue;
    }

    const int16_t x = PLOT_RIGHT -
                      (uint32_t)age * (PLOT_RIGHT - PLOT_LEFT) / HISTORY_MS;
    const int16_t y = distanceToY(sampleDistanceMm[i]);
    const uint16_t color = pointColor(sampleDistanceMm[i]);

    if (havePreviousPoint) {
      tft.drawLine(previousX, previousY, x, y, color);
    }
    tft.fillCircle(x, y, 2, color);

    previousX = x;
    previousY = y;
    havePreviousPoint = true;
  }

  tft.fillRect(0, PLOT_BOTTOM + 2, tft.width(),
               tft.height() - PLOT_BOTTOM - 2, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(1, 117);
  tft.print(F("-1s"));
  tft.setCursor(103, 117);
  tft.print(F("now"));
}

void printSerialReading(uint16_t distanceMm) {
  if (distanceMm == INVALID_DISTANCE) {
    Serial.println(F("No echo"));
    return;
  }

  Serial.print(distanceMm / 10);
  Serial.print('.');
  Serial.println(distanceMm % 10);
}

void setup() {
  Serial.begin(115200);

  pinMode(HC_TRIG, OUTPUT);
  pinMode(HC_ECHO, INPUT);
  digitalWrite(HC_TRIG, LOW);

  tft.initR(INITR_144GREENTAB);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(1);
  tft.setCursor(12, 55);
  tft.print(F("Ultrasonic scope"));
  delay(700);

  lastSampleMs = millis() - SAMPLE_INTERVAL_MS;
}

void loop() {
  const uint32_t now = millis();
  if ((uint32_t)(now - lastSampleMs) < SAMPLE_INTERVAL_MS) {
    return;
  }

  // Advance by the requested interval rather than assigning now, reducing
  // long-term timing drift. Recover cleanly if an operation took too long.
  lastSampleMs += SAMPLE_INTERVAL_MS;
  if ((uint32_t)(now - lastSampleMs) >= SAMPLE_INTERVAL_MS) {
    lastSampleMs = now;
  }

  const uint16_t distanceMm = readDistanceMm();
  const uint32_t measuredAt = millis();
  addSample(measuredAt, distanceMm);

  drawHeader(distanceMm);
  drawPlot(measuredAt);
  printSerialReading(distanceMm);
}
