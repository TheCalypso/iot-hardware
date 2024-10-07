#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "credentials.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

const char* server = "iot.gaspezia.fr";

// Initialisation de l'objet écran (utilise le mode matériel SPI)
Adafruit_ST7735 tft = Adafruit_ST7735(10, 9, 8);

// Objet Wi-Fi client
WiFiSSLClient client;

void setup() {
  // Initialisation du moniteur série pour débogage
  Serial.begin(9600);
  
  // Initialisation de l'écran
  tft.initR(INITR_BLACKTAB);  // Type d'initialisation pour l'écran RB TFT 1.8"
  tft.fillScreen(ST7735_BLACK);  // Efface l'écran et remplit en noir
  
  // Définir la couleur de texte et le fond
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);  // Texte blanc sur fond noir
  tft.setTextSize(1);  // Taille du texte

  // Message de démarrage sur l'écran
  tft.setCursor(10, 10);
  tft.println("Demarrage...");
  
  // Tentative de connexion au réseau Wi-Fi
  tft.setCursor(10, 30);
  tft.println("Connexion Wi-Fi...");
  while (WiFi.begin(SSID, WIFI_PASSWORD) != WL_CONNECTED) {
    Serial.print(".");
    tft.print(".");  // Affiche des points de progression sur l'écran
    delay(1000);
  }

  Serial.println("Connecté au Wi-Fi !");
  tft.setCursor(10, 50);
  tft.println("Connecté au Wi-Fi!");
}

void loop() {
  // Vérification de la connexion Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    // Si la connexion Wi-Fi est active, faire une requête GET via HTTPS
    faireRequeteGET();
  } else {
    Serial.println("Connexion Wi-Fi perdue !");
    tft.setCursor(10, 70);
    tft.println("Wi-Fi perdu!");
  }

  delay(10000);  // Attendre 10 secondes avant de refaire la requête GET
}

void faireRequeteGET() {
  // Connexion au serveur via HTTPS sur le port 443 (HTTPS)
  if (client.connect(server, 443)) {
    Serial.println("Connexion au serveur réussie (HTTPS) !");
    tft.setCursor(10, 90);
    tft.println("Connecté au serveur!");

    // Envoi de la requête HTTP GET sur /api
    client.println("GET /api/ HTTP/1.1");  // Appel de l'API
    client.println("Host: iot.gaspezia.fr");
    client.println("Connection: close");
    client.println();  // Ligne vide pour terminer la requête

    // Attendre la réponse du serveur
    String response = "";  // Variable pour stocker la réponse complète
    bool headersEnded = false; // Indicateur pour la fin des en-têtes HTTP

    while (client.connected() || client.available()) {
      if (client.available()) {
        String ligne = client.readStringUntil('\n');  // Lire ligne par ligne
        Serial.println(ligne);  // Afficher la réponse dans le moniteur série
        
        // Si une ligne vide est rencontrée, cela signifie la fin des en-têtes HTTP
        if (ligne == "\r") {
          headersEnded = true; // Les en-têtes sont terminés, le reste est le body
        }

        // Si les en-têtes sont terminés, stocker la ligne de contenu
        if (headersEnded) {
          response += ligne + "\n";
        }
      }
    }

    // Afficher le contenu (body) de la réponse sur l'écran
    tft.setCursor(10, 10);
    tft.fillScreen(ST7735_BLACK);  // Efface l'écran avant d'afficher la nouvelle réponse
    tft.println("Reponse du serveur:");
    tft.setCursor(10, 30);
    tft.println(response.substring(0, 60));  // Afficher les 60 premiers caractères

    // Déconnexion du serveur
    client.stop();
    Serial.println("Déconnecté du serveur.");
    tft.setCursor(10, 50);
    tft.println("Serveur deconnecte.");
  } else {
    Serial.println("Échec de la connexion au serveur HTTPS !");
    tft.setCursor(10, 70);
    tft.println("Echec connexion serveur!");
  }
}
