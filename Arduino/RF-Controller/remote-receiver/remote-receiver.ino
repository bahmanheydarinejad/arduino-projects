#define TINY_GSM_MODEM_SIM800
#include <SoftwareSerial.h>

// Define the RX and TX pins for the SIM800L module
SoftwareSerial sim800L(10, 11);  // RX, TX

void setup() {
  // Start the serial communication with the SIM800L and the Serial Monitor
  Serial.begin(9600);
  sim800L.begin(9600);

  // Set the SIM800L to text mode
  setTextMode();

  // Example: Send an SMS
  sendSMS("+989354415015", "Sim800L started.");
}

void loop() {
  // Example: Read received SMS
  readSMS();

  delay(1000);  // Wait for 30 seconds before next operation
}

// Method to set the SIM800L to text mode
void setTextMode() {
  sim800L.println("AT+CMGF=1");  // Set text mode
  delay(100);
  readResponse();
}

// Method to send an SMS
void sendSMS(const char* phoneNumber, const char* message) {
  sim800L.print("AT+CMGS=\"");
  sim800L.print(phoneNumber);
  sim800L.println("\"");
  delay(100);

  // Send the message
  sim800L.println(message);
  delay(100);
  sim800L.write(26);  // Send Ctrl+Z
  delay(100);
  readResponse();
}

// Method to read received SMS
void readSMS() {
  sim800L.println("AT+CMGL=\"REC UNREAD\"");  // Read all unread SMS
  delay(100);
  readResponse();
}

// Method to read the response from SIM800L
void readResponse() {
  while (sim800L.available()) {
    Serial.write(sim800L.read());  // Print response to Serial Monitor
  }
  while (Serial.available()) {
    Serial.write(Serial.read());  // Echo Serial Monitor input
  }
}
