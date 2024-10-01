#include <WiFiNINA.h>
#include <ArduinoJson.h>

// Remplace par les informations de ton réseau Wi-Fi
const char* ssid = "Pixel 6 Bastien";  // Nom du réseau Wi-Fi (par exemple le partage de connexion de ton téléphone)
const char* password = "Gaspezia";  // Mot de passe du réseau Wi-Fi

const char* server = "iot.gaspezia.fr";

// Objet Wi-Fi client
WiFiSSLClient client;

void setup() {
  // Initialisation du moniteur série pour débogage
  Serial.begin(9600);
  
  // Tentative de connexion au réseau Wi-Fi
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("Connecté au Wi-Fi !");
}

void loop() {
  // Vérification de la connexion Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    // Si la connexion Wi-Fi est active, faire une requête GET via HTTPS
    faireRequeteGET();
  } else {
    Serial.println("Connexion Wi-Fi perdue !");
  }

  delay(10000);  // Attendre 10 secondes avant de refaire la requête GET
}

void faireRequeteGET() {
  // Connexion au serveur via HTTPS sur le port 443 (HTTPS)
  if (client.connect(server, 443)) {
    Serial.println("Connexion au serveur réussie (HTTPS) !");

    // Envoi de la requête HTTP GET sur /api
    client.println("GET /api/ HTTP/1.1");  // Appel de l'API
    client.println("Host: iot.gaspezia.fr");
    client.println("Connection: close");
    client.println();  // Ligne vide pour terminer la requête

    // Attendre la réponse du serveur
    while (client.connected() || client.available()) {
      if (client.available()) {
        String ligne = client.readStringUntil('\n');  // Lire ligne par ligne
        Serial.println(ligne);  // Afficher la réponse dans le moniteur série
      }
    }

    // Déconnexion du serveur
    client.stop();
    Serial.println("Déconnecté du serveur.");
  } else {
    Serial.println("Échec de la connexion au serveur HTTPS !");
  }
}

