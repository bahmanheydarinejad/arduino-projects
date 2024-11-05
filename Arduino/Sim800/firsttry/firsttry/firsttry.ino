#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <SoftwareSerial.h>

#define MODEM_RST 5
#define MODEM_PWRKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 10  // arduino port
#define MODEM_RX 11  // arduino port

// Your SIM card credentials
const char apn[] = "mcinet";  // Replace with your network provider's APN
const char user[] = "";
const char pass[] = "";

SoftwareSerial SerialAT(MODEM_TX, MODEM_RX);
TinyGsm modem(SerialAT);

void setup() {
  // Set up serial communication
  Serial.begin(9600);
  delay(10);

  // Set up modem
  SerialAT.begin(9600);
  delay(50);

  // Restart takes quite some time
  modem.restart();
  Serial.println("Modem initialized.");

  // Test sending SMS
  modem.sendSMS("09354415015", "Hello from SIM800L!"); 
  Serial.println("SMS sent.");
}

void loop() {
  
}