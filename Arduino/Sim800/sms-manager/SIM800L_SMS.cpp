#include <Arduino.h>
#include "SIM800L_SMS.h"

SIM800L_SMS::SIM800L_SMS(int rx, int tx, long baud)
  : sim800Serial(rx, tx), rxPin(rx), txPin(tx), baudRate(baud) {}

bool SIM800L_SMS::init() {
  sim800Serial.begin(baudRate);
  delay(1000);

  sendATCommand("AT");
  if (!checkResponse("OK", timeout)) return false;

  sendATCommand("AT+CMGF=1");         // Set to text mode
  sendATCommand("AT+CSCS=\"UCS2\"");  // Set character set to UCS2
  return checkResponse("OK", timeout);
}

bool SIM800L_SMS::isNetworkAvailable() {
  sendATCommand("AT+CREG?");
  return checkResponse("+CREG: 0,1", timeout) || checkResponse("+CREG: 0,5", timeout);
}

int SIM800L_SMS::getSignalStrength() {
  sendATCommand("AT+CSQ");
  String response = readResponse();
  if (response.indexOf("+CSQ: ") != -1) {
    int signal = response.substring(response.indexOf(":") + 2, response.indexOf(",")).toInt();
    return signal;  // Returns signal strength, -1 if not found
  }
  return -1;
}

void SIM800L_SMS::printNetworkInfo() {
  Serial.print("Network Status: ");
  Serial.println(isNetworkAvailable() ? "Connected" : "Disconnected");

  Serial.print("Signal Strength: ");
  int signal = getSignalStrength();
  if (signal >= 0) {
    Serial.println(String(signal) + " dBm");
  } else {
    Serial.println("Unknown");
  }
}

bool SIM800L_SMS::sendSMS(const char* phoneNumber, const char* message, int retryCount) {
  for (int attempt = 0; attempt < retryCount; ++attempt) {
    sim800Serial.print("AT+CMGS=\"");
    sim800Serial.print(phoneNumber);
    sim800Serial.println("\"");

    delay(100);
    sim800Serial.print(message);
    delay(100);
    sim800Serial.write(26);  // Send Ctrl+Z to send the message

    if (checkResponse("OK", timeout)) {
      return true;  // Message sent successfully
    }
  }
  return false;  // Message failed after retries
}

bool SIM800L_SMS::checkForIncomingSMS() {
  sendATCommand("AT+CMGL=\"REC UNREAD\"");
  return checkResponse("+CMGL:", timeout);
}

String SIM800L_SMS::readSMS(int index) {
  String command = "AT+CMGR=" + String(index);
  sendATCommand(command.c_str());
  String response = readResponse();
  if (response.indexOf("+CMGR:") != -1) {
    return response;
  }
  return "";
}

bool SIM800L_SMS::deleteSMS(int index) {
  String command = "AT+CMGD=" + String(index);
  sendATCommand(command.c_str());
  return checkResponse("OK", timeout);
}

bool SIM800L_SMS::deleteAllSMS() {
  sendATCommand("AT+CMGDA=\"DEL ALL\"");
  return checkResponse("OK", timeout);
}

bool SIM800L_SMS::checkForCommand(const String& command) {
  if (checkForIncomingSMS()) {
    for (int i = 1; i <= 10; i++) {  // Check first 10 messages (modify if needed)
      String smsContent = readSMS(i);
      if (smsContent.indexOf(command) != -1) {
        deleteSMS(i);  // Optionally delete after processing
        return true;
      }
    }
  }
  return false;
}

void SIM800L_SMS::sendATCommand(const char* command) {
  sim800Serial.println(command);
}

String SIM800L_SMS::readResponse() {
  String response = "";
  long timeoutStart = millis();
  while (millis() - timeoutStart < timeout) {
    if (sim800Serial.available()) {
      char c = sim800Serial.read();
      response += c;
    }
  }
  return response;
}

bool SIM800L_SMS::checkResponse(const char* expectedResponse, int timeoutMs) {
  String response = readResponse();
  return response.indexOf(expectedResponse) != -1;
}

void SIM800L_SMS::clearBuffer(char* buffer, int length) {
  for (int i = 0; i < length; i++) {
    buffer[i] = '\0';
  }
}

bool SIM800L_SMS::sendSMS_UTF8(const char* phoneNumber, const char* utf8Message, int retryCount) {
  // Convert UTF-8 message to UCS2 hex format
  String ucs2Message = utf8ToUcs2(utf8Message);

  // Set up SIM800L for UCS2 encoding and text mode for SMS
  sendATCommand("AT+CMGF=1");         // Set SMS to text mode
  sendATCommand("AT+CSCS=\"UCS2\"");  // Set encoding to UCS2

  for (int attempt = 0; attempt < retryCount; ++attempt) {
    // Use UCS2-encoded phone number if required, otherwise use standard format
    sim800Serial.print("AT+CMGS=\"");
    sim800Serial.print(phoneNumber);  // No conversion needed for phone number
    sim800Serial.println("\"");

    delay(100);  // Small delay after issuing the command

    // Send UCS2-encoded message
    sim800Serial.print(ucs2Message);
    delay(100);
    sim800Serial.write(26);  // Send Ctrl+Z to send the message

    if (checkResponse("OK", timeout)) {
      return true;  // Message sent successfully
    }
  }
  return false;  // Message failed after retries
}

// Helper methods remain the same (checkForIncomingSMS, readSMS, deleteSMS, etc.)

// UTF-8 to UCS2 Conversion Helper
String SIM800L_SMS::utf8ToUcs2(const char* utf8Message) {
  String ucs2Message = "";
  while (*utf8Message) {
    uint16_t ucs2Char = 0;
    if ((*utf8Message & 0x80) == 0) {
      // 1-byte UTF-8 character (ASCII)
      ucs2Char = *utf8Message++;
    } else if ((*utf8Message & 0xE0) == 0xC0) {
      // 2-byte UTF-8 character
      ucs2Char = (*utf8Message++ & 0x1F) << 6;
      ucs2Char |= (*utf8Message++ & 0x3F);
    } else if ((*utf8Message & 0xF0) == 0xE0) {
      // 3-byte UTF-8 character
      ucs2Char = (*utf8Message++ & 0x0F) << 12;
      ucs2Char |= (*utf8Message++ & 0x3F) << 6;
      ucs2Char |= (*utf8Message++ & 0x3F);
    }
    // Convert UCS2 character to hex and append to message
    ucs2Message += String(ucs2Char, HEX);
  }
  return ucs2Message;
}
