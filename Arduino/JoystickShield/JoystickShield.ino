#include <Arduino.h>

const int joystickPrecision = 10;
const int floorBound = 512 + joystickPrecision;
const int surfaceBound = 512 - joystickPrecision;

// Data structure to store states
struct SimpleJoyStickPadData {
  int A, B, C, D, E, F, SW, UNKNOWN;
  int X, Y;
};

enum ButtonState {
  UNHOLD = 0,
  HOLD = 1,
  PRESSED = 2,
  RELEASED = 3
};

// Abstract base for buttons
class Button {
protected:
  int pin;
  bool lastState;
  int* refValue;

public:
  Button(int pin, int* ref) : pin(pin), lastState(HIGH), refValue(ref) {
    pinMode(pin, INPUT_PULLUP);
    *refValue = UNHOLD;
  }

  void update() {
    bool state = digitalRead(pin);

    if (lastState == HIGH && state == LOW) {
      *refValue = PRESSED;
    } else if (lastState == LOW && state == HIGH) {
      *refValue = RELEASED;
    } else if (lastState == LOW && state == LOW) {
      *refValue = HOLD;
    } else {
      *refValue = UNHOLD;
    }

    lastState = state;
  }
};

// Joystick class
class Joystick {
private:
  int xPin, yPin;
  int* xRef;
  int* yRef;

public:
  Joystick(int x, int y, int* xPtr, int* yPtr) : xPin(x), yPin(y), xRef(xPtr), yRef(yPtr) {
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
    *xRef = 0;
    *yRef = 0;
  }

  void update() {
    int rawX = analogRead(xPin);
    int rawY = analogRead(yPin);
    *xRef = map(rawX, 0, 1023, -1024, 1024);
    *yRef = map(rawY, 0, 1023, -1024, 1024);
  }
};

// Main Pad Controller
class SimpleJoyStickPad {
private:
  Button* buttons[8];
  Joystick* joystick;
  SimpleJoyStickPadData data;

public:
  SimpleJoyStickPad(
    int btnA, int btnB, int btnC, int btnD,
    int btnE, int btnF, int btnSW, int btnUnknown,
    int joyX, int joyY
  ) {
    buttons[0] = new Button(btnA, &data.A);
    buttons[1] = new Button(btnB, &data.B);
    buttons[2] = new Button(btnC, &data.C);
    buttons[3] = new Button(btnD, &data.D);
    buttons[4] = new Button(btnE, &data.E);
    buttons[5] = new Button(btnF, &data.F);
    buttons[6] = new Button(btnSW, &data.SW);
    buttons[7] = new Button(btnUnknown, &data.UNKNOWN);

    joystick = new Joystick(joyX, joyY, &data.X, &data.Y);
  }

  void action() {
    for (int i = 0; i < 8; ++i) {
      buttons[i]->update();
    }
    joystick->update();
  }

  SimpleJoyStickPadData getSimpleJoyStickPadData() {
    action();
    return data;
  }
};

// Define input pins
const int BTN_A = 2;
const int BTN_B = 3;
const int BTN_C = 4;
const int BTN_D = 5;
const int BTN_E = 6;
const int BTN_F = 7;
const int BTN_SW = 8;
const int BTN_UNKNOWN = 9;
const int JOY_X = A0;
const int JOY_Y = A1;

SimpleJoyStickPad pad(BTN_A, BTN_B, BTN_C, BTN_D, BTN_E, BTN_F, BTN_SW, BTN_UNKNOWN, JOY_X, JOY_Y);

void setup() {
  Serial.begin(9600);
}

void loop() {
  SimpleJoyStickPadData data = pad.getSimpleJoyStickPadData();

  Serial.print("Buttons: A=");
  Serial.print(data.A);
  Serial.print(" B=");
  Serial.print(data.B);
  Serial.print(" C=");
  Serial.print(data.C);
  Serial.print(" D=");
  Serial.print(data.D);
  Serial.print(" E=");
  Serial.print(data.E);
  Serial.print(" F=");
  Serial.print(data.F);
  Serial.print(" SW=");
  Serial.print(data.SW);
  Serial.print(" UNKNOWN=");
  Serial.print(data.UNKNOWN);

  Serial.print(" | Joystick: X=");
  Serial.print(data.X);
  Serial.print(" Y=");
  Serial.println(data.Y);

  delay(100);
}
