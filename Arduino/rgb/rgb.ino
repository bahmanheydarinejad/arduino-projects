int pins [] = { 10, 11, 12 };

void setup() {
  pinMode(pins[0], OUTPUT);
  pinMode(pins[1], OUTPUT);
  pinMode(pins[2], OUTPUT);
}

void loop() {

  RGB_output(255, 0, 0);  //red color

  delay(1000);

  RGB_output(0, 255, 0);  //lime color

  delay(1000);

  RGB_output(0, 0, 255);  //blue color

  delay(1000);

  RGB_output(255, 255, 255);  //white color

  delay(1000);

  RGB_output(128, 0, 0);  //maroon color

  delay(1000);

  RGB_output(0, 128, 0);  //green color

  delay(1000);

  RGB_output(128, 128, 0);  //olive

  delay(1000);

  RGB_output(0, 0, 0);  //black color

  delay(1000);
}

void RGB_output(int redLight, int greenLight, int blueLight)

{
  analogWrite(pins[0], redLight);
  analogWrite(pins[1], greenLight);
  analogWrite(pins[2], blueLight);
}
