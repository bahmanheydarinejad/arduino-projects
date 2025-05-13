#include <SoftwareSerial.h>

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(10, 11);  //SIM800L Tx & Rx is connected to Arduino #3 & #2

void setup() {
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);

  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Initializing...");
  delay(1000);

  Serial.println("Once the handshake test is successful, it will back to OK");
  mySerial.println("AT");  //Once the handshake test is successful, it will back to OK
  updateSerial();
  Serial.println("Signal quality test, value range is 0-31 , 31 is the best");
  mySerial.println("AT+CSQ");  //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  Serial.println("Read SIM information to confirm whether the SIM is plugged");
  mySerial.println("AT+CCID");  //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  Serial.println("Check whether it has registered in the network");
  mySerial.println("AT+CREG?");  //Check whether it has registered in the network
  updateSerial();
}

void loop() {
  updateSerial();
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    mySerial.write(Serial.read());  //Forward what Serial received to Software Serial Port
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());  //Forward what Software Serial received to Serial Port
  }
}

String decodeUCS2(String ucs2) {
  String decoded = "";
  for (int i = 0; i < ucs2.length(); i += 4) {
    // Take 4 hex characters (2 bytes of UCS2)
    String hexChar = ucs2.substring(i, i + 4);
    int unicodeValue = strtol(hexChar.c_str(), NULL, 16);  // Convert hex to integer

    // Append the Unicode character to the decoded string
    decoded += (char16_t)unicodeValue;  // Use char16_t for 2-byte UCS2 character
  }
  return decoded;
}
