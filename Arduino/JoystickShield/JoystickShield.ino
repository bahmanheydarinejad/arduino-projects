#include <Arduino.h>

// Precision for joystick neutrality
const int joystickPrecision = 10;
const int floorBound = 512 + joystickPrecision;
const int surfaceBound = 512 - joystickPrecision;

// Struct to hold all input states
struct JoyStickPadData {
  int A = 0;
  int B = 0;
  int C = 0;
  int D = 0;
  int E = 0;
  int F = 0;
  int SW = 0;
  int UNKNOWN = 0;
  int X = 0;
  int Y = 0;
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

class MyPrintable {
public:
  virtual void print() = 0;
  virtual ~MyPrintable() {}
};

// Forward declaration
class SimpleJoyStickPad;

// Button base class
class Button : public MyUpdatable, public MyActionable {
protected:
  int pin;
  const char* name;
  bool lastState;
  int* targetAction;
  MyPrintable* parent;

public:
  Button(int p, const char* n, int* target, MyPrintable* parent)
    : pin(p), name(n), lastState(HIGH), targetAction(target), parent(parent) {
    pinMode(pin, INPUT_PULLUP);
  }

  void update() {
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

  void action() {
    if (parent && *targetAction != 0) parent->print();
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
  int* xTarget;
  int* yTarget;
  MyPrintable* parent;

public:
  Joystick(int x, int y, const char* n, int* xT, int* yT, MyPrintable* parent)
    : xPin(x), yPin(y), name(n), xTarget(xT), yTarget(yT), parent(parent) {
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
  }

  bool isJoystickMoved(int xValue, int yValue) {
    bool isXMoved = xValue < surfaceBound || xValue > floorBound;
    bool isYMoved = yValue < surfaceBound || yValue > floorBound;
    return isXMoved || isYMoved;
  }

  void update() override {
    int xVal = analogRead(xPin);
    int yVal = analogRead(yPin);
    // *xTarget = map(xVal, 0, 1023, -1024, 1024);
    // *yTarget = map(yVal, 0, 1023, -1024, 1024);
    *xTarget = xVal;
    *yTarget = yVal;
  }

  void action() {
    bool isMoved = isJoystickMoved(*xTarget, *yTarget);
    if (isMoved && parent) parent->print();
  }
};

// Forward declaration for SimpleJoyStickPad
class SimpleJoyStickPad : public MyPrintable {
private:
  JoyStickPadData data;

  Button* buttons[8];
  Joystick* joystick;

public:
  SimpleJoyStickPad() {
    buttons[0] = new Button(2, "A", &data.A, this);
    buttons[1] = new Button(3, "B", &data.B, this);
    buttons[2] = new Button(4, "C", &data.C, this);
    buttons[3] = new Button(5, "D", &data.D, this);
    buttons[4] = new Button(6, "E", &data.E, this);
    buttons[5] = new Button(7, "F", &data.F, this);
    buttons[6] = new Button(8, "SW", &data.SW, this);
    buttons[7] = new Button(9, "UNKNOWN", &data.UNKNOWN, this);

    joystick = new Joystick(A0, A1, "Joystick", &data.X, &data.Y, this);
  }

  ~SimpleJoyStickPad() {
    for (int i = 0; i < 8; i++) delete buttons[i];
    delete joystick;
  }

  void update() {
    for (int i = 0; i < 8; i++) buttons[i]->update();
    joystick->update();
  }

  JoyStickPadData getSimpleJoyStickPadData() {
    update();  // Always updates data before returning
    return data;
  }

  void print() {
    Serial.print("A:");
    Serial.print(data.A);
    Serial.print("\tB:");
    Serial.print(data.B);
    Serial.print("\tC:");
    Serial.print(data.C);
    Serial.print("\tD:");
    Serial.print(data.D);
    Serial.print("\tE:");
    Serial.print(data.E);
    Serial.print("\tF:");
    Serial.print(data.F);
    Serial.print("\tSW:");
    Serial.print(data.SW);
    Serial.print("\tUNK:");
    Serial.print(data.UNKNOWN);
    Serial.print("\tX:");
    Serial.print(data.X);
    Serial.print("\tY:");
    Serial.println(data.Y);
  }
};

// Create the joystick pad instance
SimpleJoyStickPad simplePad;

void setup() {
  Serial.begin(9600);
}

void loop() {
  simplePad.update();
  delay(10);  // Slight delay to avoid flooding
}
