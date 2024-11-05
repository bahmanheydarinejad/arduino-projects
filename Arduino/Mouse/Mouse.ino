// joystick pins
const int SWITCH = 3;  // digital pin 2 connected to SW output of JoyStick
const int X_AX = A0;   // analog pin 0 connected to X output of JoyStick
const int Y_AX = A1;   // analog pin 1 connected to Y output of JoyStick

int rows = 0;

void setup() {
  pinMode(SWITCH, INPUT);
  digitalWrite(SWITCH, HIGH);
  Serial.begin(115200);
}
void loop() {
  Serial.print("Switch: ");
  Serial.print(digitalRead(SWITCH));
  Serial.print("     X AX: ");
  Serial.print(analogRead(X_AX));
  Serial.print("     Y AX: ");
  Serial.println(analogRead(Y_AX));
  delay(50);
}