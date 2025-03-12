const int touchPin = 8;  // پین ماژول تاچ
const int txPin = 1;     // پین TX آردوینو

void setup() {
  pinMode(touchPin, INPUT);  // تنظیم پین تاچ به‌عنوان ورودی
  pinMode(txPin, OUTPUT);    // تنظیم پین TX به‌عنوان خروجی
  digitalWrite(txPin, LOW);  // در ابتدا LED خاموش باشد
}

void loop() {
  int touchState = digitalRead(touchPin);  // خواندن مقدار ماژول تاچ

  if (touchState == HIGH) {
    digitalWrite(txPin, HIGH);  // LED را روشن کن
  } else {
    digitalWrite(txPin, LOW);  // LED را خاموش کن
  }
}
