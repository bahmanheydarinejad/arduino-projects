#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);  // CE, CSN pins

const byte address[6] = "00001";
char incomingMessage[32];
char outgoingMessage[32] = "Hello from Arduino";

unsigned long lastSendTime = 0;
unsigned long sendInterval = 100;  // Adjust as needed

// LED pin assignments
const int powerOnLED = 2;
const int transmitLED = 3;
const int listeningLED = 4;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_2MBPS);  // Set to maximum data rate
  radio.setRetries(3, 15);        // Set retries (3 times with 15 * 250us delay)
  radio.stopListening();

  // Initialize LED pins
  pinMode(listeningLED, OUTPUT);
  pinMode(powerOnLED, OUTPUT);
  pinMode(transmitLED, OUTPUT);

  // Set initial LED states
  digitalWrite(powerOnLED, HIGH);
  digitalWrite(listeningLED, LOW);
  digitalWrite(transmitLED, LOW);
}

void loop() {
  // Sending data
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval) {
    radio.stopListening();  // Stop listening and start sending
    digitalWrite(listeningLED, LOW);
    digitalWrite(transmitLED, HIGH);

    bool ack = radio.write(&outgoingMessage, sizeof(outgoingMessage));
    if (ack) {
      Serial.println("Message sent successfully");
    } else {
      Serial.println("Message send failed");
    }

    digitalWrite(transmitLED, LOW);
    lastSendTime = currentTime;
  }

  // Receiving data
  radio.startListening();  // Start listening to receive data
  digitalWrite(listeningLED, HIGH);

  unsigned long startTime = millis();
  bool timeout = false;
  while (!radio.available() && !timeout) {
    if (millis() - startTime > 100) {  // Adjust timeout as needed
      timeout = true;
    }
  }

  if (!timeout) {
    radio.read(&incomingMessage, sizeof(incomingMessage));
    Serial.print("Received message: ");
    Serial.println(incomingMessage);
  }

  radio.stopListening();  // Stop listening to prepare for the next send
  digitalWrite(listeningLED, LOW);
  digitalWrite(transmitLED, LOW);
}
