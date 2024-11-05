int[] segments = new int[] {
  2, 3, 4, 5, 6, 7, 8, 9
}
int delayTime = 300;

void setup() {
  // put your setup code here, to run once:
  for (s : segments) {
    pinMode(s, OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(delayTime);
  
  delay(delayTime);
}
