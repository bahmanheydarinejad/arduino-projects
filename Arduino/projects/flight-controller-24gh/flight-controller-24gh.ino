/**
 * UAV Remote Control & Telemetry — unified Remote/UAV firmware.
 *
 * Role selection (both): hold A7 above ROLE_THRESHOLD while resetting for
 * REMOTE; leave it low for UAV. Release it after boot so REMOTE can use A7 as
 * Linear-2. A 100k pulldown on A7 gives UAV a deterministic default.
 *
 * Pairing (both): hold the D8 button to GND for PAIR_HOLD_MS. Pairing is only
 * accepted while both devices have their physical PAIR button held.
 *
 * Safety: this sketch validates commands and drives ESC outputs to their safe
 * minimum. A real flight-control loop must be integrated before propeller use.
 */

#include <Arduino.h>
#include <EEPROM.h>
#include <RF24.h>
#include <SPI.h>

// ---------------------------------------------------------------------------
// Pin map — shared pins are reused only by mutually exclusive runtime roles.
// ---------------------------------------------------------------------------
namespace Pins {
// Both roles
constexpr uint8_t RADIO_CE = 9;
constexpr uint8_t RADIO_CSN = 10;
constexpr uint8_t PAIR_BUTTON = 8;  // Active LOW, uses INPUT_PULLUP.
constexpr uint8_t ROLE_SELECT = A7; // HIGH at reset = REMOTE; LOW = UAV.

// REMOTE only
constexpr uint8_t BUTTONS[6] = {2, 3, 4, 5, 6, 7};
constexpr uint8_t JOY_X1 = A0;
constexpr uint8_t JOY_Y1 = A1;
constexpr uint8_t JOY_SW1 = A2;
constexpr uint8_t JOY_X2 = A3;
constexpr uint8_t JOY_Y2 = A4;
constexpr uint8_t JOY_SW2 = A5;
constexpr uint8_t LINEAR_1 = A6;
constexpr uint8_t LINEAR_2 = A7; // Reused after role selection is latched.

// UAV only
constexpr uint8_t ESC_OUTPUTS[4] = {2, 3, 4, 5};
constexpr uint8_t BATTERY_SENSE = A0;
constexpr uint8_t DISTANCE_SENSE = A1;
// A4=SDA and A5=SCL remain available for an IMU in UAV mode.
} // namespace Pins

// ---------------------------------------------------------------------------
// Tunable configuration — all behavior outside the pin map is parameterized.
// ---------------------------------------------------------------------------
namespace Config {
constexpr uint32_t SERIAL_BAUD = 115200UL;
constexpr uint8_t RF_CHANNEL = 108;
constexpr uint8_t RF_RETRY_DELAY = 3; // (3 + 1) * 250 us between retries.
constexpr uint8_t RF_RETRY_COUNT = 5;
constexpr uint8_t PROTOCOL_VERSION = 1;
constexpr uint8_t PAYLOAD_SIZE = 32;

constexpr uint16_t ROLE_THRESHOLD = 700;
constexpr uint16_t INPUT_SCAN_MS = 3;
constexpr uint16_t CONTROL_PERIOD_MS = 10;
constexpr uint16_t HEARTBEAT_PERIOD_MS = 50;
constexpr uint16_t COMM_TIMEOUT_MS = 250;
constexpr uint16_t PAIR_HOLD_MS = 1200;
constexpr uint16_t PAIR_WINDOW_MS = 30000;
constexpr uint16_t PAIR_REQUEST_MS = 250;
constexpr uint16_t PAIR_RESPONSE_WAIT_MS = 120;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 20;

constexpr int16_t JOYSTICK_ENTER_DEADZONE = 20;
constexpr int16_t JOYSTICK_EXIT_DEADZONE = 14;
constexpr int16_t JOYSTICK_CHANGE_THRESHOLD = 3;
constexpr uint8_t CALIBRATION_SAMPLES = 16;

constexpr uint8_t LINK_BUCKETS = 20;       // Twenty 50 ms buckets = one second.
constexpr uint16_t LINK_BUCKET_MS = 50;

// Replace these values when provisioning production devices.
constexpr uint32_t REMOTE_DEVICE_ID = 0xA31F82C4UL;
constexpr uint32_t UAV_DEVICE_ID = 0x72BC194AUL;
constexpr uint16_t EEPROM_MAGIC = 0x24A7;
constexpr uint8_t EEPROM_VERSION = 1;
} // namespace Config

enum class Role : uint8_t { Remote = 1, Uav = 2 };
enum class PacketType : uint8_t {
  Control = 1,
  Telemetry = 2,
  PairRequest = 16,
  PairResponse = 17,
  PairConfirm = 18,
};

enum PacketFlags : uint8_t {
  FLAG_NONE = 0,
  FLAG_INPUT_CHANGED = 1 << 0,
  FLAG_LINK_DEGRADED = 1 << 1,
  FLAG_FAILSAFE = 1 << 2,
  FLAG_AUTH_RESERVED = 1 << 7,
};

// ---------------------------------------------------------------------------
// Wire protocol — all packets are exactly one nRF24 payload (32 bytes).
// AVR peers use little-endian values; static assertions prevent layout drift.
// ---------------------------------------------------------------------------
struct __attribute__((packed)) PacketHeader {
  uint8_t version;
  uint8_t type;
  uint8_t sequence;
  uint8_t flags;
  uint32_t sourceId;
  uint32_t targetId;
  uint16_t sessionId;
};

struct __attribute__((packed)) ControlPayload {
  int16_t joystickX1;
  int16_t joystickY1;
  int16_t joystickX2;
  int16_t joystickY2;
  uint16_t linear1;
  uint16_t linear2;
  uint8_t buttons;
  uint8_t changedMask;
};

struct __attribute__((packed)) TelemetryPayload {
  int16_t accelX;
  int16_t accelY;
  int16_t accelZ;
  int16_t gyroX;
  int16_t gyroY;
  int16_t gyroZ;
  uint8_t batteryPercent;
  uint8_t status;
};

struct __attribute__((packed)) PairPayload {
  uint32_t remoteNonce;
  uint32_t uavNonce;
  uint32_t proof;
  uint16_t capabilities;
};

template <typename Payload> struct __attribute__((packed)) WirePacket {
  PacketHeader header;
  Payload payload;
  uint32_t authTag;
};

using ControlPacket = WirePacket<ControlPayload>;
using TelemetryPacket = WirePacket<TelemetryPayload>;
using PairPacket = WirePacket<PairPayload>;

static_assert(sizeof(PacketHeader) == 14, "PacketHeader layout changed");
static_assert(sizeof(ControlPacket) == Config::PAYLOAD_SIZE, "Control packet must be 32 bytes");
static_assert(sizeof(TelemetryPacket) == Config::PAYLOAD_SIZE, "Telemetry packet must be 32 bytes");
static_assert(sizeof(PairPacket) == Config::PAYLOAD_SIZE, "Pair packet must be 32 bytes");

struct __attribute__((packed)) BindingRecord {
  uint16_t magic;
  uint8_t version;
  uint8_t role;
  uint32_t localId;
  uint32_t peerId;
  uint32_t linkKey;
  uint16_t crc;
};

// ---------------------------------------------------------------------------
// Small stateless helpers — shared by both roles.
// ---------------------------------------------------------------------------
static bool elapsed(uint32_t now, uint32_t since, uint32_t period) {
  return static_cast<uint32_t>(now - since) >= period;
}

static uint32_t mix32(uint32_t value) {
  value ^= value >> 16;
  value *= 0x7FEB352DUL;
  value ^= value >> 15;
  value *= 0x846CA68BUL;
  value ^= value >> 16;
  return value;
}

static uint16_t crc16(const uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;
  while (length--) {
    crc ^= static_cast<uint16_t>(*data++) << 8;
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x8000) ? static_cast<uint16_t>((crc << 1) ^ 0x1021)
                           : static_cast<uint16_t>(crc << 1);
    }
  }
  return crc;
}

static uint32_t makeNonce(uint32_t deviceId) {
  uint32_t value = micros() ^ deviceId;
  value ^= static_cast<uint32_t>(analogRead(A6)) << 20;
  value ^= static_cast<uint32_t>(analogRead(A0)) << 10;
  value ^= static_cast<uint32_t>(analogRead(A1));
  return mix32(value);
}

static uint32_t provisionalTag(const uint8_t *data, size_t length, uint32_t key) {
  // Lightweight keyed integrity tag, not a cryptographic MAC. The 32-bit field
  // is deliberately replaceable by a truncated AES-CMAC/SipHash in production.
  uint32_t hash = 2166136261UL ^ key;
  while (length--) {
    hash ^= *data++;
    hash *= 16777619UL;
  }
  return mix32(hash ^ key);
}

static void signPacket(ControlPacket &packet, uint32_t key) {
  packet.authTag = provisionalTag(reinterpret_cast<const uint8_t *>(&packet), 28, key);
}

static void signPacket(TelemetryPacket &packet, uint32_t key) {
  packet.authTag = provisionalTag(reinterpret_cast<const uint8_t *>(&packet), 28, key);
}

static void signPacket(PairPacket &packet, uint32_t key) {
  packet.authTag = provisionalTag(reinterpret_cast<const uint8_t *>(&packet), 28, key);
}

static bool hasValidTag(const ControlPacket &packet, uint32_t key) {
  return packet.authTag == provisionalTag(reinterpret_cast<const uint8_t *>(&packet), 28, key);
}

static bool hasValidTag(const TelemetryPacket &packet, uint32_t key) {
  return packet.authTag == provisionalTag(reinterpret_cast<const uint8_t *>(&packet), 28, key);
}

static bool hasValidTag(const PairPacket &packet, uint32_t key) {
  return packet.authTag == provisionalTag(reinterpret_cast<const uint8_t *>(&packet), 28, key);
}

static void makeBoundAddress(uint32_t firstId, uint32_t secondId, uint32_t key,
                             uint8_t (&address)[5]) {
  const uint32_t low = min(firstId, secondId);
  const uint32_t high = max(firstId, secondId);
  const uint32_t hash = mix32(low ^ mix32(high) ^ key);
  address[0] = 0xC3; // Avoid common preambles with a fixed non-zero prefix.
  address[1] = static_cast<uint8_t>(hash);
  address[2] = static_cast<uint8_t>(hash >> 8);
  address[3] = static_cast<uint8_t>(hash >> 16);
  address[4] = static_cast<uint8_t>(hash >> 24);
}

// ---------------------------------------------------------------------------
// EEPROM repository — both roles persist their own ID and their bound peer.
// ---------------------------------------------------------------------------
class BindingStore {
public:
  bool load(Role role, uint32_t expectedLocalId, BindingRecord &record) const {
    EEPROM.get(0, record);
    const uint16_t storedCrc = record.crc;
    record.crc = 0;
    const uint16_t actualCrc = crc16(reinterpret_cast<const uint8_t *>(&record), sizeof(record));
    record.crc = storedCrc;
    return record.magic == Config::EEPROM_MAGIC &&
           record.version == Config::EEPROM_VERSION &&
           record.role == static_cast<uint8_t>(role) &&
           record.localId == expectedLocalId && storedCrc == actualCrc;
  }

  void save(Role role, uint32_t localId, uint32_t peerId, uint32_t linkKey) const {
    BindingRecord record{};
    record.magic = Config::EEPROM_MAGIC;
    record.version = Config::EEPROM_VERSION;
    record.role = static_cast<uint8_t>(role);
    record.localId = localId;
    record.peerId = peerId;
    record.linkKey = linkKey;
    record.crc = 0;
    record.crc = crc16(reinterpret_cast<const uint8_t *>(&record), sizeof(record));
    EEPROM.put(0, record); // put/update avoids rewriting unchanged EEPROM bytes.
  }
};

// ---------------------------------------------------------------------------
// Debounced digital input — REMOTE only.
// ---------------------------------------------------------------------------
class DebouncedButton {
public:
  void begin(uint8_t pin) {
    pin_ = pin;
    pinMode(pin_, INPUT_PULLUP);
    rawPressed_ = stablePressed_ = (digitalRead(pin_) == LOW);
    changedAt_ = millis();
  }

  bool update(uint32_t now) {
    const bool raw = (digitalRead(pin_) == LOW);
    if (raw != rawPressed_) {
      rawPressed_ = raw;
      changedAt_ = now;
    }
    if (stablePressed_ != rawPressed_ && elapsed(now, changedAt_, Config::BUTTON_DEBOUNCE_MS)) {
      stablePressed_ = rawPressed_;
      return true;
    }
    return false;
  }

  bool pressed() const { return stablePressed_; }

private:
  uint8_t pin_ = 0;
  bool rawPressed_ = false;
  bool stablePressed_ = false;
  uint32_t changedAt_ = 0;
};

// ---------------------------------------------------------------------------
// Remote input manager — calibrates, filters and detects meaningful changes.
// ---------------------------------------------------------------------------
class RemoteInputs {
public:
  void begin() {
    for (uint8_t i = 0; i < 6; ++i) buttons_[i].begin(Pins::BUTTONS[i]);
    joySwitch1_.begin(Pins::JOY_SW1);
    joySwitch2_.begin(Pins::JOY_SW2);
    calibrateCenters();
    sample(millis());
    lastSent_ = state_;
    changedMask_ = 0xFF;
  }

  void update(uint32_t now) {
    if (!elapsed(now, lastScanAt_, Config::INPUT_SCAN_MS)) return;
    lastScanAt_ = now;
    sample(now);
  }

  bool changed() const { return changedMask_ != 0; }
  uint8_t changedMask() const { return changedMask_; }
  const ControlPayload &state() const { return state_; }

  void markDelivered() {
    lastSent_ = state_;
    changedMask_ = 0;
  }

private:
  static int16_t filterAxis(uint8_t pin, int16_t center, int16_t previous) {
    const int16_t raw = static_cast<int16_t>(analogRead(pin)) - center;
    const int16_t filtered = static_cast<int16_t>((static_cast<int32_t>(previous) * 3 + raw) / 4);
    const int16_t magnitude = abs(filtered);
    const bool wasMoving = previous != 0;
    const int16_t boundary = wasMoving ? Config::JOYSTICK_EXIT_DEADZONE
                                       : Config::JOYSTICK_ENTER_DEADZONE;
    return magnitude <= boundary ? 0 : constrain(filtered, -512, 511);
  }

  void calibrateCenters() {
    int32_t sums[4] = {0, 0, 0, 0};
    for (uint8_t sampleIndex = 0; sampleIndex < Config::CALIBRATION_SAMPLES; ++sampleIndex) {
      sums[0] += analogRead(Pins::JOY_X1);
      sums[1] += analogRead(Pins::JOY_Y1);
      sums[2] += analogRead(Pins::JOY_X2);
      sums[3] += analogRead(Pins::JOY_Y2);
    }
    for (uint8_t axis = 0; axis < 4; ++axis) centers_[axis] = sums[axis] / Config::CALIBRATION_SAMPLES;
  }

  void sample(uint32_t now) {
    state_.joystickX1 = filterAxis(Pins::JOY_X1, centers_[0], state_.joystickX1);
    state_.joystickY1 = filterAxis(Pins::JOY_Y1, centers_[1], state_.joystickY1);
    state_.joystickX2 = filterAxis(Pins::JOY_X2, centers_[2], state_.joystickX2);
    state_.joystickY2 = filterAxis(Pins::JOY_Y2, centers_[3], state_.joystickY2);
    state_.linear1 = analogRead(Pins::LINEAR_1);
    state_.linear2 = analogRead(Pins::LINEAR_2);

    uint8_t bits = 0;
    for (uint8_t i = 0; i < 6; ++i) {
      buttons_[i].update(now);
      if (buttons_[i].pressed()) bits |= static_cast<uint8_t>(1U << i);
    }
    joySwitch1_.update(now);
    joySwitch2_.update(now);
    if (joySwitch1_.pressed()) bits |= 1U << 6;
    if (joySwitch2_.pressed()) bits |= 1U << 7;
    state_.buttons = bits;

    changedMask_ = 0;
    if (axisChanged(state_.joystickX1, lastSent_.joystickX1) ||
        axisChanged(state_.joystickY1, lastSent_.joystickY1)) changedMask_ |= 1U << 0;
    if (axisChanged(state_.joystickX2, lastSent_.joystickX2) ||
        axisChanged(state_.joystickY2, lastSent_.joystickY2)) changedMask_ |= 1U << 1;
    if (abs(static_cast<int16_t>(state_.linear1 - lastSent_.linear1)) >= Config::JOYSTICK_CHANGE_THRESHOLD ||
        abs(static_cast<int16_t>(state_.linear2 - lastSent_.linear2)) >= Config::JOYSTICK_CHANGE_THRESHOLD)
      changedMask_ |= 1U << 2;
    if (state_.buttons != lastSent_.buttons) changedMask_ |= 1U << 3;
    state_.changedMask = changedMask_;
  }

  static bool axisChanged(int16_t current, int16_t previous) {
    return abs(static_cast<int16_t>(current - previous)) >= Config::JOYSTICK_CHANGE_THRESHOLD;
  }

  DebouncedButton buttons_[6];
  DebouncedButton joySwitch1_;
  DebouncedButton joySwitch2_;
  int16_t centers_[4]{};
  ControlPayload state_{};
  ControlPayload lastSent_{};
  uint8_t changedMask_ = 0;
  uint32_t lastScanAt_ = 0;
};

// ---------------------------------------------------------------------------
// One-second time-window delivery ratio — REMOTE only, no dynamic allocation.
// ---------------------------------------------------------------------------
class LinkQuality {
public:
  void record(uint32_t now, bool delivered) {
    advance(now);
    if (attempts_[cursor_] < 255) ++attempts_[cursor_];
    if (delivered && successes_[cursor_] < 255) ++successes_[cursor_];
    consecutiveFailures_ = delivered ? 0 : static_cast<uint8_t>(min(255, consecutiveFailures_ + 1));
  }

  uint8_t percent(uint32_t now) {
    advance(now);
    uint16_t attempts = 0;
    uint16_t successes = 0;
    for (uint8_t i = 0; i < Config::LINK_BUCKETS; ++i) {
      attempts += attempts_[i];
      successes += successes_[i];
    }
    return attempts ? static_cast<uint8_t>((successes * 100U) / attempts) : 0;
  }
  uint8_t consecutiveFailures() const { return consecutiveFailures_; }

private:
  void advance(uint32_t now) {
    if (!initialized_) {
      initialized_ = true;
      bucketStartedAt_ = now;
      return;
    }
    uint32_t steps = static_cast<uint32_t>(now - bucketStartedAt_) / Config::LINK_BUCKET_MS;
    if (!steps) return;
    if (steps > Config::LINK_BUCKETS) steps = Config::LINK_BUCKETS;
    for (uint8_t i = 0; i < steps; ++i) {
      cursor_ = static_cast<uint8_t>((cursor_ + 1) % Config::LINK_BUCKETS);
      attempts_[cursor_] = 0;
      successes_[cursor_] = 0;
    }
    bucketStartedAt_ += steps * Config::LINK_BUCKET_MS;
    if (steps == Config::LINK_BUCKETS) bucketStartedAt_ = now;
  }

  uint8_t attempts_[Config::LINK_BUCKETS]{};
  uint8_t successes_[Config::LINK_BUCKETS]{};
  uint8_t cursor_ = 0;
  uint8_t consecutiveFailures_ = 0;
  bool initialized_ = false;
  uint32_t bucketStartedAt_ = 0;
};

// ---------------------------------------------------------------------------
// UAV outputs — disabled until a verified flight controller is added.
// ---------------------------------------------------------------------------
class UavOutputs {
public:
  void begin() {
    for (uint8_t i = 0; i < 4; ++i) {
      pinMode(Pins::ESC_OUTPUTS[i], OUTPUT);
      digitalWrite(Pins::ESC_OUTPUTS[i], LOW);
    }
  }

  void acceptSetpoints(const ControlPayload &control) {
    latestControl_ = control;
    // Intentional: no direct joystick-to-motor mapping. Flight control belongs
    // here and must use validated IMU feedback before commanding live motors.
  }

  void enterFailsafe() {
    for (uint8_t i = 0; i < 4; ++i) digitalWrite(Pins::ESC_OUTPUTS[i], LOW);
  }

private:
  ControlPayload latestControl_{};
};

// ---------------------------------------------------------------------------
// Unified application — owns RF, pairing and the selected role state machine.
// ---------------------------------------------------------------------------
class FlightRadioApplication {
public:
  FlightRadioApplication() : radio_(Pins::RADIO_CE, Pins::RADIO_CSN) {}

  void begin() {
    Serial.begin(Config::SERIAL_BAUD);
    pinMode(Pins::PAIR_BUTTON, INPUT_PULLUP);

    role_ = analogRead(Pins::ROLE_SELECT) >= Config::ROLE_THRESHOLD ? Role::Remote : Role::Uav;
    localId_ = role_ == Role::Remote ? Config::REMOTE_DEVICE_ID : Config::UAV_DEVICE_ID;
    sessionId_ = static_cast<uint16_t>(makeNonce(localId_));

    if (role_ == Role::Remote) inputs_.begin();
    else outputs_.begin();

    if (!radio_.begin()) {
      Serial.println(F("RF24 init failed"));
      radioReady_ = false;
      return;
    }
    radioReady_ = true;
    configureRadio();

    paired_ = store_.load(role_, localId_, binding_);
    if (paired_) configureNormalPipe();
    else configurePairingPipe();

    Serial.print(F("Role: "));
    Serial.println(role_ == Role::Remote ? F("REMOTE") : F("UAV"));
    Serial.println(paired_ ? F("Binding loaded") : F("Not paired; hold PAIR on both devices"));
  }

  void update() {
    const uint32_t now = millis();
    updatePairButton(now);
    if (!radioReady_) return;

    if (pairing_) {
      updatePairing(now);
      return;
    }

    if (!paired_) return;
    if (role_ == Role::Remote) updateRemote(now);
    else updateUav(now);
  }

private:
  static constexpr uint8_t DISCOVERY_ADDRESS_[6] = {'P', 'A', 'I', 'R', '0', 0};

  void configureRadio() {
    radio_.setAddressWidth(5);
    radio_.setChannel(Config::RF_CHANNEL);
    radio_.setDataRate(RF24_250KBPS);
    radio_.setPALevel(RF24_PA_LOW);
    radio_.setCRCLength(RF24_CRC_16);
    radio_.setRetries(Config::RF_RETRY_DELAY, Config::RF_RETRY_COUNT);
    radio_.setAutoAck(true);
    radio_.enableDynamicPayloads();
    radio_.enableAckPayload();
  }

  void configurePairingPipe() {
    radio_.stopListening();
    radio_.flush_rx();
    radio_.flush_tx();
    radio_.openWritingPipe(DISCOVERY_ADDRESS_);
    radio_.openReadingPipe(1, DISCOVERY_ADDRESS_);
    if (role_ == Role::Uav) radio_.startListening();
  }

  void configureNormalPipe() {
    uint8_t address[5];
    makeBoundAddress(localId_, binding_.peerId, binding_.linkKey, address);
    radio_.stopListening();
    radio_.flush_rx();
    radio_.flush_tx();
    radio_.openWritingPipe(address);
    radio_.openReadingPipe(1, address);
    if (role_ == Role::Uav) {
      radio_.startListening();
      queueTelemetryAck();
    }
  }

  void updatePairButton(uint32_t now) {
    const bool pressed = digitalRead(Pins::PAIR_BUTTON) == LOW;
    if (pressed && !pairButtonWasPressed_) pairPressedAt_ = now;
    if (!pressed) pairTriggered_ = false;
    if (pressed && !pairTriggered_ && elapsed(now, pairPressedAt_, Config::PAIR_HOLD_MS)) {
      pairTriggered_ = true;
      enterPairing(now);
    }
    pairButtonWasPressed_ = pressed;
  }

  void enterPairing(uint32_t now) {
    pairing_ = true;
    paired_ = false;
    pairStartedAt_ = now;
    pairNonceLocal_ = makeNonce(localId_);
    pairNoncePeer_ = 0;
    candidatePeerId_ = 0;
    configurePairingPipe();
    Serial.println(F("Pairing window opened"));
  }

  void updatePairing(uint32_t now) {
    if (elapsed(now, pairStartedAt_, Config::PAIR_WINDOW_MS)) {
      pairing_ = false;
      paired_ = store_.load(role_, localId_, binding_);
      if (paired_) configureNormalPipe();
      Serial.println(F("Pairing timed out"));
      return;
    }
    if (role_ == Role::Remote) updateRemotePairing(now);
    else updateUavPairing();
  }

  PairPacket makePairPacket(PacketType type, uint32_t targetId, uint32_t remoteNonce,
                            uint32_t uavNonce, uint32_t proof) {
    PairPacket packet{};
    fillHeader(packet.header, type, targetId, FLAG_NONE);
    packet.payload.remoteNonce = remoteNonce;
    packet.payload.uavNonce = uavNonce;
    packet.payload.proof = proof;
    packet.payload.capabilities = 0x0001; // Protocol supports a future strong MAC.
    signPacket(packet, 0); // Pairing discovery has integrity only, not secrecy.
    return packet;
  }

  void updateRemotePairing(uint32_t now) {
    if (!waitingForPairResponse_ && elapsed(now, lastPairRequestAt_, Config::PAIR_REQUEST_MS)) {
      lastPairRequestAt_ = now;
      PairPacket request = makePairPacket(PacketType::PairRequest, 0, pairNonceLocal_, 0, 0);
      radio_.stopListening();
      radio_.openWritingPipe(DISCOVERY_ADDRESS_);
      radio_.write(&request, sizeof(request));
      radio_.startListening();
      waitingForPairResponse_ = true;
      responseWaitStartedAt_ = now;
    }

    if (radio_.available()) {
      PairPacket response{};
      if (readPacket(response) && response.header.type == static_cast<uint8_t>(PacketType::PairResponse) &&
          response.header.targetId == localId_ && response.payload.remoteNonce == pairNonceLocal_ &&
          hasValidTag(response, 0)) {
        candidatePeerId_ = response.header.sourceId;
        pairNoncePeer_ = response.payload.uavNonce;
        const uint32_t linkKey = deriveLinkKey(localId_, candidatePeerId_, pairNonceLocal_, pairNoncePeer_);
        const uint32_t proof = pairingProof(localId_, candidatePeerId_, pairNonceLocal_, pairNoncePeer_, linkKey);
        PairPacket confirm = makePairPacket(PacketType::PairConfirm, candidatePeerId_,
                                            pairNonceLocal_, pairNoncePeer_, proof);
        radio_.stopListening();
        const bool delivered = radio_.write(&confirm, sizeof(confirm));
        if (delivered) completePairing(candidatePeerId_, linkKey);
        else radio_.startListening();
      }
    }
    if (waitingForPairResponse_ && elapsed(now, responseWaitStartedAt_, Config::PAIR_RESPONSE_WAIT_MS)) {
      waitingForPairResponse_ = false;
      radio_.stopListening();
    }
  }

  void updateUavPairing() {
    if (!radio_.available()) return;
    PairPacket packet{};
    if (!readPacket(packet) || !hasValidTag(packet, 0)) return;

    if (packet.header.type == static_cast<uint8_t>(PacketType::PairRequest)) {
      candidatePeerId_ = packet.header.sourceId;
      pairNoncePeer_ = packet.payload.remoteNonce;
      PairPacket response = makePairPacket(PacketType::PairResponse, candidatePeerId_,
                                           pairNoncePeer_, pairNonceLocal_, 0);
      radio_.stopListening();
      radio_.write(&response, sizeof(response));
      radio_.startListening();
      return;
    }

    if (packet.header.type == static_cast<uint8_t>(PacketType::PairConfirm) &&
        packet.header.sourceId == candidatePeerId_ && packet.header.targetId == localId_ &&
        packet.payload.remoteNonce == pairNoncePeer_ && packet.payload.uavNonce == pairNonceLocal_) {
      const uint32_t linkKey = deriveLinkKey(candidatePeerId_, localId_, pairNoncePeer_, pairNonceLocal_);
      const uint32_t expected = pairingProof(candidatePeerId_, localId_, pairNoncePeer_, pairNonceLocal_, linkKey);
      if (packet.payload.proof == expected) completePairing(candidatePeerId_, linkKey);
    }
  }

  static uint32_t deriveLinkKey(uint32_t remoteId, uint32_t uavId,
                                uint32_t remoteNonce, uint32_t uavNonce) {
    return mix32(remoteId ^ mix32(uavId) ^ remoteNonce ^ mix32(uavNonce));
  }

  static uint32_t pairingProof(uint32_t remoteId, uint32_t uavId,
                               uint32_t remoteNonce, uint32_t uavNonce, uint32_t key) {
    uint32_t fields[4] = {remoteId, uavId, remoteNonce, uavNonce};
    return provisionalTag(reinterpret_cast<const uint8_t *>(fields), sizeof(fields), key);
  }

  void completePairing(uint32_t peerId, uint32_t linkKey) {
    store_.save(role_, localId_, peerId, linkKey);
    store_.load(role_, localId_, binding_);
    paired_ = true;
    pairing_ = false;
    waitingForPairResponse_ = false;
    configureNormalPipe();
    Serial.println(F("Pairing complete and saved"));
  }

  void updateRemote(uint32_t now) {
    inputs_.update(now);
    const bool controlDue = elapsed(now, lastControlAt_, Config::CONTROL_PERIOD_MS);
    const bool heartbeatDue = elapsed(now, lastHeartbeatAt_, Config::HEARTBEAT_PERIOD_MS);
    if (!controlDue || (!inputs_.changed() && !heartbeatDue)) return;

    lastControlAt_ = now;
    if (heartbeatDue) lastHeartbeatAt_ = now;
    ControlPacket packet{};
    const uint8_t flags = inputs_.changed() ? FLAG_INPUT_CHANGED : FLAG_NONE;
    fillHeader(packet.header, PacketType::Control, binding_.peerId, flags);
    packet.payload = inputs_.state();
    signPacket(packet, binding_.linkKey);

    radio_.stopListening();
    const bool delivered = radio_.write(&packet, sizeof(packet));
    linkQuality_.record(now, delivered);
    if (delivered) inputs_.markDelivered();

    if (radio_.isAckPayloadAvailable()) {
      TelemetryPacket telemetry{};
      if (readPacket(telemetry) && validateNormalPacket(telemetry, PacketType::Telemetry) &&
          isNewSequence(telemetry.header.sequence, telemetry.header.sessionId)) {
        latestTelemetry_ = telemetry.payload;
      }
    }

    if (elapsed(now, lastStatusAt_, 1000)) {
      lastStatusAt_ = now;
      Serial.print(F("Link:"));
      Serial.print(linkQuality_.percent(now));
      Serial.print(F("% Failures:"));
      Serial.print(linkQuality_.consecutiveFailures());
      Serial.print(F(" Battery:"));
      Serial.println(latestTelemetry_.batteryPercent);
    }
  }

  void updateUav(uint32_t now) {
    bool receivedControl = false;
    while (radio_.available()) {
      ControlPacket packet{};
      if (readPacket(packet) && validateNormalPacket(packet, PacketType::Control) &&
          isNewSequence(packet.header.sequence, packet.header.sessionId)) {
        outputs_.acceptSetpoints(packet.payload);
        lastValidControlAt_ = now;
        failsafe_ = false;
        receivedControl = true;
      }
    }
    if (receivedControl) queueTelemetryAck();
    if (!failsafe_ && elapsed(now, lastValidControlAt_, Config::COMM_TIMEOUT_MS)) {
      failsafe_ = true;
      outputs_.enterFailsafe();
      Serial.println(F("FAILSAFE: control timeout"));
    }
  }

  void queueTelemetryAck() {
    TelemetryPacket packet{};
    fillHeader(packet.header, PacketType::Telemetry, binding_.peerId,
               failsafe_ ? FLAG_FAILSAFE : FLAG_NONE);
    // Sensor manager integration points; safe placeholders until hardware is selected.
    packet.payload.accelX = 0;
    packet.payload.accelY = 0;
    packet.payload.accelZ = 0;
    packet.payload.gyroX = 0;
    packet.payload.gyroY = 0;
    packet.payload.gyroZ = 0;
    packet.payload.batteryPercent = readBatteryPercent();
    packet.payload.status = failsafe_ ? FLAG_FAILSAFE : FLAG_NONE;
    signPacket(packet, binding_.linkKey);
    radio_.writeAckPayload(1, &packet, sizeof(packet));
  }

  uint8_t readBatteryPercent() const {
    // Placeholder linear conversion. Replace with calibrated divider values.
    return static_cast<uint8_t>(map(analogRead(Pins::BATTERY_SENSE), 0, 1023, 0, 100));
  }

  void fillHeader(PacketHeader &header, PacketType type, uint32_t targetId, uint8_t flags) {
    header.version = Config::PROTOCOL_VERSION;
    header.type = static_cast<uint8_t>(type);
    header.sequence = txSequence_++;
    header.flags = flags;
    header.sourceId = localId_;
    header.targetId = targetId;
    header.sessionId = sessionId_;
  }

  template <typename Packet>
  bool validateNormalPacket(const Packet &packet, PacketType expectedType) const {
    return packet.header.version == Config::PROTOCOL_VERSION &&
           packet.header.type == static_cast<uint8_t>(expectedType) &&
           packet.header.sourceId == binding_.peerId &&
           packet.header.targetId == localId_ && hasValidTag(packet, binding_.linkKey);
  }

  bool isNewSequence(uint8_t sequence, uint16_t peerSessionId) {
    if (!hasRxSequence_ || peerSessionId != lastPeerSessionId_) {
      hasRxSequence_ = true;
      lastPeerSessionId_ = peerSessionId;
      lastRxSequence_ = sequence;
      return true;
    }
    const int8_t delta = static_cast<int8_t>(sequence - lastRxSequence_);
    if (delta <= 0) return false;
    lastRxSequence_ = sequence;
    return true;
  }

  template <typename Packet> bool readPacket(Packet &packet) {
    const uint8_t size = radio_.getDynamicPayloadSize();
    if (size != sizeof(Packet)) {
      if (size) {
        uint8_t discard[Config::PAYLOAD_SIZE];
        radio_.read(discard, min(size, Config::PAYLOAD_SIZE));
      } else {
        radio_.flush_rx();
      }
      return false;
    }
    radio_.read(&packet, sizeof(packet));
    return true;
  }

  RF24 radio_;
  BindingStore store_;
  BindingRecord binding_{};
  RemoteInputs inputs_;
  LinkQuality linkQuality_;
  UavOutputs outputs_;
  TelemetryPayload latestTelemetry_{};

  Role role_ = Role::Uav;
  bool radioReady_ = false;
  bool paired_ = false;
  bool pairing_ = false;
  bool failsafe_ = true;
  bool pairButtonWasPressed_ = false;
  bool pairTriggered_ = false;
  bool waitingForPairResponse_ = false;
  bool hasRxSequence_ = false;

  uint8_t txSequence_ = 0;
  uint8_t lastRxSequence_ = 0;
  uint16_t sessionId_ = 0;
  uint16_t lastPeerSessionId_ = 0;
  uint32_t localId_ = 0;
  uint32_t candidatePeerId_ = 0;
  uint32_t pairNonceLocal_ = 0;
  uint32_t pairNoncePeer_ = 0;
  uint32_t pairPressedAt_ = 0;
  uint32_t pairStartedAt_ = 0;
  uint32_t lastPairRequestAt_ = 0;
  uint32_t responseWaitStartedAt_ = 0;
  uint32_t lastControlAt_ = 0;
  uint32_t lastHeartbeatAt_ = 0;
  uint32_t lastStatusAt_ = 0;
  uint32_t lastValidControlAt_ = 0;
};

constexpr uint8_t FlightRadioApplication::DISCOVERY_ADDRESS_[6];

FlightRadioApplication application;

void setup() { application.begin(); }

void loop() { application.update(); }
