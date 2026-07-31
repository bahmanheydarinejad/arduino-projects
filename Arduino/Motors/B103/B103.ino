const int pot = A0;
const int pwmPin = 5;

unsigned long lastTime = 0;

// فیلتر
float filtered = 0;
const float alpha = 0.3;  // 0.05~0.2 → هرچی کمتر، نرم‌تر

int lastPwm = 0;
const int deadband = 2;  // حذف نوسان‌های کوچک

void setup() {
  pinMode(pwmPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if (millis() - lastTime >= 50) {
    lastTime = millis();

    int raw = analogRead(pot);  // 0-1023

    // --- Low-pass filter ---
    filtered = alpha * raw + (1 - alpha) * filtered;

    // --- map ---
    int pwm = map((int)filtered, 0, 1023, 0, 100);

    // --- deadband (حذف لرزش) ---
    if (abs(pwm - lastPwm) > deadband) {
      lastPwm = pwm;
      analogWrite(pwmPin, pwm);
    }

    // debug
    Serial.print("raw:");
    Serial.print(raw);
    Serial.print(" filt:");
    Serial.print((int)filtered);
    Serial.print(" pwm:");
    Serial.println(lastPwm);
  }
}