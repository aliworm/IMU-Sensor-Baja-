/*
  ESP32-WROOM-32 + Adafruit BNO055 -> RAW sensor data on Serial Monitor
  I2C pins: SDA = GPIO18, SCL = GPIO19

  WIRING:
    BNO055 VIN -> ESP32 3V3
    BNO055 GND -> ESP32 GND
    BNO055 SDA -> ESP32 GPIO18
    BNO055 SCL -> ESP32 GPIO19

  LIBRARIES: Adafruit BNO055, Adafruit Unified Sensor, Adafruit BusIO
  SERIAL MONITOR: 115200 baud

  Runs the chip in AMG mode (raw accel / mag / gyro, no sensor fusion).
  Raw values are signed 16-bit counts straight from the chip registers:
    Accel: 1 m/s^2 = 100 LSB      Mag:  1 uT   = 16 LSB      Gyro: 1 dps = 16 LSB
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define SDA_PIN 18
#define SCL_PIN 19

// BNO055 register: first byte of the accel/mag/gyro data block
#define REG_ACC_DATA_X_LSB 0x08

uint8_t bnoAddr = 0;
Adafruit_BNO055 *bno = nullptr;

// Scan the bus and print everything found. Returns BNO055 address (0x28/0x29) or 0.
uint8_t scanBus() {
  uint8_t bnoFound = 0;
  int count = 0;
  Serial.println("Scanning I2C bus...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  Found device at 0x%02X\n", addr);
      count++;
      if (addr == 0x28 || addr == 0x29) bnoFound = addr;
    }
  }
  if (count == 0) Serial.println("  No I2C devices found.");
  return bnoFound;
}

// Read len bytes starting at reg directly over I2C
bool readBytes(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(bnoAddr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)bnoAddr, (int)len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

int16_t toInt16(uint8_t lsb, uint8_t msb) {
  return (int16_t)((msb << 8) | lsb);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP32 + BNO055 RAW data (SDA=18, SCL=19)");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);   // 100 kHz, safest for the BNO055

  bnoAddr = scanBus();
  if (bnoAddr == 0) {
    Serial.println("ERROR: No BNO055 found at 0x28 or 0x29.");
    Serial.println("Check: VIN to 3V3, GND to GND, SDA to GPIO18, SCL to GPIO19.");
    while (true) delay(1000);
  }
  Serial.printf("BNO055 found at 0x%02X\n", bnoAddr);

  bno = new Adafruit_BNO055(55, bnoAddr, &Wire);
  if (!bno->begin(OPERATION_MODE_AMG)) {   // raw accel + mag + gyro
    Serial.println("ERROR: BNO055 responded to scan but begin() failed (wrong chip ID?).");
    while (true) delay(1000);
  }
  delay(1000);
  bno->setExtCrystalUse(true);
  Serial.println("BNO055 ready. Streaming raw data...\n");
}

void loop() {
  uint8_t d[18];
  if (!readBytes(REG_ACC_DATA_X_LSB, d, 18)) {
    Serial.println("I2C read failed");
    delay(500);
    return;
  }

  // Register order: ACC (6), MAG (6), GYR (6)
  int16_t ax = toInt16(d[0],  d[1]),  ay = toInt16(d[2],  d[3]),  az = toInt16(d[4],  d[5]);
  int16_t mx = toInt16(d[6],  d[7]),  my = toInt16(d[8],  d[9]),  mz = toInt16(d[10], d[11]);
  int16_t gx = toInt16(d[12], d[13]), gy = toInt16(d[14], d[15]), gz = toInt16(d[16], d[17]);

  Serial.printf("ACC raw  X:%6d Y:%6d Z:%6d  (%.2f, %.2f, %.2f m/s2)\n",
                ax, ay, az, ax / 100.0, ay / 100.0, az / 100.0);
  Serial.printf("MAG raw  X:%6d Y:%6d Z:%6d  (%.2f, %.2f, %.2f uT)\n",
                mx, my, mz, mx / 16.0, my / 16.0, mz / 16.0);
  Serial.printf("GYR raw  X:%6d Y:%6d Z:%6d  (%.2f, %.2f, %.2f dps)\n",
                gx, gy, gz, gx / 16.0, gy / 16.0, gz / 16.0);
  Serial.printf("Temp: %d C\n\n", bno->getTemp());

  delay(100);
}
