// Définition de la LED intégrée sur la carte Arduino Nano 33 BLE
int ledPin = LED_BUILTIN;

void setup() {
  // Initialisation de la LED comme une sortie
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Allumer la LED
  digitalWrite(ledPin, HIGH);
  // Attendre 1 seconde
  delay(1000);
  // Éteindre la LED
  digitalWrite(ledPin, LOW);
  // Attendre 1 seconde
  delay(1000);
}
