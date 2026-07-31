const int joyX = A4;
const int joyY = A5;
const int joyButton = 7;

// DRV8833 (حتماً PWM pins واقعی باشن)
const int AIN1 = 3;
const int AIN2 = 5;

const int BIN1 = 6;
const int BIN2 = 9;

const int STBY = 8;


// timing
unsigned long lastCycle = 0;
const unsigned long interval = 50;


// joystick
const int DEADZONE = 15;


// standby
bool enabled = true;
bool lastButton = HIGH;


// output
int pwmA = 0;
int pwmB = 0;

int axisX = 0;
int axisY = 0;


void setup() {

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);

  pinMode(joyButton, INPUT_PULLUP);

  digitalWrite(STBY, HIGH);

  Serial.begin(115200);
}



void loop() {

  if (millis() - lastCycle >= interval) {

    lastCycle = millis();


    // toggle
    bool btn = digitalRead(joyButton);

    if (btn == LOW && lastButton == HIGH) {
      enabled = !enabled;
    }

    lastButton = btn;


    int rawX = analogRead(joyX);
    int rawY = analogRead(joyY);


    // map to -512..+512
    axisX = map(rawX, 0, 1023, -512, 512);
    axisY = map(rawY, 0, 1023, -512, 512);


    pwmA = 0;
    pwmB = 0;


    if (enabled) {

      digitalWrite(STBY, HIGH);


      // ===== Motor A =====
      if (abs(axisX) <= DEADZONE) {
        stopMotorA();
      } else {
        pwmA = map(abs(axisX), DEADZONE, 512, 0, 255);
        setMotorA(pwmA, axisX > 0);
      }


      // ===== Motor B =====
      if (abs(axisY) <= DEADZONE) {
        stopMotorB();
      } else {
        pwmB = map(abs(axisY), DEADZONE, 512, 0, 255);
        setMotorB(pwmB, axisY > 0);
      }

    } else {
      disableMotors();
    }


    // ===== DEBUG =====
    Serial.print("EN:");
    Serial.print(enabled);

    Serial.print(" RAW:");
    Serial.print(rawX);
    Serial.print(",");
    Serial.print(rawY);

    Serial.print(" AX:");
    Serial.print(axisX);
    Serial.print(",");
    Serial.print(axisY);

    Serial.print(" PWM:");
    Serial.print(pwmA);
    Serial.print(",");
    Serial.print(pwmB);

    Serial.print(" OUTA(");
    Serial.print(pwmA);
    Serial.print(",");
    Serial.print(255 - pwmA);
    Serial.print(")");

    Serial.print(" OUTB(");
    Serial.print(pwmB);
    Serial.print(",");
    Serial.print(255 - pwmB);
    Serial.print(")");

    Serial.println();
  }
}



// ===== MOTOR A =====
void setMotorA(int pwm, bool forward) {
  pwm = constrain(pwm, 0, 255);

  if (forward) {
    analogWrite(AIN1, pwm);
    analogWrite(AIN2, 255 - pwm);
  } else {
    analogWrite(AIN1, 255 - pwm);
    analogWrite(AIN2, pwm);
  }
}

void stopMotorA() {
  analogWrite(AIN1, 0);
  analogWrite(AIN2, 0);
}



// ===== MOTOR B =====
void setMotorB(int pwm, bool forward) {
  pwm = constrain(pwm, 0, 255);

  if (forward) {
    analogWrite(BIN1, pwm);
    analogWrite(BIN2, 255 - pwm);
  } else {
    analogWrite(BIN1, 255 - pwm);
    analogWrite(BIN2, pwm);
  }
}

void stopMotorB() {
  analogWrite(BIN1, 0);
  analogWrite(BIN2, 0);
}



// ===== DISABLE =====
void disableMotors() {
  digitalWrite(STBY, LOW);

  stopMotorA();
  stopMotorB();
}
