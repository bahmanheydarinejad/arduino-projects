#include <SoftwareSerial.h>
#include "SIM800L.h"

#define SIM800_RX_PIN 10  // Change if needed
#define SIM800_TX_PIN 11  // Change if needed
#define SIM800_RST_PIN 9

SIM800L* sim800l;
SoftwareSerial sim800Serial(SIM800_RX_PIN, SIM800_TX_PIN);  // RX, TX

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);
  while (!Serial)
    ;

  // Initialize SoftwareSerial for SIM800L
  sim800Serial.begin(9600);
  delay(1000);

  // Initialize SIM800L driver with an internal buffer of 200 bytes and a reception buffer of 512 bytes, debug disabled
  sim800l = new SIM800L((Stream*)&sim800Serial, SIM800_RST_PIN, 200, 512);

  // Equivalent line with debug enabled on the Serial
  // sim800l = new SIM800L((Stream *)&sim800Serial, SIM800_RST_PIN, 200, 512, (Stream *)&Serial);

  Serial.println("Start of test protocol");

  // Wait until the module is ready to accept AT commands
  while (!sim800l->isReady()) {
    Serial.println(F("Problem initializing AT command, retry in 1 sec"));
    delay(1000);
  }

  // Enable echo mode (if required for some modules)
  sim800l->enableEchoMode();

  Serial.println("Module ready");

  // Print version
  Serial.print("Module ");
  Serial.println(sim800l->getVersion());
  Serial.print("Firmware ");
  Serial.println(sim800l->getFirmware());

  // Print SIM card status
  Serial.print(F("SIM status "));
  Serial.println(sim800l->getSimStatus());

  // Print SIM card number
  Serial.print("SIM card number ");
  Serial.println(sim800l->getSimCardNumber());

  // Wait for the GSM signal
  uint8_t signal = sim800l->getSignal();
  while (signal <= 0) {
    delay(1000);
    signal = sim800l->getSignal();
  }

  if (signal > 5) {
    Serial.print(F("Signal OK (strength: "));
  } else {
    Serial.print(F("Signal low (strength: "));
  }
  Serial.print(signal);
  Serial.println(F(")"));
  delay(1000);

  // Wait for operator network registration (national or roaming network)
  NetworkRegistration network = sim800l->getRegistrationStatus();
  while (network != REGISTERED_HOME && network != REGISTERED_ROAMING) {
    delay(1000);
    network = sim800l->getRegistrationStatus();
  }
  Serial.println(F("Network registration OK"));

  Serial.println("End of test protocol");
}

void loop() {
}
