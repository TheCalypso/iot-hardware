#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MCP9808.h>
#include <Adafruit_TSL2591.h>
#include <ArduinoBLE.h>

// Initialisation des capteurs
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

// Service BLE
BLEService environmentService("181A"); // UUID du service BLE standard pour environnement
BLEFloatCharacteristic temperatureCharacteristic("2A6E", BLERead | BLENotify); // Température
BLEFloatCharacteristic lightCharacteristic("2A77", BLERead | BLENotify); // Lumière

// Fonctions pour le MCP9808 (gestion Wake et Shutdown)
void wakeSensor() {
  Wire.beginTransmission(0x18); // Adresse I2C du MCP9808
  Wire.write(0x01);            // Pointer sur le registre de configuration
  Wire.write(0x00);            // Effacer le bit Shutdown (mode normal)
  Wire.write(0x00);
  Wire.endTransmission();
  delay(10); // Petite attente pour permettre au capteur de se réveiller
}

void shutdownSensor() {
  Wire.beginTransmission(0x18); // Adresse I2C du MCP9808
  Wire.write(0x01);            // Pointer sur le registre de configuration
  Wire.write(0x01);            // Activer le bit Shutdown
  Wire.write(0x00);
  Wire.endTransmission();
}

// Configuration initiale
void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialisation du capteur MCP9808
  if (!tempsensor.begin(0x18)) { // Adresse I2C par défaut
    Serial.println("Erreur: Capteur MCP9808 non détecté !");
    while (1);
  }

  // Initialisation du capteur TSL2591
  if (!tsl.begin()) {
    Serial.println("Erreur: Capteur TSL2591 non détecté !");
    while (1);
  }
  tsl.setGain(TSL2591_GAIN_MED);   // Gain moyen
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS); // Temps d'intégration de 100 ms

  // Initialisation du BLE
  if (!BLE.begin()) {
    Serial.println("Erreur: Initialisation du BLE échouée !");
    while (1);
  }

  BLE.setLocalName("BLEcard");  // Name should be set as "BLEcard"
  BLE.setAdvertisedService(environmentService);
  environmentService.addCharacteristic(temperatureCharacteristic);
  environmentService.addCharacteristic(lightCharacteristic);
  BLE.addService(environmentService);
  BLE.advertise();

  Serial.println("Serveur BLE prêt !");
}

// Lecture de la lumière (TSL2591)
float readLight() {
  uint16_t broadband = tsl.getFullLuminosity() >> 16; // Lecture de la lumière visible et IR
  return (float)broadband;
}

// Boucle principale
void loop() {
  // Keep advertising and handle connections
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connecté à : ");
    Serial.println(central.address());

    while (central.connected()) {
      // Réveil du capteur MCP9808
      wakeSensor();

      // Lecture des capteurs
      float temperature = tempsensor.readTempC();
      float light = readLight();

      // Mise à jour des caractéristiques BLE
      temperatureCharacteristic.writeValue(temperature);
      lightCharacteristic.writeValue(light);

      // Affichage des valeurs sur le moniteur série
      Serial.print("Température: ");
      Serial.print(temperature);
      Serial.println(" °C");

      Serial.print("Lumière: ");
      Serial.print(light);
      Serial.println(" lux");

      // Mise en veille du capteur MCP9808
      shutdownSensor();

      delay(3000); // Attente avant la prochaine lecture
    }

    Serial.println("Déconnecté !");
  }
}