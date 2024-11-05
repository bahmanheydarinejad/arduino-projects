// include the library code:
#include <LiquidCrystal.h>

// Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int counter;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Clears the LCD screen
  lcd.clear();

  // set the cursor to column 0, line 0
  lcd.setCursor(0, 1);

  lcd.print("Hello!");

  int counter = 0;
}

void loop() {
  delay(500);
  lcd.clear();

  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(counter > 15 ? counter % 15 : counter, 1);

  // Print a message to the LCD.
  lcd.print(counter++);
}
