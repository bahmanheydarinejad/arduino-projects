#include "SIM800L_SMS.h"

SIM800L_SMS smsModule(10, 11, 9600);  // RX pin 10, TX pin 11

void setup() {
  Serial.begin(9600);
  if (smsModule.init()) {
    Serial.println("SIM800L Initialized!");
  } else {
    Serial.println("Failed to initialize SIM800L.");
  }
}

void loop() {
  const char* phoneNumber = "+989354415015";
  const char* utf8Message = "سلام، این یک پیام تست است";  // Persian UTF-8 message example

  if (smsModule.sendSMS_UTF8(phoneNumber, utf8Message, 3)) {
    Serial.println("UTF-8 SMS Sent Successfully!");
  } else {
    Serial.println("Failed to send UTF-8 SMS.");
  }

  delay(10000);  // Send message every 10 seconds (for testing)
}
