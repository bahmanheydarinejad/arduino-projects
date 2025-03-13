#include <Arduino.h>

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

// کلاس پایه برای دکمه‌ها
class Button {
protected:
  int pin;
  const char* name;
  bool lastState;

public:
  Button(int p, const char* n) : pin(p), name(n), lastState(HIGH) {
    pinMode(pin, INPUT_PULLUP);
  }

  virtual bool isPressed() {
    return digitalRead(pin) == LOW;
  }

  const char* getName() {
    return name;
  }

  virtual void performAction() {
    Serial.print(name);
    Serial.println(" button pressed");
  }

  void checkAndAct() {
    bool currentState = isPressed();
    if (currentState == LOW && lastState == HIGH) {
      performAction();
    }
    lastState = currentState;
  }
};

// کلاس برای دکمه‌ها
class AButton : public Button {
public:
  AButton() : Button(A, "A") {}
};

class BButton : public Button {
public:
  BButton() : Button(B, "B") {}
};

class CButton : public Button {
public:
  CButton() : Button(C, "C") {}
};

class DButton : public Button {
public:
  DButton() : Button(D, "D") {}
};

class EButton : public Button {
public:
  EButton() : Button(E, "E") {}
};

class FButton : public Button {
public:
  FButton() : Button(F, "F") {}
};

class SWButton : public Button {
public:
  SWButton() : Button(SW, "SW") {}
};

class UnknownButton : public Button {
public:
  UnknownButton() : Button(UNKNOWN, "UNKNOWN") {}
};

// کلاس برای جوی‌استیک
class Joystick {
private:
  int xPin, yPin;
  const char* name;
  int lastX, lastY;

public:
  Joystick(int x, int y, const char* n) : xPin(x), yPin(y), name(n), lastX(0), lastY(0) {
    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
  }

  void checkPosition() {
    int xValue = analogRead(xPin);
    int yValue = analogRead(yPin);

    if (xValue != lastX || yValue != lastY) {
      Serial.print(name);
      Serial.print(" - X: ");
      Serial.print(xValue);
      Serial.print(", Y: ");
      Serial.println(yValue);

      lastX = xValue;
      lastY = yValue;
    }
  }
};

// مدیریت دکمه‌ها و جوی‌استیک
class ButtonManager {
private:
  Button* buttons[8];
  Joystick* joystick;

public:
  ButtonManager() {
    buttons[0] = new AButton();
    buttons[1] = new BButton();
    buttons[2] = new CButton();
    buttons[3] = new DButton();
    buttons[4] = new EButton();
    buttons[5] = new FButton();
    buttons[6] = new SWButton();
    buttons[7] = new UnknownButton();
    joystick = new Joystick(JOY_X, JOY_Y, "Joystick");
  }

  void checkButtons() {
    for (int i = 0; i < 8; i++) {
      buttons[i]->checkAndAct();
    }
  }

  void checkJoystick() {
    joystick->checkPosition();
  }
};

ButtonManager buttonManager;

void setup() {
  Serial.begin(9600);
}

void loop() {
  buttonManager.checkButtons();
  buttonManager.checkJoystick();
  delay(30); // برای کاهش نویز
}
