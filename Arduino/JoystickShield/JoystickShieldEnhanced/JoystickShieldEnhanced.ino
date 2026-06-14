#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10);
const byte address[6] = "RCCTL";

struct Packet {
  uint8_t buttons;
  int8_t X1;
  int8_t Y1;
  int8_t X2;
  int8_t Y2;
  int8_t R1;
  int8_t R2;
};

#define LED LED_BUILTIN

// ================= BUTTON =================

template<uint8_t PIN, uint8_t BIT>
class Button {

  bool last = false;

  bool readRaw() const {
    return digitalRead(PIN) == LOW;
  }

public:

  Button() {
    pinMode(PIN, INPUT_PULLUP);
  }

  bool isChanged() {
    return readRaw() != last;
  }

  void writeTo(Packet& p) {

    bool v = readRaw();
    last = v;

    if (v) p.buttons |= (1 << BIT);
    else p.buttons &= ~(1 << BIT);
  }
};


// ================= AXIS =================

template<uint8_t PIN, int8_t Packet::*FIELD, int CENTER = 512>
class AxisInput {

  int8_t last = 0;

  int8_t readScaled() const {

    int v = analogRead(PIN) - CENTER;
    v /= 4;

    if (v > 127) v = 127;
    if (v < -128) v = -128;

    return v;
  }

public:

  bool isChanged() {
    return readScaled() != last;
  }

  void writeTo(Packet& p) {

    last = readScaled();
    p.*FIELD = last;
  }
};


// ================= JOYSTICK =================

template<class AX, class AY, class SW>
class JoyInput {

  AX ax;
  AY ay;
  SW sw;

public:

  bool isChanged() {

    return ax.isChanged()
           || ay.isChanged()
           || sw.isChanged();
  }

  void writeTo(Packet& p) {

    ax.writeTo(p);
    ay.writeTo(p);
    sw.writeTo(p);
  }
};


// ================= VARIADIC JOYSTICK =================

template<class... Components>
class JoyStick;

template<>
class JoyStick<> {

public:

  bool isChanged() {
    return false;
  }
  void write(Packet&) {}
};

template<class First, class... Rest>
class JoyStick<First, Rest...> {

  First first;
  JoyStick<Rest...> rest;

public:

  bool isChanged() {

    if (first.isChanged()) return true;
    return rest.isChanged();
  }

  void write(Packet& p) {

    first.writeTo(p);
    rest.write(p);
  }

  Packet getValue() {

    Packet p{};
    write(p);
    return p;
  }
};


// ================= TYPE DEFINITIONS =================

using JS = JoyStick<

  Button<2, 0>,
  Button<3, 1>,
  Button<4, 2>,
  Button<5, 3>,
  Button<6, 4>,
  Button<7, 5>,
  Button<8, 6>,
  Button<9, 7>,

  AxisInput<A0, &Packet::R1>,
  AxisInput<A1, &Packet::R2>,

  JoyInput<
    AxisInput<A2, &Packet::X1>,
    AxisInput<A3, &Packet::Y1>,
    Button<10, 6> >,

  JoyInput<
    AxisInput<A4, &Packet::X2>,
    AxisInput<A5, &Packet::Y2>,
    Button<11, 7> >

  >;

JS js;


// ================= SEND =================

void sendPacket() {

  Packet p = js.getValue();

  bool ok = radio.write(&p, sizeof(p));

  if (ok) digitalWrite(LED, HIGH);
  else digitalWrite(LED, LOW);

  Serial.print(ok ? "OK" : "FAIL");
  if (ok) {
    Serial.print(" B:");
    Serial.print(p.buttons);
    Serial.print(" X1:");
    Serial.print(p.X1);
    Serial.print(" Y1:");
    Serial.print(p.Y1);
    Serial.print(" X2:");
    Serial.print(p.X2);
    Serial.print(" Y2:");
    Serial.print(p.Y2);
    Serial.print(" R1:");
    Serial.print(p.R1);
    Serial.print(" R2:");
    Serial.println(p.R2);
  } else {
    Serial.println("");
  }
}


// ================= SETUP =================

void setup() {

  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  radio.begin();
  radio.openWritingPipe(address);
  radio.stopListening();
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_LOW);
}


// ================= LOOP =================

void loop() {

  if (js.isChanged()) {
    sendPacket();
    delay(50);
  }
}
