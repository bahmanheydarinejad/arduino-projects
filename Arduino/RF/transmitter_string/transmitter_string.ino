#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WString.h>

RF24 radio(8, 9);  // CE, CSN
const byte address[6] = "00001";

const int button1Pin = 2;
const int button2Pin = 3;
const int button3Pin = 4;
const int LED_SUCCESS = 5;
const int LED_FAIL = 6;

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
    send("b1");
  }

  if (digitalRead(button2Pin) == LOW) {
    send("b2");
  }

  if (digitalRead(button3Pin) == LOW) {
    send("b3");
  }
  digitalWrite(LED_SUCCESS, false);
  digitalWrite(LED_FAIL, false);
}

void send(char text[]) {
  
  int r = radio.write(&text, text.length + 1);
  Serial.println("Send result: [" + String(sizeof(text)) + "] " + String(text) + "::" + String(r));
  digitalWrite(LED_SUCCESS, r ? HIGH : LOW);
  digitalWrite(LED_FAIL, !r ? HIGH : LOW);
  delay(50);  // debounce delay
}


