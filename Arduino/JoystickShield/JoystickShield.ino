#include <Arduino.h>

const int joystickPrecision = 10;

const int floorBound = 512 + joystickPrecision;
const int surfaceBound = 512 - joystickPrecision;


// تعریف پین‌های دکمه‌ها و جوی‌استیک
const int A = 2;
const int B = 3;
const int C = 4;
const int D = 5;
const int E = 6;
const int F = 7;
const int SW = 8;
const int UNKNOWN = 9;
const int JOY_X = A0;  // محور X جوی‌استیک
const int JOY_Y = A1;  // محور Y جوی‌استیک

// کلاس برای جوی‌استیک
class Joystick {
private:
  int xPin, yPin;
  const char* name;
  int lastX, lastY;

public:
  Joystick(int x, int y, const char* n)
    : xPin(x), yPin(y), name(n), lastX(0), lastY(0) {
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
  }

  void checkPosition() {
    int xValue = analogRead(xPin);  // صفر کردن چند بیت آخر
    int yValue = analogRead(yPin);  // صفر کردن چند بیت آخر

    bool isMoved = joystickMoved(xValue, yValue);

    if (isMoved) {
      Serial.print(name);
      Serial.print(" - X: ");
      Serial.print(xValue);
      Serial.print(", Y: ");
      Serial.println(yValue);
    }

    // lastX = xValue;
    // lastY = yValue;
  }

  bool joystickMoved(int xValue, int yValue) {
    bool isXMoved = xValue < surfaceBound || xValue > floorBound;
    bool isYMoved = yValue < surfaceBound || yValue > floorBound;
    return isXMoved || isYMoved;
  }
};

// کلاس پایه برای دکمه‌ها
class Button {
protected:
  int pin;
  const char* name;
  bool lastState;

public:
  Button(int p, const char* n)
    : pin(p), name(n), lastState(HIGH) {
    pinMode(pin, INPUT_PULLUP);
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

  const char* getName() {
    return name;
  }

  virtual void presseAction() {
    Serial.print(name);
    Serial.println(" button Pressed");
  }

  virtual void releaseAction() {
    Serial.print(name);
    Serial.println(" button Released");
  }

  virtual void holdAction() {
    Serial.print(name);
    Serial.println(" button Hold");
  }

  void checkAndAct() {
    bool currentState = digitalRead(pin);
    if (isPressed(currentState)) {
      presseAction();
    }
    if (isReleased(currentState)) {
      releaseAction();
    }
    if (isHold(currentState)) {
      holdAction();
    }
    lastState = currentState;
  }
};

// کلاس برای دکمه‌ها
class AButton : public Button {
public:
  AButton()
    : Button(A, "A") {}
};

class BButton : public Button {
public:
  BButton()
    : Button(B, "B") {}
};

class CButton : public Button {
public:
  CButton()
    : Button(C, "C") {}
};

class DButton : public Button {
public:
  DButton()
    : Button(D, "D") {}
};

class EButton : public Button {
public:
  EButton()
    : Button(E, "E") {}
};

class FButton : public Button {
public:
  FButton()
    : Button(F, "F") {}
};

class SWButton : public Button {
public:
  SWButton()
    : Button(SW, "SW") {}
};

class UnknownButton : public Button {
public:
  UnknownButton()
    : Button(UNKNOWN, "UNKNOWN") {}
};

// مدیریت دکمه‌ها و جوی‌استیک
class ButtonManager {
private:
  Joystick* joystick;
  Button* buttons[8];

public:
  ButtonManager() {
    joystick = new Joystick(JOY_X, JOY_Y, "Joystick");
    buttons[0] = new AButton();
    buttons[1] = new BButton();
    buttons[2] = new CButton();
    buttons[3] = new DButton();
    buttons[4] = new EButton();
    buttons[5] = new FButton();
    buttons[6] = new SWButton();
    buttons[7] = new UnknownButton();
  }

  void checkJoystick() {
    joystick->checkPosition();
  }

  void checkButtons() {
    for (int i = 0; i < 8; i++) {
      buttons[i]->checkAndAct();
    }
  }
};

ButtonManager buttonManager;

void setup() {
  Serial.begin(9600);
}

void loop() {
  buttonManager.checkButtons();
  buttonManager.checkJoystick();
  delay(10);  // برای کاهش نویز
}
