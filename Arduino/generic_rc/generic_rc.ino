#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/* =========================================================
   ===================== CONFIG =============================
   ========================================================= */

#define LED_PIN 13
#define BTN_TX 2
#define BTN_RX 3

RF24 radio(9, 10);

/* =========================================================
   ===================== ENUM ===============================
   ========================================================= */

enum RoleType { ROLE_TX, ROLE_RX };
enum SystemState { INIT, READY, PAIRING, RUNNING, ERROR };
enum BtnEvent { NONE, PRESS, RELEASE, HOLD, REPEAT };

/* =========================================================
   ===================== DATA ===============================
   ========================================================= */

struct ControlData {
    uint16_t buttons;
    uint8_t  events[10];
    int16_t  axes[4];
    int16_t  ranges[2];
} __attribute__((packed));

/* =========================================================
   ===================== LOGGER =============================
   ========================================================= */

void logPacket(const char* tag, const ControlData& d) {
    Serial.print("["); Serial.print(tag); Serial.print("] ");

    Serial.print("BTN="); Serial.print(d.buttons, BIN);

    Serial.print(" EVT=");
    for (int i = 0;i<10;i++){ Serial.print(d.events[i]); Serial.print(","); }

    Serial.print(" AX=");
    for (int i = 0;i<4;i++){ Serial.print(d.axes[i]); Serial.print(","); }

    Serial.print(" RG=");
    Serial.print(d.ranges[0]); Serial.print(",");
    Serial.print(d.ranges[1]);

    Serial.println();
}

/* =========================================================
   ===================== BUTTON =============================
   ========================================================= */

class Button {
    uint8_t pin, id;
    bool stable = HIGH, last = HIGH;
    unsigned long tDebounce = 0, tPress = 0, tRepeat = 0;

    const uint16_t debounceMs = 30, holdMs = 500, repeatMs = 200;

    ControlData* d;

    public:
    void begin(uint8_t p, uint8_t i, ControlData* data){
        pin = p; id = i; d = data;
        pinMode(pin, INPUT_PULLUP);
    }

    bool update(){
        bool r = digitalRead(pin);
        bool changed = false;

        if (r!=last) tDebounce = millis();

        if (millis()-tDebounce>debounceMs){

        if (r!=stable){
        stable = r;

        if (stable==LOW){
        d->events[id] = PRESS;
        tPress = millis();
    }else d->events[id] = RELEASE;

        changed = true;
    }

        if (stable==LOW){
        if (millis()-tPress>holdMs){
        if (millis()-tRepeat>repeatMs){
        d->events[id] = REPEAT;
        tRepeat = millis();
        changed = true;
    }else d->events[id] = HOLD;
    }
    } else d->events[id] = NONE;

        if (stable==LOW) d->buttons|=(1<<id);
        else d->buttons&=~(1<<id);
    }

        last = r;
        return changed;
    }
};

/* =========================================================
   ===================== ANALOG =============================
   ========================================================= */

class Analog {
    uint8_t xp, yp;
    int16_t cx, cy;
    int16_t *xo, *yo;
    const int deadzone = 10;

    public:
    void begin(uint8_t x, uint8_t y, int16_t*ox, int16_t*oy){
        xp = x; yp = y; xo = ox; yo = oy;
        delay(10);
        cx = analogRead(xp);
        cy = analogRead(yp);
    }

    bool update(){
        int16_t dx = analogRead(xp)-cx;
        int16_t dy = analogRead(yp)-cy;
        bool c = false;

        if (abs(dx)>deadzone){ *xo = dx; c = true; }
        if (abs(dy)>deadzone){ *yo = dy; c = true; }

        return c;
    }
};

/* =========================================================
   ===================== RANGE ==============================
   ========================================================= */

class Range {
    int16_t *o, last;

    public:
    void begin(int16_t* out){ o = out; last = 0; }

    bool update(){
        if (Serial.available()){
        int v = Serial.parseInt();
        if (v!=last){ *o = v; last = v; return true; }
    }
        return false;
    }
};

/* =========================================================
   ===================== BOARD INTERFACE ====================
   ========================================================= */

class IBoard {
    public:
    virtual void setupInputs(ControlData& d) = 0;
    virtual bool update(ControlData& d) = 0;
};

/* =========================================================
   ===================== NANO ===============================
   ========================================================= */

class NanoBoard : public IBoard {
    Button btn[10];
    Analog joy[2];
    Range rng[2];

    public:
    void setupInputs(ControlData& d){
        uint8_t pins[10] = {4, 5, 6, 7, 8, 12, A2, A3, A4, A5};
        for (int i = 0;i<10;i++) btn[i].begin(pins[i], i, &d);

        joy[0].begin(A0, A1, &d.axes[0], &d.axes[1]);
        joy[1].begin(A6, A7, &d.axes[2], &d.axes[3]);

        rng[0].begin(&d.ranges[0]);
        rng[1].begin(&d.ranges[1]);
    }

    bool update(ControlData& d){
        bool c = false;
        for (auto &b:btn) if (b.update()) c = true;
        for (auto &j:joy) if (j.update()) c = true;
        for (auto &r:rng) if (r.update()) c = true;
        return c;
    }
};

/* =========================================================
   ===================== UNO ================================
   ========================================================= */

class UnoBoard : public NanoBoard {};
class UnoR4Board : public NanoBoard {}; // mapping قابل تغییر

/* =========================================================
   ===================== BOARD FACTORY ======================
   ========================================================= */

class BoardFactory {
    public:
    static IBoard* create(){
        #if defined(ARDUINO_AVR_UNO)
        return new UnoBoard();
        #elif defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
        return new UnoR4Board();
        #else
        return new NanoBoard();
        #endif
    }
};

/* =========================================================
   ===================== PROTOCOL ===========================
   ========================================================= */

struct Packet {
    uint8_t type;
    uint8_t seq;
    ControlData data;
    uint8_t crc;
} __attribute__((packed));

uint8_t crc8(uint8_t* d, size_t l){
    uint8_t c = 0;
    for (size_t i = 0;i<l;i++) c^=d[i];
    return c;
}

/* =========================================================
   ===================== COMM ===============================
   ========================================================= */

class Comm {
    uint8_t seq = 0;

    public:

    bool send(ControlData& d){
        Packet p;
        p.type = 1;
        p.seq = seq++;
        p.data = d;
        p.crc = crc8((uint8_t*)&p, sizeof(p)-1);

        radio.stopListening();
        bool ok = radio.write(&p, sizeof(p));
        radio.startListening();

        logPacket("TX", d);
        return ok;
    }

    bool receive(ControlData& d){
        if (radio.available()){
        Packet p;
        radio.read(&p, sizeof(p));

        if (p.crc==crc8((uint8_t*)&p, sizeof(p)-1)){
        d = p.data;
        logPacket("RX", d);
        return true;
    }
    }
        return false;
    }
};

/* =========================================================
   ===================== MAIN ===============================
   ========================================================= */

IBoard* board;
Comm comm;

ControlData data;

void setup(){

    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    if (!radio.begin()){
    while (1) digitalWrite(LED_PIN, HIGH); // ERROR
}

    radio.openWritingPipe(0xF0F0F0F0E1LL);
    radio.openReadingPipe(1, 0xF0F0F0F0D2LL);
    radio.startListening();

    board = BoardFactory::create();
    board->setupInputs(data);

    Serial.println("SYSTEM READY");
}

void loop(){

    if (board->update(data)){
    comm.send(data);
}

    comm.receive(data);
}
