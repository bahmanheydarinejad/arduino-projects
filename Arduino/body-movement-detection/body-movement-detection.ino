int ledPin = 13;
int ledPins[] = { 2, 3, 4, 5, 6, 7, 8, 9 };
int inputPin = 10;
int pirState = LOW;
int val = 0;

void setup() {
  int index;
  for (index = 0; index <= 7; index++) {
    pinMode(ledPins[index], OUTPUT);
  }
  pinMode(ledPin, OUTPUT);
  pinMode(inputPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  val = digitalRead(inputPin);
  if (val == HIGH) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Motion detected!");
    pingPong();

  } else {
    digitalWrite(ledPin, LOW);
  }
}

void pingPong() {
  int index;
  int delayTime = 10;

  for (index = 0; index <= 7; index++) {
    digitalWrite(ledPins[index], HIGH);
    delay(delayTime);
    digitalWrite(ledPins[index], LOW);
  }

  for (index = 7; index >= 0; index--) {
    digitalWrite(ledPins[index], HIGH);
    delay(delayTime);
    digitalWrite(ledPins[index], LOW);
  }
}