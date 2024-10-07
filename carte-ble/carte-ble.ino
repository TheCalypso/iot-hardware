// Include required libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_MCP9808.h>

// Create sensor objects
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// Define arbitrary IDs for sensors
const uint8_t TSL2591_ID = 0x01; // Custom ID for TSL2591
const uint8_t MCP9808_ID = 0x02; // Custom ID for MCP9808

void setup() {
  Serial.begin(115200);
  
  // Initialize TSL2591 light sensor
  if (!tsl.begin()) {
    Serial.println("Couldn't find TSL2591 sensor!");
    while (1);
  }
  Serial.println("TSL2591 Light Sensor initialized");

  // Set gain and timing for TSL2591
  tsl.setGain(TSL2591_GAIN_MED);  // Options: LOW, MED, HIGH, MAX
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS); // Options: 100MS to 600MS

  // Read the chip version from the status register
  uint8_t chipVersion = tsl.getStatus() & 0x0F; // Get version bits
  Serial.print("TSL2591 Chip Version: 0x");
  Serial.println(chipVersion, HEX); // Print version in hex

  // Initialize MCP9808 temperature sensor
  if (!tempsensor.begin(0x18)) {
    Serial.println("Couldn't find MCP9808 sensor!");
    while (1);
  }
  Serial.println("MCP9808 Temperature Sensor initialized");

  // Read manufacturer and device IDs from MCP9808
  uint16_t manufacturerID = tempsensor.read16(0x06);  // Manufacturer ID register
  uint16_t deviceID = tempsensor.read16(0x07);        // Device ID register
  
  Serial.print("MCP9808 Manufacturer ID: 0x"); Serial.println(manufacturerID, HEX);
  Serial.print("MCP9808 Device ID: 0x"); Serial.println(deviceID, HEX);
}

void loop() {
  // Read TSL2591 light sensor data and calculate lux
  uint32_t full_spectrum = tsl.getFullLuminosity(); // Get combined IR + visible
  uint16_t infrared = full_spectrum >> 16;          // IR data
  uint16_t visible = full_spectrum & 0xFFFF;        // Visible data

  // Calculate lux
  float lux = tsl.calculateLux(visible, infrared);
  
  if (lux < 0) {
    Serial.println("Lux calculation error. Please check sensor configuration.");
  } else {
    Serial.print("TSL2591 ID: 0x"); Serial.print(TSL2591_ID, HEX); // Print custom ID
    Serial.print(" | Lux: "); Serial.println(lux);
  }

  // Read MCP9808 temperature sensor data
  float temperature = tempsensor.readTempC();
  
  Serial.print("MCP9808 ID: 0x"); Serial.print(MCP9808_ID, HEX); // Print custom ID
  Serial.print(" | Temperature: "); Serial.print(temperature); Serial.println(" *C");
  
  // Delay between readings
  delay(2000); // Delay 2 seconds
}