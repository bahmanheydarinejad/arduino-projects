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
  int8_t SW1 = 0;
  int16_t X1 = 0;
  int16_t Y1 = 0;
  int8_t SW2 = 0;
  int16_t X2 = 0;
  int16_t Y2 = 0;
} __attribute__((packed));

// Abstract updateable interface
class Updatable {
public:
  virtual void update() = 0;
  virtual bool isChanged() = 0;
  virtual ~Updatable() {}
};

// Button base class
class Button : public Updatable {
protected:
  int pin;
  const char* name;
  bool lastState;
  int8_t* value;
  Updatable* parent;

public:
  Button(int pin, const char* name, int8_t* value, Updatable* parent)
    : pin(pin), name(name), lastState(HIGH), value(value), parent(parent) {
    pinMode(pin, INPUT_PULLUP);
  }

  bool isChanged() {
    return *value != 0;
  }

  void update() override {
    bool currentState = digitalRead(pin);
    *value = retrieveAction(currentState);
    lastState = currentState;
  }

  int retrieveAction(bool current) {
    if (isReleased(current)) return 3;      // Released
    else if (isPressed(current)) return 2;  // Pressed
    else if (isHold(current)) return 1;     // Hold
    else return 0;                          // Unhold
  }

  bool isPressed(bool state) {
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
class Joystick : public Updatable {
private:
  int xPin, yPin;
  const char* name;
  int16_t* xValue;
  int16_t* yValue;
  Button* swButton;
  Updatable* parent;

public:
  Joystick(char* name, int xPin, int yPin, int swPin, int16_t* xValue, int16_t* yValue, int8_t* swValue, Updatable* parent)
    : name(name), xPin(xPin), yPin(yPin), xValue(xValue), yValue(yValue), parent(parent) {
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);

    char swName[10];
    snprintf(swName, sizeof(swName), "%s_SW", name);
    swButton = new Button(swPin, swName, swValue, this);
  }

  ~Joystick() {
    delete swButton;
  }

  virtual bool isChanged() {
    bool isXMoved = (*xValue < floorBound) || (*xValue > surfaceBound);
    bool isYMoved = (*yValue < floorBound) || (*yValue > surfaceBound);

    if (!isXMoved)
      *xValue = 0;

    if (!isYMoved)
      *yValue = 0;

    return isXMoved || isYMoved || this->swButton->isChanged();
  }

  void update() override {
    *xValue = analogRead(xPin) - 512;
    *yValue = analogRead(yPin) - 512;
    this->swButton->update();
  }
};

// SimpleJoyStickPad
class SimpleJoyStickPad : public Updatable {
private:

  JoyStickPadData data;

  static constexpr int BUTTON_COUNT = 6;
  Button* buttons[BUTTON_COUNT];

  Joystick* joystick1;
  Joystick* joystick2;

public:
  SimpleJoyStickPad(const int* pins) {

    const char* names[BUTTON_COUNT] = { "A", "B", "C", "D", "E", "F" };
    int8_t* referenceData[BUTTON_COUNT] = { &data.A, &data.B, &data.C, &data.D, &data.E, &data.F };
    for (int i = 0; i < BUTTON_COUNT; ++i) {
      buttons[i] = new Button(pins[i], names[i], referenceData[i], this);
    }

    joystick1 = new Joystick("JS1", pins[6], pins[7], pins[8], &data.X1, &data.Y1, &data.SW1, this);
    joystick2 = new Joystick("JS2", pins[9], pins[10], pins[11], &data.X2, &data.Y2, &data.SW2, this);
  }

  ~SimpleJoyStickPad() {
    for (int i = 0; i < BUTTON_COUNT; i++) delete buttons[i];
    delete joystick1;
    delete joystick2;
  }

  void update() {
    for (int i = 0; i < BUTTON_COUNT; i++) buttons[i]->update();
    joystick1->update();
    joystick2->update();
  }

  bool isChanged() {
    bool changed = false;
    for (int i = 0; i < BUTTON_COUNT; i++) changed = buttons[i]->isChanged() || changed;
    changed = joystick1->isChanged() || changed;
    changed = joystick2->isChanged() || changed;
    return changed;
  }

public:
  JoyStickPadData getData() {
    return data;
  }
};

// ====== GLOBALS ======
RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "00001";

const int padPins[] = { 2, 3, 4, 5, 6, 7, A0, A1, A2, A3, A4, A5 };
const int ledStatus = 8;

SimpleJoyStickPad simplePad(padPins);

void setup() {
  Serial.begin(9600);

  pinMode(ledStatus, OUTPUT);
  digitalWrite(ledStatus, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.stopListening();
}

void loop() {
  simplePad.update();
  if (simplePad.isChanged()) {
    bool r = send(simplePad.getData());
    if (r) {
      digitalWrite(ledStatus, HIGH);
    } else {
      digitalWrite(ledStatus, LOW);
    }
  }
  digitalWrite(LED_BUILTIN, HIGH);
  delay(20);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(ledStatus, LOW);
}

bool send(const JoyStickPadData& data) {
  bool r = radio.write(&data, sizeof(data));

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
  return r;
}
