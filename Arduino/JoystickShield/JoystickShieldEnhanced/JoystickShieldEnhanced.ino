#include <SPI.h>
#include <RF24.h>
#include <string.h>

/* ============================================================
   CONFIGURATION
   ============================================================ */

#define DEVICE_IS_TRANSMITTER 1  // 1 = TX, 0 = RX

static const uint8_t CE_PIN = 9;
static const uint8_t CSN_PIN = 10;

static const uint8_t DEADZONE = 10;
static const uint8_t SMOOTH_FACTOR = 4;
static const uint16_t TX_RATE_MS = 5;  // 200Hz
static const uint16_t FAILSAFE_TIMEOUT_MS = 500;
static const uint16_t CENTER_POINT = 128;


/* ============================================================
   PACKET
   ============================================================ */

struct ControlPacket {
  uint8_t counter;  // packet counter (0–255) mixed with CRC by PacketCodec
  uint8_t joy1x;    // joystick 1 X axis
  uint8_t joy1y;    // joystick 1 Y axis
  uint8_t joy2x;    // joystick 2 X axis
  uint8_t joy2y;    // joystick 2 Y axis
  uint8_t range1;   // potentiometer / range control 1
  uint8_t range2;   // potentiometer / range control 2
  uint8_t buttons;  // 8 buttons (BTN1..BTN6 + SW1 + SW2)
};

/* ============================================================
   LOGGER (prints only when message changes)
   ============================================================ */

class Logger {
public:
  Logger(unsigned long baud = 115200) {
    Serial.begin(baud);
    delay(50);  // allow serial to stabilize
  }

  void log(const String &msg) {
    Serial.println(msg);
  }

  void logPacket(const ControlPacket &p) {
    Serial.print("PKT ");
    Serial.print(p.counter);

    Serial.print(" J1(");
    Serial.print(p.joy1x);
    Serial.print(",");
    Serial.print(p.joy1y);
    Serial.print(")");

    Serial.print(" J2(");
    Serial.print(p.joy2x);
    Serial.print(",");
    Serial.print(p.joy2y);
    Serial.print(")");

    Serial.print(" R(");
    Serial.print(p.range1);
    Serial.print(",");
    Serial.print(p.range2);
    Serial.print(")");

    Serial.print(" BCRC:");
    Serial.println(p.buttons);
  }
};

Logger logger;


/* ============================================================
   FILTERS
   ============================================================ */

class LowPassFilter {
private:
  uint8_t factor;
  int filtered;

public:
  LowPassFilter(uint8_t smoothFactor = 4)
    : factor(smoothFactor), filtered(0) {}

  void begin(int initialValue) {
    filtered = initialValue;
  }

  int process(int input) {
    filtered = ((long)filtered * (factor - 1) + input) / factor;
    return filtered;
  }

  int current() const {
    return filtered;
  }
};

class DeadZoneFilter {
private:
  uint8_t zone;
  uint8_t center;  // Add center variable

public:
  // Allow user to define center (default CENTER_POINT)
  DeadZoneFilter(uint8_t deadzone = DEADZONE, uint8_t centerPoint = CENTER_POINT)
    : zone(deadzone), center(centerPoint) {}

  uint8_t apply(uint8_t value) const {
    if (abs((int)value - center) < zone)
      return center;

    return value;
  }
};


/* ============================================================
   INPUT DEVICES
   ============================================================ */

class AnalogAxis {
private:
  uint8_t pin;
  LowPassFilter lowPass;
  DeadZoneFilter deadZone;

public:
  AnalogAxis()
    : pin(A0), lowPass(SMOOTH_FACTOR), deadZone(DEADZONE, CENTER_POINT) {}

  AnalogAxis(uint8_t analogPin)
    : pin(analogPin), lowPass(SMOOTH_FACTOR), deadZone(DEADZONE, CENTER_POINT) {}

  void attach(uint8_t analogPin) {
    pin = analogPin;
  }

  void begin() {
    int first = analogRead(pin);
    lowPass.begin(first);
  }

  void update() {
    int raw = analogRead(pin);
    lowPass.process(raw);
  }

  uint8_t get8bit() const {
    int filtered = lowPass.current();
    int scaled = filtered >> 2;

    if (scaled < 0) scaled = 0;
    if (scaled > 255) scaled = 255;

    return deadZone.apply((uint8_t)scaled);
  }
};

class DigitalButton {
private:
  uint8_t pin;
  bool state;
  bool previous;

public:
  DigitalButton()
    : pin(2), state(HIGH), previous(HIGH) {}

  DigitalButton(uint8_t digitalPin)
    : pin(digitalPin), state(HIGH), previous(HIGH) {}

  void attach(uint8_t digitalPin) {
    pin = digitalPin;
  }

  void begin() {
    pinMode(pin, INPUT_PULLUP);
    state = digitalRead(pin);
    previous = state;
  }

  void update() {
    previous = state;
    state = digitalRead(pin);
  }

  bool isPressed() const {
    return state == LOW;
  }

  bool isChanged() const {
    return state != previous;
  }
};

/* ============================================================
   JOYSTICK MODULE (2 AXES + SW BUTTON)
   ============================================================ */

class JoyInput {
private:
  AnalogAxis axisX;
  AnalogAxis axisY;
  DigitalButton sw;

public:
  JoyInput() {}

  JoyInput(uint8_t xPin, uint8_t yPin, uint8_t swPin)
    : axisX(xPin), axisY(yPin), sw(swPin) {}

  void begin() {
    axisX.begin();
    axisY.begin();
    sw.begin();
  }

  void update() {
    axisX.update();
    axisY.update();
    sw.update();
  }

  /* signed axis value (-128..127) */
  int8_t getSignedX() const {
    return (int16_t)axisX.get8bit() - 128;
  }

  int8_t getSignedY() const {
    return (int16_t)axisY.get8bit() - 128;
  }

  /* packet format (0..255) */
  uint8_t getPacketX() const {
    return axisX.get8bit();
  }

  uint8_t getPacketY() const {
    return axisY.get8bit();
  }

  bool isPressed() const {
    return sw.isPressed();
  }
};


/* ============================================================
   PACKET CODEC
   ============================================================ */

class PacketCodec {
public:

  static uint8_t crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;

    while (len--) {
      crc ^= *data++;

      for (uint8_t i = 0; i < 8; i++) {
        if (crc & 0x80)
          crc = (crc << 1) ^ 0x07;
        else
          crc <<= 1;
      }
    }

    return crc;
  }

  static void finalize(ControlPacket &packet, uint8_t buttonsMask) {

    packet.buttons = buttonsMask;

    uint8_t *data = (uint8_t *)&packet;

    uint8_t crc = crc8(data, 7);

    packet.counter ^= crc;  // mix CRC with counter
  }

  static bool verify(const ControlPacket &packet) {

    ControlPacket temp = packet;

    uint8_t mixed = temp.counter;

    uint8_t *data = (uint8_t *)&temp;

    uint8_t crc = crc8(data, 7);

    return ((mixed ^ crc) == (packet.counter ^ crc));
  }

  static uint8_t getButtons(const ControlPacket &packet) {
    return packet.buttons;
  }

  static bool getButton(const ControlPacket &packet, uint8_t index) {
    return packet.buttons & (1 << index);
  }
};


/* ============================================================
   JOYSTICK INPUT AGGREGATOR
   ============================================================ */

class JoystickController {
private:
  JoyInput joy1;
  JoyInput joy2;
  AnalogAxis range1;
  AnalogAxis range2;

  DigitalButton buttons[6];

  uint8_t packetCounter;

public:
  JoystickController()
    : joy1(A0, A1, 8),
      joy2(A2, A3, 12),
      range1(A4), range2(A5),
      buttons{
        DigitalButton(2),
        DigitalButton(3),
        DigitalButton(4),
        DigitalButton(5),
        DigitalButton(6),
        DigitalButton(7)
      },
      packetCounter(0) {}

  void begin() {
    joy1.begin();
    joy2.begin();
    range1.begin();
    range2.begin();

    for (uint8_t i = 0; i < 6; ++i)
      buttons[i].begin();
  }

  void update() {
    joy1.update();
    joy2.update();
    range1.update();
    range2.update();

    for (uint8_t i = 0; i < 6; ++i)
      buttons[i].update();
  }

  uint8_t buildButtonMask() const {
    uint8_t mask = 0;

    for (uint8_t i = 0; i < 6; ++i) {
      if (buttons[i].isPressed())
        mask |= (1 << i);
    }

    /* joystick SW buttons */
    if (joy1.isPressed())
      mask |= (1 << 6);

    if (joy2.isPressed())
      mask |= (1 << 7);

    return mask;
  }


  void buildPacket(ControlPacket &packet) {
    packet.counter = packetCounter++;
    packet.joy1x = joy1.getPacketX();
    packet.joy1y = joy1.getPacketY();
    packet.joy2x = joy2.getPacketX();
    packet.joy2y = joy2.getPacketY();
    packet.range1 = range1.get8bit();
    packet.range2 = range2.get8bit();

    PacketCodec::finalize(packet, buildButtonMask());
  }
};


/* ============================================================
   FAILSAFE
   ============================================================ */

class FailSafeMonitor {
private:
  unsigned long lastReceiveTime;

public:
  FailSafeMonitor()
    : lastReceiveTime(0) {}

  void begin() {
    lastReceiveTime = millis();
  }

  void markPacketReceived() {
    lastReceiveTime = millis();
  }

  bool isSignalLost() const {
    return (millis() - lastReceiveTime) > FAILSAFE_TIMEOUT_MS;
  }
};


/* ============================================================
   RADIO LINK
   ============================================================ */

class RadioLink {
private:
  RF24 radio;
  uint8_t pipeAddress[6];

public:
  RadioLink()
    : radio(CE_PIN, CSN_PIN),
      pipeAddress{ 'N', 'O', 'D', 'E', '1', '\0' } {}

  void beginCommon() {
    bool ok = radio.begin();

    if (ok)
      logger.log("NRF24 INIT OK");
    else
      logger.log("NRF24 INIT FAIL");

    radio.setPALevel(RF24_PA_LOW);
    radio.setDataRate(RF24_2MBPS);
    radio.setChannel(108);
    radio.setPayloadSize(sizeof(ControlPacket));
  }

  void beginTransmitter() {
    beginCommon();
    radio.openWritingPipe(pipeAddress);
    radio.stopListening();
  }

  void beginReceiver() {
    beginCommon();
    radio.openReadingPipe(0, pipeAddress);
    radio.startListening();
  }

  bool send(const ControlPacket &packet) {
    return radio.write(&packet, sizeof(packet));
  }

  bool available() {
    return radio.available();
  }

  void receive(ControlPacket &packet) {
    radio.read(&packet, sizeof(packet));
  }
};


/* ============================================================
   APPLICATIONS
   ============================================================ */

class TransmitterApp {
private:
  RadioLink radio;
  JoystickController joystick;
  ControlPacket packet;
  ControlPacket lastPacket;
  unsigned long lastTxTime;

  bool packetChanged() {
    return memcmp(&packet, &lastPacket, sizeof(ControlPacket)) != 0;
  }

public:
  TransmitterApp()
    : lastTxTime(0) {}

  void begin() {
    radio.beginTransmitter();
    joystick.begin();
    lastTxTime = millis();
  }

  void update() {
    unsigned long now = millis();

    if ((now - lastTxTime) >= TX_RATE_MS) {
      lastTxTime = now;

      joystick.update();
      joystick.buildPacket(packet);

      bool ok = radio.send(packet);

      if (packetChanged()) {
        lastPacket = packet;

        String msg = "TX ";
        msg += packet.counter;
        msg += ok ? " OK " : " FAIL ";

        msg += "J1(";
        msg += packet.joy1x;
        msg += ",";
        msg += packet.joy1y;
        msg += ") ";

        msg += "J2(";
        msg += packet.joy2x;
        msg += ",";
        msg += packet.joy2y;
        msg += ") ";

        msg += "R(";
        msg += packet.range1;
        msg += ",";
        msg += packet.range2;
        msg += ") ";

        msg += "B:";
        msg += String(PacketCodec::getButtons(packet), BIN);

        logger.log(msg);
      }
    }
  }
};

class ReceiverApp {
private:
  RadioLink radio;
  FailSafeMonitor failsafe;
  ControlPacket packet;
  ControlPacket lastPacket;
  bool failsafePrinted;

public:
  ReceiverApp()
    : failsafePrinted(false) {}

  void begin() {
    radio.beginReceiver();
    failsafe.begin();
  }

  void update() {
    if (radio.available()) {
      radio.receive(packet);

      if (PacketCodec::verify(packet)) {
        failsafe.markPacketReceived();
        failsafePrinted = false;

        bool changed = memcmp(&packet, &lastPacket, sizeof(ControlPacket)) != 0;

        if (changed) {
          lastPacket = packet;

          String msg = "RX ";
          msg += packet.counter;

          msg += " J1(";
          msg += packet.joy1x;
          msg += ",";
          msg += packet.joy1y;
          msg += ")";

          msg += " J2(";
          msg += packet.joy2x;
          msg += ",";
          msg += packet.joy2y;
          msg += ")";

          msg += " R(";
          msg += packet.range1;
          msg += ",";
          msg += packet.range2;
          msg += ")";

          msg += " B:";
          msg += String(PacketCodec::getButtons(packet), BIN);

          logger.log(msg);
        }
      } else {
        logger.log("CRC FAIL");
      }
    }

    if (failsafe.isSignalLost() && !failsafePrinted) {
      failsafePrinted = true;
      logger.log("FAILSAFE: SIGNAL LOST");
    }
  }
};


/* ============================================================
   GLOBAL APPLICATION INSTANCE
   ============================================================ */

#if DEVICE_IS_TRANSMITTER
TransmitterApp app;
#else
ReceiverApp app;
#endif


/* ============================================================
   ARDUINO ENTRY POINTS
   ============================================================ */

void setup() {
  Serial.begin(115200);
  app.begin();
}

void loop() {
  app.update();
}
