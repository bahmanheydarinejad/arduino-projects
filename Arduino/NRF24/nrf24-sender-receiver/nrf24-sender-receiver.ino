#include <SPI.h>
#include <RF24.h>

const uint8_t CE_PIN = 9;
const uint8_t CSN_PIN = 10;
const uint8_t IS_TRANSMITTER = 11;
const uint8_t BLINK_LED = 13;
const byte PIPE_ADDRESS[6] = "CTRL1";

struct ControlPacket {
  int throttle;
  int yaw;
  int pitch;
  int roll;
};

class RadioDevice {
protected:
  RF24& radio;
  const byte* address;

public:
  RadioDevice(RF24& radioRef, const byte* pipeAddr)
    : radio(radioRef), address(pipeAddr) {}
  virtual void begin() = 0;
  virtual void update() = 0;
};

class Sender : public RadioDevice {
public:
  Sender(RF24& radioRef, const byte* pipeAddr)
    : RadioDevice(radioRef, pipeAddr) {}

  void begin() override {
    Serial.begin(9600);
    radio.begin();
    radio.setPALevel(RF24_PA_LOW);
    radio.setDataRate(RF24_250KBPS);
    radio.setRetries(5, 15);
    radio.openWritingPipe(address);
    radio.stopListening();
  }

  void update() override {
    digitalWrite(BLINK_LED, !digitalRead(BLINK_LED));
    ControlPacket packet = { random(0, 255), 0, 50, -30 };
    bool sent = radio.write(&packet, sizeof(packet));
    Serial.println(sent ? "\u2713 ACK received" : "\u2717 Failed, no ACK");
    delay(200);
  }
};

class Receiver : public RadioDevice {
public:
  Receiver(RF24& radioRef, const byte* pipeAddr)
    : RadioDevice(radioRef, pipeAddr) {}

  void begin() override {
    Serial.begin(115200);
    radio.begin();
    radio.setPALevel(RF24_PA_LOW);
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(1, address);
    radio.startListening();
  }

  void update() override {
    if (radio.available()) {
      digitalWrite(BLINK_LED, !digitalRead(BLINK_LED));
      ControlPacket packet;
      radio.read(&packet, sizeof(packet));
      Serial.print("Received Throttle: ");
      Serial.println(packet.throttle);
    }
  }
};

RF24 radio(CE_PIN, CSN_PIN);
RadioDevice* device;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(IS_TRANSMITTER, INPUT);

  if (digitalRead(IS_TRANSMITTER) == HIGH) {
    device = new Sender(radio, PIPE_ADDRESS);
    Serial.println("Sender initialized.");
  } else {
    device = new Receiver(radio, PIPE_ADDRESS);
    Serial.println("Receiver initialized.");
  }
  device->begin();
}

void loop() {
  device->update();
}
