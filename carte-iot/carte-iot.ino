#include <ArduinoBLE.h>
#include <Firmata.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "wifiConfig.h"
#include <WiFiNINA.h>

// ST7735 screen configuration
#define TFT_CS    10  // CS pin
#define TFT_RST   9   // Reset pin
#define TFT_DC    8   // Data/Command pin

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// BLE objects
BLEDevice peripheral;
BLECharacteristic temperatureCharacteristic;
BLECharacteristic lightCharacteristic;

// Wi-Fi connection state tracking
int status = WL_IDLE_STATUS;

#define SSID ""          // Replace with your WiFi SSID
#define WIFI_PASSWORD ""  // Replace with your WiFi password

const char* server = "iot.gaspezia.fr"; 
const int port = 443;
float temperature = 0;
float light = 0;
bool bleConnected = false;

void setup() {
  Serial.begin(9600);
  
  // Initialize display
  tft.initR(INITR_BLACKTAB); 
  tft.fillScreen(ST7735_BLACK); 
  tft.setRotation(0); 
  tft.setTextColor(ST7735_WHITE); 
  tft.setTextSize(2); 
  tft.setCursor(0, 0);
  tft.print("Starting...");
}

void loop() {
  initiateBluetoothConnection();
  delay(1000);
  terminateBluetoothConnection();
  delay(1000);
  
  if (bleConnected) {
    connectToWiFi();
    delay(3000);
    sendDataToServer();
    disconnectWiFi();
    bleConnected = false;
  }
  delay(10000); 
}

void initiateBluetoothConnection() {
  Serial.println("Initializing Bluetooth module...");

  while (!Serial);
  BLE.begin();

  if (!BLE.begin()) {
    Serial.println("Error: BLE initialization failed!");
    while (1);
  }

  Serial.println("Searching for Bluetooth devices...");
  BLE.scanForUuid("181A");

  delay(2000);

  peripheral = BLE.available();
  if (peripheral) {
    Serial.print("Device found: ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() == "BLEcard") {
      BLE.stopScan();
      connectToPeripheral(peripheral);
    }
  } else {
    Serial.println("No BLE devices found.");
  }
  delay(2000);
}

void terminateBluetoothConnection() {
  if (BLE.connected()) {
    Serial.println("Disconnecting from BLE device...");
    peripheral.disconnect();
    Serial.println("Disconnected.");
  }
  delay(3000);
  BLE.end();
  Serial.println("BLE module stopped.");
}

void connectToPeripheral(BLEDevice peripheral) {
  Serial.println("Connecting to device...");
  
  if (peripheral.connect()) {
    Serial.println("Connected successfully");
    bleConnected = true;
    delay(1000);
  } else {
    Serial.println("Connection failed.");
    return;
  }

  Serial.println("Discovering attributes...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered!");
    temperatureCharacteristic = peripheral.characteristic("2A6E");
    lightCharacteristic = peripheral.characteristic("2A77");

    if (temperatureCharacteristic && lightCharacteristic) {
      while (peripheral.connected()) {
        if (temperatureCharacteristic.canRead()) {
          uint8_t tempBuffer[4];
          temperatureCharacteristic.readValue(tempBuffer, sizeof(tempBuffer));
          memcpy(&temperature, tempBuffer, sizeof(temperature));
          Serial.print("Temperature: ");
          Serial.print(temperature, 2);
          Serial.println(" °C");
        }

        if (lightCharacteristic.canRead()) {
          uint8_t lightBuffer[4]; 
          lightCharacteristic.readValue(lightBuffer, sizeof(lightBuffer));
          memcpy(&light, lightBuffer, sizeof(light));
          Serial.print("Light: ");
          Serial.print(light, 2);
          Serial.println(" lux");
        }

        tft.fillScreen(ST7735_BLACK); 
        tft.setCursor(0, 0);
        tft.print("Temp: ");
        tft.setCursor(0, 20);
        tft.print(temperature, 1);
        tft.println(" C");

        tft.setCursor(0, 50);
        tft.print("Lum: ");
        tft.setCursor(0, 70);
        tft.print(light, 1);
        tft.println(" lux");
        return;
      }
    } else {
      Serial.println("Missing characteristics!");
    }
  } else {
    Serial.println("Failed to discover attributes!");
  }
  delay(2000);
}

void connectToWiFi() {
  Serial.print("Connecting to network: ");
  Serial.println(ssid);
  delay(1000);

  WiFi.begin(ssid, wpa_passphrase);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    attempts++;
    Serial.print(".");
    delay(1000);
    if (attempts > 10) {
      Serial.println("Failed to connect to Wi-Fi.");
      return;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected!");
  } else {
    Serial.println("Wi-Fi connection failed.");
  }
}

void disconnectWiFi() {
  delay(2000);
  WiFi.disconnect();
  Serial.println("Disconnected from Wi-Fi.");
  
  delay(2000);
  WiFi.end();
}

String sendPOSTRequest(const char* path, int sensorId, int value) {
  String response = "";
  String postData = "{\"sensorId\":" + String(sensorId) + ",\"value\":" + String(value) + "}";

  WiFiSSLClient client;  // Remplace WiFiClient par WiFiSSLClient

  if (client.connect(server, port)) {
    Serial.println("Connecté au serveur HTTPS !");
    
    // Construire la requête HTTP
    client.println(String("POST ") + path + " HTTP/1.1");
    client.println(String("Host: ") + server);
    client.println("Content-Type: application/json");
    client.println("User-Agent: Arduino/1.0");  // Ajout User-Agent
    client.println("Accept: */*");  // Ajout Accept header
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println(); 
    client.println(postData);

    // Lire la réponse du serveur
    String statusLine = client.readStringUntil('\n');
    Serial.println("HTTP Status: " + statusLine);

    if (!statusLine.startsWith("HTTP/1.1 201")) {
      Serial.println("Erreur: Statut HTTP non 201 !");
      client.stop();
      return "";
    }

    // Lire les en-têtes jusqu'à la ligne vide
    while (client.connected()) {
      String header = client.readStringUntil('\n');
      if (header == "\r") break;
    }

    // Lire la réponse du serveur
    while (client.available()) {
      response += client.readString();
    }

    client.stop();
    Serial.println("Réponse reçue !");
  } else {
    Serial.println("Échec de connexion au serveur HTTPS !");
  }

  return response;
}
void sendDataToServer() {
  // Send separate POST requests for each sensor
String tempResponse = sendPOSTRequest("/api/sensor-data", 1, temperature);
  if (!tempResponse.isEmpty()) {
    displayResponse(tempResponse);
  }

  String lightResponse = sendPOSTRequest("/api/sensor-data", 2, light);
  if (!lightResponse.isEmpty()) {
    displayResponse(lightResponse);
  }
}

void displayResponse(const String& response) {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);

  if (response.isEmpty()) {
    tft.println("No response");
  } else {
    tft.println("Response:");

    int lineStart = 0;
    int lineLength = 20;
    for (int i = 0; i < response.length(); i += lineLength) {
      tft.setCursor(0, 20 + (i / lineLength) * 10);
      tft.println(response.substring(i, i + lineLength));
    }
  }
}