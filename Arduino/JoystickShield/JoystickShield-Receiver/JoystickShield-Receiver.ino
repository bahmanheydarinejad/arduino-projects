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
  int8_t SW1 = 0;
  int16_t X1 = 0;
  int16_t Y1 = 0;
  int8_t SW2 = 0;
  int16_t X2 = 0;
  int16_t Y2 = 0;
} __attribute__((packed));

void printData(const JoyStickPadData &data) {
  bool r = 1;
  char buf[120];
  sprintf(buf,
          "Status:%s\tA:%d\tB:%d\tC:%d\tD:%d\tE:%d\tF:%d"
          "\tSW1:%d\tX1:%d\tY1:%d\tSW2:%d\tX2:%d\tY2:%d",
          r ? "OK" : "FAIL",
          data.A, data.B, data.C, data.D,
          data.E, data.F,
          data.SW1, data.X1, data.Y1,
          data.SW2, data.X2, data.Y2);
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
