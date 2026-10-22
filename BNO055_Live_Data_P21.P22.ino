#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// Define custom I2C pins for ESP32
#define SDA_PIN 21
#define SCL_PIN 22

// Create BNO055 instance (Sensor ID: 55, Default I2C Address: 0x28)
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // Wait for serial monitor to open

  Serial.println("Initializing BNO055...");

  // Initialize I2C bus with custom pins (SDA = 18, SCL = 19)
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize sensor
  if (!bno.begin()) {
    Serial.println("Error: No BNO055 detected! Check wiring or I2C address (0x28 / 0x29).");
    while (1) {
      delay(100);
    }
  }

  delay(1000);

  // Enable external crystal for higher accuracy orientation calculation
  bno.setExtCrystalUse(true);

  Serial.println("BNO055 initialized successfully.");
}

void loop() {
  // Read Euler angles (Heading, Pitch, Roll)
  sensors_event_t orientationData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);

  // Read calibration status (0 = uncalibrated, 3 = fully calibrated)
  uint8_t system, gyro, accel, mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);

  // Display orientation measurements in degrees
  Serial.print("Heading (Yaw): ");
  Serial.print(orientationData.orientation.x, 2);
  Serial.print("° | Pitch: ");
  Serial.print(orientationData.orientation.y, 2);
  Serial.print("° | Roll: ");
  Serial.print(orientationData.orientation.z, 2);
  Serial.print("°");

  // Display calibration status
  Serial.print("  [Calib - Sys:");
  Serial.print(system);
  Serial.print(" G:");
  Serial.print(gyro);
  Serial.print(" A:");
  Serial.print(accel);
  Serial.print(" M:");
  Serial.print(mag);
  Serial.println("]");

  delay(100); // 10 Hz readout rate
  
}