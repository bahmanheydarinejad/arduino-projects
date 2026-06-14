#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10);
const byte address[6] = "RCCTL";

#define LED LED_BUILTIN

struct Packet {
  uint8_t buttons;
  int8_t X1;
  int8_t Y1;
  int8_t X2;
  int8_t Y2;
  int8_t R1;
  int8_t R2;
};

Packet data;

unsigned long lastReceive = 0;

void setup() {

  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
}

void loop() {

  if (radio.available()) {

    radio.read(&data, sizeof(data));

    digitalWrite(LED, HIGH);
    lastReceive = millis();

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

  if (millis() - lastReceive > 500) {
    digitalWrite(LED, LOW);
  }
}
