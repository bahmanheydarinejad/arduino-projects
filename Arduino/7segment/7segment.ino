int segmentPins[][7] = {
  { 2, 3, 4, 5, 6, 7 },
  { 3, 4 },
  { 2, 3, 5, 6, 8 },
  { 2, 3, 4, 5, 8 },
  { 3, 4, 7, 8 },
  { 2, 4, 5, 7, 8 },
  { 2, 4, 5, 6, 7, 8 },
  { 2, 3, 4 },
  { 2, 3, 4, 5, 6, 7, 8 },
  { 2, 3, 4, 5, 7, 8 }
};

int pins[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

int buzzPin = 12;
int pinDP = 9;
int delayTime = 1000;
int size = 10;

void setup() {

  for (int pinNumber : pins) {
    pinMode(pinNumber, OUTPUT);
  }

  digitalWrite(pinDP, HIGH);
  digitalWrite(buzzPin, LOW);
}

void loop() {

  delay(delayTime);
  digitalWrite(buzzPin, LOW);
  for (int row = size - 1; row >= 0; row--) {
    for (int col = 0; col < (sizeof(segmentPins[row]) / sizeof(int)); col++) {
      digitalWrite(segmentPins[row][col], HIGH);
    }
    delay(delayTime);
    for (int pinNumber : pins) {
      digitalWrite(pinNumber, LOW);
    }
  }
  digitalWrite(buzzPin, HIGH);
  delay(delayTime);
}
