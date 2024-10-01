#include <ArduinoBLE.h>

// Déclaration du service et de la caractéristique BLE
BLEService messageService("12345678-1234-1234-1234-123456789abc"); // UUID du service
BLEStringCharacteristic messageCharacteristic("87654321-4321-4321-4321-cba987654321", BLERead | BLENotify, 20); // UUID de la caractéristique, avec une longueur maximale de 20 caractères

void setup() {
  Serial.begin(9600);
  
  // Initialisation du BLE
  if (!BLE.begin()) {
    Serial.println("Erreur d'initialisation du BLE");
    while (1);
  }
  
  // Configuration du service BLE
  BLE.setLocalName("Nano33BLE-Sense");  // Nom du périphérique BLE
  BLE.setAdvertisedService(messageService); // Faire la publicité du service
  
  // Ajout de la caractéristique au service
  messageService.addCharacteristic(messageCharacteristic);
  
  // Ajout du service BLE
  BLE.addService(messageService);
  
  // Démarrer la publicité BLE
  BLE.advertise();
  
  Serial.println("Périphérique BLE prêt !");
}

void loop() {
  // Envoyer le message "Test" toutes les 5 secondes
  messageCharacteristic.writeValue("Test");
  Serial.println("Message envoyé : Test");
  
  delay(5000);  // Attendre 5 secondes
}
