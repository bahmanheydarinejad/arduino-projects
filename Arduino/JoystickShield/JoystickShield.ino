#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Precision for joystick neutrality
const int16_t joystickPrecision = 15;
const int16_t surfaceBound = joystickPrecision;
const int16_t floorBound = -joystickPrecision;

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

// Abstract updateable interface
class MyUpdatable {
public:
  virtual void update() = 0;
  virtual ~MyUpdatable() {}
};

class MyActionable {
public:
  virtual void action() = 0;
  virtual ~MyActionable() {}
};

// Button base class
class Button : public MyUpdatable, public MyActionable {
protected:
  int pin;
  const char* name;
  bool lastState;
  int8_t* targetAction;
  MyActionable* parent;

public:
  Button(int p, const char* n, int8_t* target, MyActionable* parent)
    : pin(p), name(n), lastState(HIGH), targetAction(target), parent(parent) {
    pinMode(pin, INPUT_PULLUP);
  }

  void update() override {
    bool currentState = digitalRead(pin);
    *targetAction = retrieveAction(currentState);
    lastState = currentState;
    action();
  }

  int retrieveAction(bool current) {
    if (isReleased(current)) return 3;      // Released
    else if (isPressed(current)) return 2;  // Pressed
    else if (isHold(current)) return 1;     // Hold
    else return 0;                          // Unhold
  }

  void action() override {
    if (parent && *targetAction != 0) parent->action();
  }

  virtual bool isPressed(bool state) {
    return lastState == HIGH && state == LOW;
  }

  virtual bool isReleased(bool state) {
    return lastState == LOW && state == HIGH;
  }

  virtual bool isHold(bool state) {
    return lastState == LOW && state == LOW;
  }
};

// Joystick class
class Joystick : public MyUpdatable, public MyActionable {
private:
  int xPin, yPin;
  const char* name;
  int16_t* xTarget;
  int16_t* yTarget;
  MyActionable* parent;

public:
  Joystick(int x, int y, const char* n, int16_t* xT, int16_t* yT, MyActionable* parent)
    : xPin(x), yPin(y), name(n), xTarget(xT), yTarget(yT), parent(parent) {
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
  }

  bool isJoystickMoved() {
    bool isXMoved = *xTarget < floorBound || *xTarget > surfaceBound;
    bool isYMoved = *yTarget < floorBound || *yTarget > surfaceBound;

    if (!isXMoved)
      *xTarget = 0;

    if (!isYMoved)
      *yTarget = 0;

    return isXMoved || isYMoved;
  }

  void update() override {
    *xTarget = analogRead(xPin) - 512;
    *yTarget = analogRead(yPin) - 512;
    action();
  }

  void action() override {
    bool isMoved = isJoystickMoved();
    if (isMoved && parent) parent->action();
  }
};

// SimpleJoyStickPad
class SimpleJoyStickPad : public MyActionable {
private:
  JoyStickPadData data;
  Button* buttons[7];
  Joystick* joystick;

  RF24& radio;
  int ledSuccess;
  int ledFail;

public:
  SimpleJoyStickPad(RF24& r, const byte* address, int successLed, int failLed)
    : radio(r), ledSuccess(successLed), ledFail(failLed) {

    pinMode(ledSuccess, OUTPUT);
    pinMode(ledFail, OUTPUT);
    digitalWrite(ledSuccess, LOW);
    digitalWrite(ledFail, LOW);

    radio.begin();
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_HIGH);
    radio.stopListening();

    buttons[0] = new Button(2, "A", &data.A, this);
    buttons[1] = new Button(3, "B", &data.B, this);
    buttons[2] = new Button(4, "C", &data.C, this);
    buttons[3] = new Button(5, "D", &data.D, this);
    buttons[4] = new Button(6, "E", &data.E, this);
    buttons[5] = new Button(7, "F", &data.F, this);
    buttons[6] = new Button(8, "SW", &data.SW, this);

    joystick = new Joystick(A0, A1, "Joystick", &data.X, &data.Y, this);
  }

  ~SimpleJoyStickPad() {
    for (int i = 0; i < 7; i++) delete buttons[i];
    delete joystick;
  }

  void update() {
    for (int i = 0; i < 7; i++) buttons[i]->update();
    joystick->update();
  }

  JoyStickPadData getSimpleJoyStickPadData() {
    update();
    return data;
  }

  void action() override {
    bool report = radio.write(&data, sizeof(data));

    if (report) {
      digitalWrite(ledSuccess, HIGH);
      digitalWrite(ledFail, LOW);
    } else {
      digitalWrite(ledSuccess, LOW);
      digitalWrite(ledFail, HIGH);
    }

    char buf[120];
    sprintf(buf,
            "Status:%s\tA:%d\tB:%d\tC:%d\tD:%d\tE:%d\tF:%d\tSW:%d\tX:%d\tY:%d",
            report ? "OK" : "FAIL",
            data.A, data.B, data.C, data.D,
            data.E, data.F, data.SW, data.X, data.Y);
    Serial.println(buf);
  }
};

// ====== GLOBALS ======
RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "00001";

const int ledSuccess = A4;
const int ledFail = A5;

SimpleJoyStickPad* simplePad;

void setup() {
  Serial.begin(9600);
  simplePad = new SimpleJoyStickPad(radio, address, ledSuccess, ledFail);
}

void loop() {
  simplePad->update();
  // برای تست چاپ دائم
  // Serial.println("Loop running");
  // delay(500);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(20);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(ledSuccess, LOW);
  digitalWrite(ledFail, LOW);
}
