# Arduino nRF24 Joystick Controller
Clean OOP | 8‑Byte Packet | 200Hz | Deadzone | Smoothing | Failsafe

---

## Overview

This project implements a modular, object‑oriented joystick controller system for:

- Arduino Uno (ATmega328P)
- nRF24L01+ wireless module
- 8‑byte compact RF packet
- 200Hz transmission
- Axis smoothing
- Deadzone filtering
- CRC validation
- Receiver failsafe detection
- Single-file architecture (cleanly layered)

The architecture follows embedded clean design principles while staying efficient for AVR microcontrollers.

---

# Architecture Overview

Even though this is a single `.ino` file, it is logically divided into layers:

CONFIGURATION
PACKET STRUCTURE
FILTERS
INPUT DEVICES
PACKET CODEC
JOYSTICK CONTROLLER
FAILSAFE
RADIO LINK
APPLICATION LAYER
ENTRY POINTS

Each section has a clearly defined responsibility.

---

# CONFIGURATION

```cpp
#define DEVICE_IS_TRANSMITTER 1
```

Selects device role:

| Value | Role |
|------|------|
| 1 | Transmitter |
| 0 | Receiver |

---

### RF Pins

```cpp
static const uint8_t CE_PIN = 9;
static const uint8_t CSN_PIN = 10;
```

Hardware wiring for nRF24.

---

### System Parameters

| Constant | Purpose |
|----------|---------|
| DEADZONE | Joystick center deadband |
| SMOOTH_FACTOR | Low-pass filter strength |
| TX_RATE_MS | Transmission period (5ms = 200Hz) |
| FAILSAFE_TIMEOUT_MS | Signal loss timeout |

---

# PACKET STRUCTURE

```cpp
struct ControlPacket
```

Total size: 8 bytes

| Byte | Content |
|------|---------|
| 0 | Packet counter |
| 1 | Joy1 X |
| 2 | Joy1 Y |
| 3 | Joy2 X |
| 4 | Joy2 Y |
| 5 | Range1 |
| 6 | Range2 |
| 7 | Buttons (6 bits) + CRC (2 bits) |

Highly optimized for nRF24 and well below the 32‑byte payload limit.

---

# FILTERS

## LowPassFilter

Smooths noisy analog input.

### Constructor

```cpp
LowPassFilter(uint8_t smoothFactor)
```

Defines filter strength.

### begin(initialValue)

Initializes filter with first reading.

### process(input)

Applies smoothing:

filtered = ((filtered*(factor-1)) + input) / factor

### current()

Returns filtered value.

---

## DeadZoneFilter

Prevents joystick drift around center.

### Constructor

```cpp
DeadZoneFilter(uint8_t deadzone)
```

### apply(value)

If value is near 128 (center), it returns 128.

---

# INPUT DEVICES

## AnalogAxis

Handles:

- Reading analog pin
- Smoothing
- Deadzone
- Scaling to 8-bit

### attach(pin)

Assign analog pin.

### begin()

Initializes filter using first analog reading.

### update()

Reads analog pin and updates filter.

### get8bit()

Returns processed 0–255 value:

- 10-bit → 8-bit conversion
- Deadzone applied

---

## DigitalButton

Handles digital input with internal pull-up.

### attach(pin)

Assign digital pin.

### begin()

Sets INPUT_PULLUP mode.

### update()

Updates current and previous state.

### isPressed()

Returns true if button pressed.

### isChanged()

Detects state transition.

---

# PACKET CODEC

Handles CRC and button packing.

### computeCRC2(packet)

Calculates 2‑bit XOR CRC from first 7 bytes.

### finalize(packet, buttonsMask)

- Packs button bits
- Computes CRC
- Stores CRC in bits 6–7

### verify(packet)

Validates received packet CRC.

### getButtons(packet)

Extracts 6‑bit button mask.

---

# JOYSTICK CONTROLLER

Aggregates all inputs.

Contains:

- 6 analog axes
- 6 digital buttons
- Packet counter

### begin()

Initializes all axes and buttons.

### update()

Updates all inputs.

### buildButtonMask()

Builds 6‑bit button mask.

### buildPacket(packet)

Fills packet with:

- Counter
- Axis values
- Button mask
- CRC

---

# FAILSAFE

Detects signal loss on receiver.

### begin()

Stores initial timestamp.

### markPacketReceived()

Updates last receive time.

### isSignalLost()

Returns true if timeout exceeded.

---

# RADIO LINK

Encapsulates RF24 logic.

### Constructor

Initializes RF24 with CE and CSN pins and pipe address.

### beginCommon()

Common radio configuration:

- RF24_PA_LOW
- 2Mbps data rate
- Channel 108
- Fixed payload size (8 bytes)

### beginTransmitter()

- Opens writing pipe
- Stops listening

### beginReceiver()

- Opens reading pipe
- Starts listening

### send(packet)

Transmits packet and returns true if ACK received.

### available()

Checks if a packet is available.

### receive(packet)

Reads received packet.

---

# APPLICATION LAYER

Two independent application modes:

- TransmitterApp
- ReceiverApp

---

# TransmitterApp

### begin()

Initializes radio and joystick.

### update()

Runs every 5 ms:

1. Update joystick inputs
2. Build control packet
3. Send packet
4. Print debug information

Example output:

TX 34 OK J1(128,130) J2(127,129) R(100,200) B:001001

---

# ReceiverApp

### begin()

Starts radio in receive mode and initializes failsafe.

### update()

1. Check if packet available
2. Validate CRC
3. Print received data
4. Update failsafe timer
5. Detect signal loss

Example:

RX 34 J1(128,130) J2(127,129) R(100,200) B:001001

On CRC error:

CRC FAIL

On signal loss:

FAILSAFE: SIGNAL LOST

---

# ENTRY POINTS

```cpp
void setup()
```

- Starts Serial
- Initializes selected application

```cpp
void loop()
```

Delegates execution to `app.update()`.

---

# Performance

| Metric | Value |
|-------|-------|
| Packet size | 8 bytes |
| RF speed | 2 Mbps |
| TX rate | 200 Hz |
| RAM usage | ~700 bytes |
| Flash usage | ~14 KB |

Suitable for ATmega328P devices such as Arduino Uno and Nano.

---

# Hardware Wiring

## nRF24L01+

| nRF24 Pin | Arduino |
|-----------|--------|
| VCC | 3.3V |
| GND | GND |
| CE | 9 |
| CSN | 10 |
| SCK | 13 |
| MOSI | 11 |
| MISO | 12 |

Recommended: place a 10µF–100µF capacitor between VCC and GND near the module.

---

# Usage

### Transmitter

Set:

```cpp
#define DEVICE_IS_TRANSMITTER 1
```

Upload to joystick controller board.

### Receiver

Set:

```cpp
#define DEVICE_IS_TRANSMITTER 0
```

Upload to receiver board.

---

# Possible Extensions

- EEPROM calibration
- Servo or motor outputs on receiver
- Changed-only transmission
- Heartbeat packets
- Channel remapping
- RC‑style failsafe positions
- Telemetry return channel

---

# Conclusion

This project provides a clean, modular, embedded‑optimized wireless joystick framework for Arduino using nRF24L01 modules.
