/*
  Distance circle display

  Hardware:
    - Arduino Nano with 3.3 V GPIO
    - Bare Z144SN005, ST7735S, 128x128 LCD
    - HC-SR04 ultrasonic sensor powered from 5 V

  The circle is red and almost screen-size at 2 cm. It shrinks and changes
  continuously toward green as the target moves away. At 200 cm its radius
  is 2 pixels. Targets beyond 200 cm are treated as out of range.

  IMPORTANT: HC-SR04 ECHO is approximately 5 V. Use the resistor divider
  shown in README.md; never connect ECHO directly to a 3.3 V Nano input.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Z144SN005 LCD control pins. Hardware SPI uses D11=MOSI and D13=SCK.
const uint8_t TFT_CS = 10;
const uint8_t TFT_DC = 9;
const uint8_t TFT_RST = 8;

// HC-SR04 pins.
const uint8_t HC_TRIG = 2;
const uint8_t HC_ECHO = 3;

// HC-SR04 usable range for this application.
const uint16_t NEAREST_MM = 20;   // 2 cm
const uint16_t FARTHEST_MM = 2000; // 200 cm
const uint16_t SAMPLE_PERIOD_MS = 60;
const uint32_t ECHO_TIMEOUT_US = 15000UL;

// The center and maximum radius leave a small margin around the 128x128 LCD.
const int16_t CIRCLE_X = 64;
const int16_t CIRCLE_Y = 64;
const int16_t MAX_RADIUS = 62;
const int16_t MIN_RADIUS = 2;

const uint16_t INVALID_DISTANCE = 0xFFFF;

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

int16_t previousRadius = -1;
bool showingNoTarget = false;
uint32_t lastSampleMs = 0;

uint16_t measureDistanceMm() {
  // Generate the HC-SR04 trigger pulse.
  digitalWrite(HC_TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(HC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(HC_TRIG, LOW);

  const uint32_t echoTimeUs = pulseIn(HC_ECHO, HIGH, ECHO_TIMEOUT_US);
  if (echoTimeUs == 0) {
    return INVALID_DISTANCE;
  }

  // Distance = time * 0.343 mm/us / 2. This integer form includes rounding.
  const uint32_t distanceMm = (echoTimeUs * 343UL + 1000UL) / 2000UL;

  if (distanceMm > FARTHEST_MM) {
    return INVALID_DISTANCE;
  }

  // Very close echoes are clamped to the HC-SR04's approximate 2 cm limit.
  return (uint16_t)max(distanceMm, (uint32_t)NEAREST_MM);
}

int16_t radiusForDistance(uint16_t distanceMm) {
  const uint32_t position = distanceMm - NEAREST_MM;
  const uint32_t span = FARTHEST_MM - NEAREST_MM;

  return MAX_RADIUS -
         (position * (MAX_RADIUS - MIN_RADIUS) + span / 2) / span;
}

uint16_t colorForDistance(uint16_t distanceMm) {
  const uint32_t position = distanceMm - NEAREST_MM;
  const uint32_t span = FARTHEST_MM - NEAREST_MM;

  // Near: red=255, green=0. Far: red=0, green=255.
  const uint8_t green = (position * 255UL + span / 2) / span;
  const uint8_t red = 255 - green;

  return tft.color565(red, green, 0);
}

void showNoTarget() {
  if (showingNoTarget) {
    return;
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(36, 55);
  tft.print(F("NO TARGET"));
  tft.setCursor(25, 69);
  tft.print(F("within 2 meters"));

  previousRadius = -1;
  showingNoTarget = true;
}

void showTarget(uint16_t distanceMm) {
  if (showingNoTarget) {
    // Clear the message only when changing from no-target to target mode.
    tft.fillScreen(ST77XX_BLACK);
    showingNoTarget = false;
  }

  // Erase the previous outline before drawing the new one. This avoids
  // repeatedly clearing the full screen and greatly reduces visible flicker.
  if (previousRadius >= 0) {
    tft.drawCircle(CIRCLE_X, CIRCLE_Y, previousRadius, ST77XX_BLACK);
  }

  const int16_t radius = radiusForDistance(distanceMm);
  const uint16_t color = colorForDistance(distanceMm);
  tft.drawCircle(CIRCLE_X, CIRCLE_Y, radius, color);
  previousRadius = radius;
}

void printDistance(uint16_t distanceMm) {
  if (distanceMm == INVALID_DISTANCE) {
    Serial.println(F("No target within 200 cm"));
    return;
  }

  Serial.print(F("Distance: "));
  Serial.print(distanceMm / 10);
  Serial.print('.');
  Serial.print(distanceMm % 10);
  Serial.println(F(" cm"));
}

void setup() {
  Serial.begin(115200);

  pinMode(HC_TRIG, OUTPUT);
  pinMode(HC_ECHO, INPUT);
  digitalWrite(HC_TRIG, LOW);

  // Correct Adafruit initialization profile for a 1.44-inch 128x128 panel.
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);

  showNoTarget();
  lastSampleMs = millis() - SAMPLE_PERIOD_MS;
}

void loop() {
  const uint32_t now = millis();
  if ((uint32_t)(now - lastSampleMs) < SAMPLE_PERIOD_MS) {
    return;
  }
  lastSampleMs = now;

  const uint16_t distanceMm = measureDistanceMm();
  if (distanceMm == INVALID_DISTANCE) {
    showNoTarget();
  } else {
    showTarget(distanceMm);
  }

  printDistance(distanceMm);
}
