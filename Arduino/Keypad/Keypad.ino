#include <Keypad.h>

const byte ROWS = 1;  //four rows
const byte COLS = 4;  //four columns

char keys[ROWS][COLS] = {
  { '1', '2', '3', '4' }
};

byte rowPins[ROWS] = { 2 };           //connect to the row pinouts of the keypad
byte colPins[COLS] = { 5, 6, 3, 4 };  //connect to the column pinouts of the keypad

//Create an object of keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
}

void loop() {
  char key = keypad.getKey();  // Read the key

  // Print if key pressed
  if (key) {
    Serial.print("Key Pressed : ");
    Serial.println(key);
  }
}