#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 9);  // CE, CSN
const byte address[6] = "00001";

const int LED_BUTTON1 = 2;
const int LED_BUTTON2 = 3;
const int LED_BUTTON3 = 4;

struct DataPacket {
  char button;
};

void setup() {
  Serial.begin(9600);
  Serial.println("Starting setup...");

  pinMode(LED_BUTTON1, OUTPUT);
  digitalWrite(LED_BUTTON1, LOW);

  pinMode(LED_BUTTON2, OUTPUT);
  digitalWrite(LED_BUTTON2, LOW);

  pinMode(LED_BUTTON3, OUTPUT);
  digitalWrite(LED_BUTTON3, LOW);

  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!!");
    while (1) {}  // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(0, address);
  radio.startListening();

  Serial.println("Receiver ready, waiting for data...");
}

void loop() {
  if (radio.available()) {
    DataPacket packet;
    radio.read(&packet, sizeof(DataPacket));

    Serial.print("Received button: ");
    Serial.println(packet.button);

    switch (packet.button) {
      case '1':
        digitalWrite(LED_BUTTON1, HIGH);
        delay(10);  // Keep LED on for 500ms
        digitalWrite(LED_BUTTON1, LOW);
        break;
      case '2':
        digitalWrite(LED_BUTTON2, HIGH);
        delay(10);  // Keep LED on for 500ms
        digitalWrite(LED_BUTTON2, LOW);
        break;
      case '3':
        digitalWrite(LED_BUTTON3, HIGH);
        delay(10);  // Keep LED on for 500ms
        digitalWrite(LED_BUTTON3, LOW);
        break;
      default:
        Serial.println("Unknown button received");
        break;
    }
  }
}
