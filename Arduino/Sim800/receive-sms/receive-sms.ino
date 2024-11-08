#include <SoftwareSerial.h>

// تعریف پین‌های RX و TX ماژول SIM800L
const int SIM800_RX_PIN = 10;  // ماژول SIM800L را به پین 10 برد Arduino متصل کنید
const int SIM800_TX_PIN = 11;  // ماژول SIM800L را به پین 11 برد Arduino متصل کنید

SoftwareSerial sim800lSerial(SIM800_RX_PIN, SIM800_TX_PIN);

void setup() {
  // شروع ارتباط Serial برای اشکال زدایی
  Serial.begin(9600);

  // شروع ارتباط SoftwareSerial با ماژول SIM800L
  sim800lSerial.begin(9600);

  delay(1000);

  // مقداردهی اولیه به ماژول SIM800L
  sendCommand("AT");
  delay(1000);
  sendCommand("AT+CMGF=1");  // تنظیم حالت متنی پیامک (SMS)
  delay(1000);
}

void loop() {
  // بررسی دریافت پیام
  if (sim800lSerial.available()) {
    String message = sim800lSerial.readString();
    Serial.print("پیام دریافت شده: ");
    Serial.println(message);
  }
}

void sendCommand(String command) {
  sim800lSerial.println(command);
  delay(1000);

  while (sim800lSerial.available()) {
    String response = sim800lSerial.readString();
    Serial.println(response);
  }
}
