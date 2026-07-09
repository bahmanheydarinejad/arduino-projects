// ------------------------------ // 8x8 LED Matrix Test // 1088BS (Common Anode) // ------------------------------
const uint8_t ROWS = 8;
const uint8_t COLS = 8;

// Row pins (Anodes)
const uint8_t rowPins[ROWS] = { 10, 11, 12, 13, A0, A1, A2, A3 };
// Column pins (Cathodes)
const uint8_t colPins[COLS] = { 2, 3, 4, 5, 6, 7, 8, 9 };
// Refresh one LED for the specified time

void lightSingleLed(uint8_t row, uint8_t col, unsigned long durationMs) {
  unsigned long start = millis();
  while (millis() - start < durationMs) {
    // Disable everything
    for (int i = 0; i < ROWS; i++) digitalWrite(rowPins[i], LOW);
    for (int i = 0; i < COLS; i++) digitalWrite(colPins[i], HIGH);
    // Enable desired LED
    digitalWrite(rowPins[row], HIGH);
    digitalWrite(colPins[col], LOW);
    delay(2);
  }
}

// Refresh all LEDs simultaneously
void lightAll(unsigned long durationMs) {
  unsigned long start = millis();
  while (millis() - start < durationMs) {
    for (int r = 0; r < ROWS; r++) {
      // Disable all rows
      for (int i = 0; i < ROWS; i++) digitalWrite(rowPins[i], LOW);
      // Turn every column ON
      for (int c = 0; c < COLS; c++) digitalWrite(colPins[c], LOW);
      // Enable current row
      digitalWrite(rowPins[r], HIGH);
      delay(2);
    }
  }
}

// Turn everything off
void allOff() {
  for (int r = 0; r < ROWS; r++) digitalWrite(rowPins[r], LOW);
  for (int c = 0; c < COLS; c++) digitalWrite(colPins[c], HIGH);
}

void setup() {
  for (int i = 0; i < ROWS; i++) pinMode(rowPins[i], OUTPUT);
  for (int i = 0; i < COLS; i++) pinMode(colPins[i], OUTPUT);
  allOff();
}

void loop() {
  // 1. All LEDs ON for 1 second
  lightAll(1000);
  // 2. Scan every LED
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      lightSingleLed(r, c, 250);
      // 250 ms each LED
    }
  }
  // 3. All OFF for 2 seconds
  allOff();
  delay(2000);
}
