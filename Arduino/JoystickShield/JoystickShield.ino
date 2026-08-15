#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/* ===== RF ===== */
RF24 radio(9, 10);
const byte address[6] = "00001";

/* ===== Constants ===== */
constexpr int16_t DEADZONE = 15;

/* ===== Data ===== */
struct JoyStickPadData {
    int8_t A, B, C, D, E, F;
    int8_t SW1;
    int16_t X1, Y1;
    int8_t SW2;
    int16_t X2, Y2;
} __attribute__((packed));

/* ===== Button ===== */
class Button {
    int pin;
    bool last;
    int8_t *v;

    public:
    void begin(int p, int8_t *val) {
        pin = p;
        v = val;
        last = HIGH;
        pinMode(pin, INPUT_PULLUP);
    }

    void update() {
        bool cur = digitalRead(pin);
        if (last == HIGH && cur == LOW) *v = 2;
        else if (last == LOW && cur == HIGH) *v = 3;
        else if (cur == LOW) *v = 1;
        else *v = 0;
        last = cur;
    }

    bool changed() const {
        return *v != 0;
    }
};

/* ===== Joystick ===== */
class Joystick {
    int xp, yp;
    int16_t xc, yc;
    int16_t *xv, *yv;
    Button sw;

    public:
    void begin(int x, int y, int swp, int16_t *xOut, int16_t *yOut, int8_t *swOut) {
        xp = x;
        yp = y;
        xv = xOut;
        yv = yOut;
        pinMode(xp, INPUT);
        pinMode(yp, INPUT);
        delay(10);
        xc = analogRead(xp);
        yc = analogRead(yp);
        sw.begin(swp, swOut);
    }

    void update() {
        int16_t dx = analogRead(xp) - xc;
        int16_t dy = analogRead(yp) - yc;
        *xv = (abs(dx) > DEADZONE) ? dx : 0;
        *yv = (abs(dy) > DEADZONE) ? dy : 0;
        sw.update();
    }

    bool changed() const {
        return *xv || *yv || sw.changed();
    }
};

/* ===== Pad ===== */
class Pad {
    JoyStickPadData d{};
    Button btn[6];
    Joystick j1, j2;

    public:
    void begin(const int *p) {
        int8_t *b[6] = { &d.A, &d.B, &d.C, &d.D, &d.E, &d.F };
        for (int i = 0; i < 6; i++) btn[i].begin(p[i], b[i]);
        j1.begin(p[6], p[7], p[8], &d.X1, &d.Y1, &d.SW1);
        j2.begin(p[9], p[10], p[11], &d.X2, &d.Y2, &d.SW2);
    }

    void update() {
        for (auto &b : btn) b.update();
        j1.update();
        j2.update();
    }

    bool changed() const {
        for (auto &b : btn)
        if (b.changed()) return true;
        return j1.changed() || j2.changed();
    }

    const JoyStickPadData &data() const {
        return d;
    }
};

/* ===== Globals ===== */
const int pins[] = { 2, 3, 4, 5, 6, 7, A0, A1, A2, A3, A4, A5 };
Pad pad;

/* ===== Setup ===== */
void setup() {
    Serial.begin(9600);
    SPI.begin();
    pad.begin(pins);

    radio.begin();
    radio.setPALevel(RF24_PA_HIGH);
    radio.setDataRate(RF24_250KBPS);
    radio.setChannel(108);
    radio.setCRCLength(RF24_CRC_16);
    radio.setRetries(5, 15);
    radio.openWritingPipe(address);
    radio.stopListening();

    Serial.println("Sender ready");
}

/* ===== Loop ===== */
void loop() {
    pad.update();

    if (pad.changed()) {
    radio.write(&pad.data(), sizeof(JoyStickPadData));

    // Portable logging using only Serial.print
    Serial.print("A:");
    Serial.print(pad.data().A);
    Serial.print(" B:");
    Serial.print(pad.data().B);
    Serial.print(" C:");
    Serial.print(pad.data().C);
    Serial.print(" D:");
    Serial.print(pad.data().D);
    Serial.print(" E:");
    Serial.print(pad.data().E);
    Serial.print(" F:");
    Serial.print(pad.data().F);

    Serial.print(" | X1:");
    Serial.print(pad.data().X1);
    Serial.print(" Y1:");
    Serial.print(pad.data().Y1);
    Serial.print(" SW1:");
    Serial.print(pad.data().SW1);

    Serial.print(" | X2:");
    Serial.print(pad.data().X2);
    Serial.print(" Y2:");
    Serial.print(pad.data().Y2);
    Serial.print(" SW2:");
    Serial.println(pad.data().SW2);
}

    delay(20);
}
