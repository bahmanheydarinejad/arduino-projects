// nrf24_radio.ino
// Clean, SOLID, YAGNI-based implementation with a simple packet struct

#include <SPI.h>
#include <RF24.h>
#include <stdint.h>

// --- Protocol: SimplePacket ---
// 4 longs (32-bit) and 8 one-byte ints
struct SimplePacket {
  long  values32[4];   // four 32-bit integers
  int8_t values8[8];   // eight 8-bit integers
};

// --- Abstract Base: IRadioEndPoint ---
class IRadioEndPoint {
protected:
  RF24 radio;
  const uint64_t pipeAddress;

  IRadioEndPoint(uint8_t cePin, uint8_t csPin, uint64_t address)
    : radio(cePin, csPin), pipeAddress(address) {}

  void configure(uint8_t channel = 90) {
    SPI.begin();
    radio.begin();
    radio.setChannel(channel);
    radio.setPALevel(RF24_PA_HIGH);
    radio.setDataRate(RF24_1MBPS);
    radio.openWritingPipe(pipeAddress);
    radio.openReadingPipe(1, pipeAddress);
  }
};

// --- Sender ---
class RadioSender : public IRadioEndPoint {
public:
  RadioSender(uint8_t cePin, uint8_t csPin, uint64_t txAddress, uint8_t channel = 90)
    : IRadioEndPoint(cePin, csPin, txAddress) {
    configure(channel);
    radio.stopListening();
  }

  bool send(const SimplePacket& packet) {
    // Directly transmit the raw struct
    return radio.write(&packet, sizeof(SimplePacket));
  }
};

// --- Receiver ---
class RadioReceiver : public IRadioEndPoint {
public:
  RadioReceiver(uint8_t cePin, uint8_t csPin, uint64_t rxAddress, uint8_t channel = 90)
    : IRadioEndPoint(cePin, csPin, rxAddress) {
    configure(channel);
    radio.startListening();
  }

  bool available() {
    return radio.available();
  }

  bool receive(SimplePacket& packet) {
    if (!radio.available()) return false;
    // Read raw into struct
    radio.read(&packet, sizeof(SimplePacket));
    return true;
  }
};

// --- Usage Example ---
const uint64_t ADDRESS = 0xF0F0F0F0E1LL;
RadioSender sender(9, 10, ADDRESS);
RadioReceiver receiver(9, 10, ADDRESS);

void setup() {
  Serial.begin(9600);
}

void loop() {
  // 1) Sending
  SimplePacket outPkt;
  // Populate 32-bit values
  outPkt.values32[0] = millis();
  outPkt.values32[1] = analogRead(A0);
  outPkt.values32[2] = 1234;
  outPkt.values32[3] = 5678;
  // Populate 8-bit values
  for (uint8_t i = 0; i < 8; ++i) {
    outPkt.values8[i] = i;
  }

  if (sender.send(outPkt)) {
    Serial.println("Packet sent");
  }
  delay(500);

  // 2) Receiving
  if (receiver.available()) {
    SimplePacket inPkt;
    if (receiver.receive(inPkt)) {
      // Print 32-bit values
      for (uint8_t i = 0; i < 4; ++i) {
        Serial.print("values32["); Serial.print(i); Serial.print("]: ");
        Serial.println(inPkt.values32[i]);
      }
      // Print 8-bit values
      for (uint8_t i = 0; i < 8; ++i) {
        Serial.print("values8["); Serial.print(i); Serial.print("]: ");
        Serial.println(inPkt.values8[i]);
      }
    }
  }
}
