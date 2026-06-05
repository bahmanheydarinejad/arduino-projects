#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);

const byte address[6] = "CTRL1";

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

unsigned long lastReceiveTime = 0;
const unsigned long failsafeTimeout = 500;

void failsafe() {

  data.buttons = 0;
  data.X1 = 0;
  data.Y1 = 0;
  data.X2 = 0;
  data.Y2 = 0;
  data.R1 = 0;
  data.R2 = 0;
}

void logPacket() {

  Serial.print("OK ");

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

void setup() {

  Serial.begin(115200);

  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_1MBPS);
  radio.openReadingPipe(0, address);
  radio.startListening();
}

void loop() {

  if (radio.available()) {

    radio.read(&data, sizeof(data));

    lastReceiveTime = millis();

    logPacket();
  }

  if (millis() - lastReceiveTime > failsafeTimeout) {
    failsafe();
  }
}
