#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Pin definitions for TFT display
#define TFT_CS     10
#define TFT_RST    8
#define TFT_DC     9

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// BLE device and characteristics
BLEDevice peripheral;
BLECharacteristic tempCharacteristic;
BLECharacteristic lightCharacteristic;

const char* deviceName = "Nano33Sense";
float prevTempValue = -999.0;
uint16_t prevLightValue = 0;
unsigned long lastReconnectAttempt = 0;
const long reconnectInterval = 10000; // Retry every 10 seconds if disconnected

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize the TFT display
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("BLE initialization failed!");
    while (1);
  }
  Serial.println("Scanning for BLE devices...");
  BLE.scan();
}

void loop() {
  // Check if the peripheral is available
  peripheral = BLE.available();

  if (peripheral && peripheral.localName() == deviceName) {
    Serial.print("Found device: ");
    Serial.println(peripheral.address());
    BLE.stopScan();

    if (peripheral.connect()) {
      Serial.println("Connected to BLE device");
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0, 0);
      tft.println("Connected to BLE");

      Serial.println("================= " + peripheral.discoverAttributes());

      if (peripheral.discoverAttributes()) {
        Serial.println("================");
        tempCharacteristic = peripheral.characteristic("12345678-1234-5678-1234-56789abcdef1");
        lightCharacteristic = peripheral.characteristic("12345678-1234-5678-1234-56789abcdef2");

        if (tempCharacteristic && lightCharacteristic) {
          Serial.println("Characteristics discovered");

          // Loop while connected
          while (peripheral.connected()) {
            Serial.println("test test test")
            float tempValue = 0;
            uint16_t lightValue = 0;

            // Read temperature characteristic
            if (tempCharacteristic && tempCharacteristic.canRead()) {
              if (!tempCharacteristic.readValue((uint8_t*)&tempValue, sizeof(tempValue))) {
                Serial.println("Error reading temperature characteristic");
              }
            }

            // Read light characteristic
            if (lightCharacteristic && lightCharacteristic.canRead()) {
              if (!lightCharacteristic.readValue((uint8_t*)&lightValue, sizeof(lightValue))) {
                Serial.println("Error reading light characteristic");
              }
            }

            if (tempValue != prevTempValue || lightValue != prevLightValue) {
              tft.fillRect(0, 20, 128, 80, ST77XX_BLACK);
              tft.setCursor(0, 20);
              tft.print("Temp: ");
              tft.print(tempValue);
              tft.println(" C");
              tft.print("Light: ");
              tft.print(lightValue);
              tft.println(" lux");

              prevTempValue = tempValue;
              prevLightValue = lightValue;
            }
            delay(500); // Reduce read frequency
          }
        }
      }

      Serial.println("Disconnected from BLE device");
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0, 0);
      tft.println("Disconnected");

      // Delay before trying to reconnect
      delay(1000); // Wait before scanning again
      BLE.scan();
    } else {
      Serial.println("Failed to connect. Waiting before scanning...");
      delay(1000); // Wait before retrying
      BLE.scan();
    }
  } else {
    // Attempt reconnection after a certain period if disconnected
    if (millis() - lastReconnectAttempt > reconnectInterval) {
      lastReconnectAttempt = millis();
      Serial.println("Re-scanning for BLE device...");
      delay(500); // Short delay before scanning
      BLE.scan();
    }
  }
}
