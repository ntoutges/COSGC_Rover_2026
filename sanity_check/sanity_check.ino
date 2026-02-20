// Sanity check: Used to ensure your chip is at least functional!

void setup() {

  // Start serial communications at 115200 baud, and wait for serial monitor to connect
  Serial.begin(115200);
  while (!Serial);
}

void loop() {

  // Occasionally print out a heartbeat message
  Serial.print("(");
  Serial.print(millis());
  Serial.print(") Hello!");

  delay(1000);
}
