// the setup function runs once when you press reset or power the board
void setup() {
    pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin as an output
}

// the loop function runs over and over again forever
void loop() {
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on
    delay(1000);                     // wait for 1 second
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off
    delay(1000);                     // wait for 1 second
}
