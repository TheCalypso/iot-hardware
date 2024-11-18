#include <WiFiNINA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "credentials.h"

// Configuration
const char* server = "iot.gaspezia.fr";
const int port = 443; // HTTPS port

WiFiSSLClient client;
Adafruit_ST7735 tft = Adafruit_ST7735(10, 9, 8);

void setup() {
  Serial.begin(9600);

  // Initialisation de l'écran
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.println("Demarrage...");

  // Connexion Wi-Fi
  connectWiFi();
}

void loop() {
  String response = sendGETRequest("/api/");
  if (!response.isEmpty()) {
    displayResponse(response);
  }

  delay(10000); // Attendre 10 secondes
}

void connectWiFi() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(10, 10);
  tft.println("Connexion Wi-Fi...");

  int retries = 0;
  while (WiFi.begin(SSID, WIFI_PASSWORD) != WL_CONNECTED && retries < 10) {
    delay(1000);
    tft.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connecté au Wi-Fi !");
    tft.setCursor(10, 30);
    tft.println("Connecté !");
  } else {
    Serial.println("Echec de connexion Wi-Fi !");
    tft.setCursor(10, 30);
    tft.println("Echec !");
  }
}

String sendGETRequest(const char* path) {
  String response = "";

  if (client.connect(server, port)) {
    Serial.println("Connecté au serveur HTTPS !");
    client.println(String("GET ") + path + " HTTP/1.1");
    client.println(String("Host: ") + server);
    client.println("Connection: close");
    client.println();

    // Vérification du statut HTTP
    String statusLine = client.readStringUntil('\n');
    Serial.println("Statut HTTP: " + statusLine);

    if (!statusLine.startsWith("HTTP/1.1 200")) {
      Serial.println("Erreur: Le serveur a répondu avec un statut non 200 !");
      client.stop();
      return ""; // Retourne une réponse vide en cas d'erreur
    }

    // Lire les en-têtes jusqu'à la ligne vide
    while (client.connected()) {
      String header = client.readStringUntil('\n');
      if (header == "\r") break;
    }

    // Lire le corps de la réponse
    while (client.available()) {
      response += client.readString();
    }

    client.stop();
    Serial.println("Reponse recue !");
  } else {
    Serial.println("Echec de connexion au serveur HTTPS !");
  }

  return response;
}


void displayResponse(const String& response) {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(10, 10);

  if (response.isEmpty()) {
    tft.println("Aucune reponse");
  } else {
    tft.println("Reponse:");

    int lineStart = 0;
    int lineLength = 20; // Nombre de caractères par ligne
    for (int i = 0; i < response.length(); i += lineLength) {
      tft.setCursor(10, 30 + (i / lineLength) * 10);
      tft.println(response.substring(i, i + lineLength));
    }
  }
}

