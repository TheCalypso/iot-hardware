#include <Wire.h>
#include <ArduinoBLE.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_MCP9808.h>

// Create instances of the sensors
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// BLE service and characteristics with custom UUIDs
BLEService sensorService("12345678-1234-5678-1234-56789abcdef0");
BLEFloatCharacteristic tempCharacteristic("12345678-1234-5678-1234-56789abcdef1", BLERead | BLENotify);
BLEUnsignedIntCharacteristic lightCharacteristic("12345678-1234-5678-1234-56789abcdef2", BLERead | BLENotify);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Initializing sensors and BLE...");

  // Initialize sensors
  if (!tsl.begin()) {
    Serial.println("Error: TSL2591 sensor not found!");
    while (1);
  }
  Serial.println("TSL2591 detected.");

  if (!tempsensor.begin(0x18)) {
    Serial.println("Error: MCP9808 sensor not found!");
    while (1);
  }
  Serial.println("MCP9808 detected.");

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Error: BLE initialization failed!");
    while (1);
  }

  BLE.setLocalName("Nano33Sense");
  BLE.setAdvertisedService(sensorService);

  sensorService.addCharacteristic(tempCharacteristic);
  sensorService.addCharacteristic(lightCharacteristic);
  BLE.addService(sensorService);

  tempCharacteristic.writeValue(0.0);
  lightCharacteristic.writeValue(0);
  BLE.advertise();

  Serial.println("BLE is advertising!");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      uint16_t visible = tsl.getLuminosity(TSL2591_VISIBLE);
      float temperature = tempsensor.readTempC();

      tempCharacteristic.writeValue(temperature);
      lightCharacteristic.writeValue(visible);

      Serial.print("Temp: ");
      Serial.print(temperature);
      Serial.print(" °C | Light: ");
      Serial.print(visible);
      Serial.println(" lux");

      delay(2000); // Reduce data update frequency
    }

    Serial.println("Central disconnected. Restarting BLE advertising...");
    BLE.advertise(); // Restart advertising after disconnection
  }
}
