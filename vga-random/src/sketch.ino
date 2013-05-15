void setup() {
    DDRD = B11111111;
}

void loop() {
    PORTD = analogRead(0);
}
