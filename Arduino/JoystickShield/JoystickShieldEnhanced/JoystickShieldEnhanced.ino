#include <SPI.h>
#include <RF24.h>

/* ============================================================
   CONFIGURATION
   ============================================================ */

#define DEVICE_IS_TRANSMITTER 1   // 1 = TX, 0 = RX

static const uint8_t CE_PIN = 9;
static const uint8_t CSN_PIN = 10;

static const uint8_t DEADZONE = 6;
static const uint8_t SMOOTH_FACTOR = 4;
static const uint16_t TX_RATE_MS = 5;          // 200Hz
static const uint16_t FAILSAFE_TIMEOUT_MS = 500;


/* ============================================================
   PACKET
   8 bytes total

   byte0: counter
   byte1: joy1x
   byte2: joy1y
   byte3: joy2x
   byte4: joy2y
   byte5: range1
   byte6: range2
   byte7: buttons[0..5] + crc[6..7]
   ============================================================ */

struct ControlPacket
{
  uint8_t counter;
  uint8_t joy1x;
  uint8_t joy1y;
  uint8_t joy2x;
  uint8_t joy2y;
  uint8_t range1;
  uint8_t range2;
  uint8_t buttons_crc;
};


/* ============================================================
   FILTERS
   ============================================================ */

class LowPassFilter
{
private:
  uint8_t factor;
  int filtered;

public:
  LowPassFilter(uint8_t smoothFactor = 4)
    : factor(smoothFactor), filtered(0)
  {
  }

  void begin(int initialValue)
  {
    filtered = initialValue;
  }

  int process(int input)
  {
    filtered = ((long)filtered * (factor - 1) + input) / factor;
    return filtered;
  }

  int current() const
  {
    return filtered;
  }
};

class DeadZoneFilter
{
private:
  uint8_t zone;

public:
  DeadZoneFilter(uint8_t deadzone = 6)
    : zone(deadzone)
  {
  }

  uint8_t apply(uint8_t value) const
  {
    if (abs((int)value - 128) < zone)
      return 128;

    return value;
  }
};


/* ============================================================
   INPUT DEVICES
   ============================================================ */

class AnalogAxis
{
private:
  uint8_t pin;
  LowPassFilter lowPass;
  DeadZoneFilter deadZone;

public:
  AnalogAxis()
    : pin(A0), lowPass(SMOOTH_FACTOR), deadZone(DEADZONE)
  {
  }

  AnalogAxis(uint8_t analogPin)
    : pin(analogPin), lowPass(SMOOTH_FACTOR), deadZone(DEADZONE)
  {
  }

  void attach(uint8_t analogPin)
  {
    pin = analogPin;
  }

  void begin()
  {
    int first = analogRead(pin);
    lowPass.begin(first);
  }

  void update()
  {
    int raw = analogRead(pin);
    lowPass.process(raw);
  }

  uint8_t get8bit() const
  {
    int filtered = lowPass.current();
    int scaled = filtered >> 2;   // 0..1023 -> 0..255

    if (scaled < 0) scaled = 0;
    if (scaled > 255) scaled = 255;

    return deadZone.apply((uint8_t)scaled);
  }
};

class DigitalButton
{
private:
  uint8_t pin;
  bool state;
  bool previous;

public:
  DigitalButton()
    : pin(2), state(HIGH), previous(HIGH)
  {
  }

  DigitalButton(uint8_t digitalPin)
    : pin(digitalPin), state(HIGH), previous(HIGH)
  {
  }

  void attach(uint8_t digitalPin)
  {
    pin = digitalPin;
  }

  void begin()
  {
    pinMode(pin, INPUT_PULLUP);
    state = digitalRead(pin);
    previous = state;
  }

  void update()
  {
    previous = state;
    state = digitalRead(pin);
  }

  bool isPressed() const
  {
    return state == LOW;
  }

  bool isChanged() const
  {
    return state != previous;
  }
};


/* ============================================================
   PACKET CODEC
   ============================================================ */

class PacketCodec
{
public:
  static uint8_t computeCRC2(const ControlPacket &packet)
  {
    const uint8_t *data = (const uint8_t*)&packet;
    uint8_t crc = 0;

    for (uint8_t i = 0; i < 7; ++i)
      crc ^= data[i];

    return crc & 0x03; // 2-bit CRC
  }

  static void finalize(ControlPacket &packet, uint8_t buttonsMask)
  {
    packet.buttons_crc = (buttonsMask & 0x3F);
    uint8_t crc = computeCRC2(packet);
    packet.buttons_crc |= (crc << 6);
  }

  static bool verify(const ControlPacket &packet)
  {
    uint8_t expected = computeCRC2(packet);
    uint8_t actual = (packet.buttons_crc >> 6) & 0x03;
    return expected == actual;
  }

  static uint8_t getButtons(const ControlPacket &packet)
  {
    return packet.buttons_crc & 0x3F;
  }
};


/* ============================================================
   JOYSTICK INPUT AGGREGATOR
   ============================================================ */

class JoystickController
{
private:
  AnalogAxis joy1x;
  AnalogAxis joy1y;
  AnalogAxis joy2x;
  AnalogAxis joy2y;
  AnalogAxis range1;
  AnalogAxis range2;

  DigitalButton buttons[6];

  uint8_t packetCounter;

public:
  JoystickController()
    : joy1x(A0), joy1y(A1),
      joy2x(A2), joy2y(A3),
      range1(A4), range2(A5),
      buttons{
        DigitalButton(2),
        DigitalButton(3),
        DigitalButton(4),
        DigitalButton(5),
        DigitalButton(6),
        DigitalButton(7)
      },
      packetCounter(0)
  {
  }

  void begin()
  {
    joy1x.begin();
    joy1y.begin();
    joy2x.begin();
    joy2y.begin();
    range1.begin();
    range2.begin();

    for (uint8_t i = 0; i < 6; ++i)
      buttons[i].begin();
  }

  void update()
  {
    joy1x.update();
    joy1y.update();
    joy2x.update();
    joy2y.update();
    range1.update();
    range2.update();

    for (uint8_t i = 0; i < 6; ++i)
      buttons[i].update();
  }

  uint8_t buildButtonMask() const
  {
    uint8_t mask = 0;

    for (uint8_t i = 0; i < 6; ++i)
    {
      if (buttons[i].isPressed())
        mask |= (1 << i);
    }

    return mask;
  }

  void buildPacket(ControlPacket &packet)
  {
    packet.counter = packetCounter++;
    packet.joy1x = joy1x.get8bit();
    packet.joy1y = joy1y.get8bit();
    packet.joy2x = joy2x.get8bit();
    packet.joy2y = joy2y.get8bit();
    packet.range1 = range1.get8bit();
    packet.range2 = range2.get8bit();

    PacketCodec::finalize(packet, buildButtonMask());
  }
};


/* ============================================================
   FAILSAFE
   ============================================================ */

class FailSafeMonitor
{
private:
  unsigned long lastReceiveTime;

public:
  FailSafeMonitor()
    : lastReceiveTime(0)
  {
  }

  void begin()
  {
    lastReceiveTime = millis();
  }

  void markPacketReceived()
  {
    lastReceiveTime = millis();
  }

  bool isSignalLost() const
  {
    return (millis() - lastReceiveTime) > FAILSAFE_TIMEOUT_MS;
  }
};


/* ============================================================
   RADIO LINK
   ============================================================ */

class RadioLink
{
private:
  RF24 radio;
  uint8_t pipeAddress[6];

public:
  RadioLink()
    : radio(CE_PIN, CSN_PIN),
      pipeAddress{'N','O','D','E','1','\0'}
  {
  }

  void beginCommon()
  {
    radio.begin();
    radio.setPALevel(RF24_PA_LOW);
    radio.setDataRate(RF24_2MBPS);
    radio.setChannel(108);
    radio.setPayloadSize(sizeof(ControlPacket));
  }

  void beginTransmitter()
  {
    beginCommon();
    radio.openWritingPipe(pipeAddress);
    radio.stopListening();
  }

  void beginReceiver()
  {
    beginCommon();
    radio.openReadingPipe(0, pipeAddress);
    radio.startListening();
  }

  bool send(const ControlPacket &packet)
  {
    return radio.write(&packet, sizeof(packet));
  }

  bool available()
  {
    return radio.available();
  }

  void receive(ControlPacket &packet)
  {
    radio.read(&packet, sizeof(packet));
  }
};


/* ============================================================
   APPLICATIONS
   ============================================================ */

class TransmitterApp
{
private:
  RadioLink radio;
  JoystickController joystick;
  ControlPacket packet;
  unsigned long lastTxTime;

public:
  TransmitterApp()
    : lastTxTime(0)
  {
  }

  void begin()
  {
    radio.beginTransmitter();
    joystick.begin();
    lastTxTime = millis();
  }

  void update()
  {
    unsigned long now = millis();

    if ((now - lastTxTime) >= TX_RATE_MS)
    {
      lastTxTime = now;

      joystick.update();
      joystick.buildPacket(packet);

      bool ok = radio.send(packet);

      Serial.print(F("TX "));
      Serial.print(packet.counter);
      Serial.print(F(" "));
      Serial.print(ok ? F("OK ") : F("FAIL "));

      Serial.print(F("J1("));
      Serial.print(packet.joy1x);
      Serial.print(F(","));
      Serial.print(packet.joy1y);
      Serial.print(F(") J2("));
      Serial.print(packet.joy2x);
      Serial.print(F(","));
      Serial.print(packet.joy2y);
      Serial.print(F(") R("));
      Serial.print(packet.range1);
      Serial.print(F(","));
      Serial.print(packet.range2);
      Serial.print(F(") B:"));
      Serial.println(PacketCodec::getButtons(packet), BIN);
    }
  }
};

class ReceiverApp
{
private:
  RadioLink radio;
  FailSafeMonitor failsafe;
  ControlPacket packet;
  bool failsafePrinted;

public:
  ReceiverApp()
    : failsafePrinted(false)
  {
  }

  void begin()
  {
    radio.beginReceiver();
    failsafe.begin();
  }

  void update()
  {
    if (radio.available())
    {
      radio.receive(packet);

      if (PacketCodec::verify(packet))
      {
        failsafe.markPacketReceived();
        failsafePrinted = false;

        Serial.print(F("RX "));
        Serial.print(packet.counter);

        Serial.print(F(" J1("));
        Serial.print(packet.joy1x);
        Serial.print(F(","));
        Serial.print(packet.joy1y);
        Serial.print(F(") J2("));
        Serial.print(packet.joy2x);
        Serial.print(F(","));
        Serial.print(packet.joy2y);
        Serial.print(F(") R("));
        Serial.print(packet.range1);
        Serial.print(F(","));
        Serial.print(packet.range2);
        Serial.print(F(") B:"));
        Serial.println(PacketCodec::getButtons(packet), BIN);
      }
      else
      {
        Serial.println(F("CRC FAIL"));
      }
    }

    if (failsafe.isSignalLost() && !failsafePrinted)
    {
      failsafePrinted = true;
      Serial.println(F("FAILSAFE: SIGNAL LOST"));
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

void setup()
{
  Serial.begin(115200);
  app.begin();
}

void loop()
{
  app.update();
}
