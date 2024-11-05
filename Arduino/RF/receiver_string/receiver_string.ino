#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WString.h>

RF24 radio(8, 9);  // CE, CSN
const byte address[6] = "00001";

const int redLEDPin = 2;
const int yellowLEDPin = 3;
const int greenLEDPin = 4;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting setup...");

  pinMode(redLEDPin, OUTPUT);
  pinMode(yellowLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);

  digitalWrite(redLEDPin, HIGH);
  digitalWrite(yellowLEDPin, HIGH);
  digitalWrite(greenLEDPin, HIGH);

  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!!");
    while (1) {}  // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(0, address);
  radio.startListening();

  Serial.println("Listening ...");
}

void loop() {
  if (radio.available()) {

    char text[32] = {0};
    radio.read(&text, sizeof(text));
    Serial.print("Recieved: [" + String(sizeof(text)) + "]" + String((char*)text) + " ==> ");

    for (int i = 0; i < sizeof(text); i++) {
      Serial.print((char)text[i]);
    }
    Serial.println();

    // memset(text, 0, sizeof(text));

    if (strcmp(text, "b1") == 0) {
      digitalWrite(redLEDPin, !digitalRead(redLEDPin));
    } else if (strcmp(text, "b2") == 0) {
      digitalWrite(yellowLEDPin, !digitalRead(yellowLEDPin));
    } else if (strcmp(text, "b3") == 0) {
      digitalWrite(greenLEDPin, !digitalRead(greenLEDPin));
    }
  }
}
