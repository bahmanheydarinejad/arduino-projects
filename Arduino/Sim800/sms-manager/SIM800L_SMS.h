#ifndef SIM800L_SMS_h
#define SIM800L_SMS_h

#include <SoftwareSerial.h>

class SIM800L_SMS {
public:
  SIM800L_SMS(int rxPin, int txPin, long baudRate);

  // Initialization method
  bool init();

  // Network and Signal
  bool isNetworkAvailable();
  int getSignalStrength();
  void printNetworkInfo();

  // SMS Management
  bool sendSMS(const char* phoneNumber, const char* utf8Message, int retryCount = 3);
  bool sendSMS_UTF8(const char* phoneNumber, const char* utf8Message, int retryCount = 3);
  bool checkForIncomingSMS();
  String readSMS(int index);
  bool deleteSMS(int index);
  bool deleteAllSMS();

  // Message Parsing and Command Handling
  bool checkForCommand(const String& command);

private:
  SoftwareSerial sim800Serial;
  int rxPin, txPin;
  long baudRate;

  // Customizable timeout for AT responses
  int timeout = 5000;

  // Helper methods for AT commands
  void sendATCommand(const char* command);
  String readResponse();
  bool checkResponse(const char* expectedResponse, int timeoutMs = 5000);

  // Utility to handle SMS storage on SIM800L
  void clearBuffer(char* buffer, int length);

  // Helper method to convert UTF-8 to UCS2
  String utf8ToUcs2(const char* utf8Message);
};

#endif
