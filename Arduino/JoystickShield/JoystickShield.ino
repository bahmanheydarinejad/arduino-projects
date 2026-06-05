#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);

const byte address[6] = "CTRL1";

#define DEADZONE 15

struct __attribute__((packed)) Packet {
  uint8_t buttons;
  int8_t X1;
  int8_t Y1;
  int8_t X2;
  int8_t Y2;
  int8_t R1;
  int8_t R2;
};

Packet data;
uint8_t lastButtons = 0;

const int buttonPins[6] = { 2, 3, 4, 5, 6, 7 };
const int joy1X = A0;
const int joy1Y = A1;
const int joy1SW = A2;

const int joy2X = A3;
const int joy2Y = A4;
const int joy2SW = A5;

const int range1Pin = A6;
const int range2Pin = A7;

int8_t mapAxis(int pin) {
  int v = analogRead(pin) - 512;
  if (abs(v) < DEADZONE) return 0;
  return constrain(v / 4, -127, 127);
}

class InputRange {
  int pin;
public:
  InputRange(int p)
    : pin(p) {}
  int8_t read() {
    int v = analogRead(pin) - 512;
    if (abs(v) < DEADZONE) return 0;
    return constrain(v / 4, -127, 127);
  }
};

InputRange range1(range1Pin);
InputRange range2(range2Pin);

uint8_t readButtons() {

  uint8_t b = 0;

  for (int i = 0; i < 6; i++) {
    if (!digitalRead(buttonPins[i])) b |= (1 << i);
  }

  if (!digitalRead(joy1SW)) b |= (1 << 6);
  if (!digitalRead(joy2SW)) b |= (1 << 7);

  return b;
}

void setup() {

  Serial.begin(115200);

  for (int i = 0; i < 6; i++)
    pinMode(buttonPins[i], INPUT_PULLUP);

  pinMode(joy1SW, INPUT_PULLUP);
  pinMode(joy2SW, INPUT_PULLUP);

  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_1MBPS);
  radio.setRetries(5, 15);
  radio.openWritingPipe(address);
  radio.stopListening();
}

void sendPacket() {

  bool ok = radio.write(&data, sizeof(data));

  Serial.print(ok ? "OK " : "FAIL ");

  Serial.print("B:");
  Serial.print(data.buttons);

  Serial.print(" X1:");
  Serial.print(data.X1);
  Serial.print(" Y1:");
  Serial.print(data.Y1);

  Serial.print(" X2:");
  Serial.print(data.X2);
  Serial.print(" Y2:");
  Serial.print(data.Y2);

  Serial.print(" R1:");
  Serial.print(data.R1);
  Serial.print(" R2:");
  Serial.println(data.R2);
}

void loop() {

  uint8_t buttons = readButtons();

  data.X1 = mapAxis(joy1X);
  data.Y1 = mapAxis(joy1Y);

  data.X2 = mapAxis(joy2X);
  data.Y2 = mapAxis(joy2Y);

  data.R1 = range1.read();
  data.R2 = range2.read();

  if (buttons != lastButtons) {

    data.buttons = buttons;
    sendPacket();

    lastButtons = buttons;
  }

  delay(5);
}
