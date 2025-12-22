const int gasPin = A0;         // LPG Gas sensor analog pin
const int flamePin = 2;        // Flame sensor digital pin
const int buzzerPin = 10;      // Buzzer pin
const int redLedPin = 9;       // Red LED pin (emergency)
const int greenLedPin = 8;     // Green LED pin (normal)
const int relayPin = 3;        // Relay pin for water spray motor

const int gasThreshold = 300;  // Adjust this threshold as per your gas sensor's sensitivity

void setup() {
  pinMode(gasPin, INPUT);
  pinMode(flamePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(relayPin, HIGH);
}

void loop() {
  int gasValue = analogRead(gasPin);
  bool flameDetected = digitalRead(flamePin) == HIGH; // HIGH means flame detected

  if (flameDetected) {
    // Emergency: Flame detected
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(relayPin, LOW); // Activate water spray
  }
  else if (gasValue > gasThreshold) {
    // Emergency: Gas detected, no flame
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(relayPin, HIGH); // Do not activate water spray
  }
  else {
    // Normal operation
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(buzzerPin, LOW);
    digitalWrite(relayPin, HIGH); // Spray OFF
  }

  delay(200); // Prevents rapid toggling
}