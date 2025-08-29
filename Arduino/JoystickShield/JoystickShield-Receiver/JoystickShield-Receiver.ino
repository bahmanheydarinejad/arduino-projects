#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "00001";

// Struct to hold all input states
struct JoyStickPadData {
  int8_t A = 0;
  int8_t B = 0;
  int8_t C = 0;
  int8_t D = 0;
  int8_t E = 0;
  int8_t F = 0;
  int8_t SW = 0;
  int16_t X = 0;
  int16_t Y = 0;
};

void printData(const JoyStickPadData &data) {
  bool report = 1;
  char buf[120];
  sprintf(buf,
          "Status:%s\tA:%d\tB:%d\tC:%d\tD:%d\tE:%d\tF:%d\tSW:%d\tX:%d\tY:%d",
          report ? "OK" : "FAIL",
          data.A, data.B, data.C, data.D,
          data.E, data.F,
          data.SW, data.X, data.Y);
  Serial.println(buf);
}

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    JoyStickPadData data;
    radio.read(&data, sizeof(data));
    printData(data);
  }
}
