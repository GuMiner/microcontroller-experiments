#include <Arduino.h>
#include <Wire.h>
#include "CDSensor.h"

#define ADDR_6700  0x15 // I2C address of the C02 sensor
#define readDelay 10  // delay between I2C write & read requests in ms (10 recommended)
#define measureDelay 2250  // delay between measure and read PPM requests in ms (2250 min recommended)

byte data[6]; // Buffer for I2C operations


unsigned long GetSerialNumber() {
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x03); Wire.write(0x0F); Wire.write(0xA2); Wire.write(0x00); Wire.write(0x02);
  Wire.endTransmission();

  delay(readDelay);

  Wire.requestFrom(ADDR_6700, 6);
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();
  data[4] = Wire.read();
  data[5] = Wire.read();

  return ((unsigned long)data[2] << 24) | ((unsigned long)data[3] << 16) | ((unsigned long)data[4] << 8) | (unsigned long)data[5];
}

unsigned int GetFirmwareVersion() {
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x89); Wire.write(0x00); Wire.write(0x01);
  Wire.endTransmission();

  delay(readDelay);

  Wire.requestFrom(ADDR_6700, 4);
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();
  return ((unsigned int)data[2] << 8) | ((unsigned int)data[3]);
}

bool IsDeviceHealthy() {
  Wire.beginTransmission(ADDR_6700);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x8A); Wire.write(0x00); Wire.write(0x01);
  Wire.endTransmission();

  delay(readDelay);

  Wire.requestFrom(ADDR_6700, 4);
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();

  return (data[0] == 0x04 && data[3] == 0x00); // response & status == ok
}

int GetCO2PPM() {
    Wire.beginTransmission(ADDR_6700);
    Wire.write(0x04); Wire.write(0x13); Wire.write(0x8B); Wire.write(0x00); Wire.write(0x01);
    Wire.endTransmission();
    
    delay(measureDelay);
    Wire.requestFrom(ADDR_6700, 4);
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();

    return ((data[2] & 0x3F ) << 8) | data[3];
}
