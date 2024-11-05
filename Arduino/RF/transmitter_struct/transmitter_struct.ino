#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 9);  // CE, CSN
const byte address[6] = "00001";

const int button1Pin = 2;
const int button2Pin = 3;
const int button3Pin = 4;
const int LED_SUCCESS = 5;
const int LED_FAIL = 6;

struct DataPacket {
  char button;
};

void setup() {
  Serial.begin(9600);
  Serial.println("Starting setup...");

  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);

  pinMode(LED_SUCCESS, OUTPUT);
  digitalWrite(LED_SUCCESS, false);

  pinMode(LED_FAIL, OUTPUT);
  digitalWrite(LED_FAIL, false);

  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!!");
    while (1) {}  // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(3, 5);
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("Press any button to send ...");
}

void loop() {
  if (digitalRead(button1Pin) == LOW) {
    send(DataPacket{'1'});
  }

  if (digitalRead(button2Pin) == LOW) {
    send(DataPacket{'2'});
  }

  if (digitalRead(button3Pin) == LOW) {
    send(DataPacket{'3'});
  }

  digitalWrite(LED_SUCCESS, false);
  digitalWrite(LED_FAIL, false);
}

void send(DataPacket packet) {
  bool success = radio.write(&packet, sizeof(DataPacket));
  Serial.print("Send result: ");
  Serial.print(packet.button);
  Serial.print(" :: ");
  Serial.println(success ? "Success" : "Fail");

  digitalWrite(LED_SUCCESS, success ? HIGH : LOW);
  digitalWrite(LED_FAIL, !success ? HIGH : LOW);
  delay(10);  // debounce delay
}
