#include <TinyWireM.h>
#include "CDSensor.h"

#define ADDR_6700  0x15 // I2C address of the C02 sensor
#define readDelay 10  // delay between I2C write & read requests in ms (10 recommended)
#define measureDelay 2250  // delay between measure and read PPM requests in ms (2250 min recommended)

byte data[6]; // Buffer for I2C operations


unsigned long GetSerialNumber() {
  TinyWireM.beginTransmission(ADDR_6700);
  TinyWireM.write(0x03); TinyWireM.write(0x0F); TinyWireM.write(0xA2); TinyWireM.write(0x00); TinyWireM.write(0x02);
  TinyWireM.endTransmission();

  delay(readDelay);

  TinyWireM.requestFrom(ADDR_6700, 6);
  data[0] = TinyWireM.read();
  data[1] = TinyWireM.read();
  data[2] = TinyWireM.read();
  data[3] = TinyWireM.read();
  data[4] = TinyWireM.read();
  data[5] = TinyWireM.read();

  return ((unsigned long)data[2] << 24) | ((unsigned long)data[3] << 16) | ((unsigned long)data[4] << 8) | (unsigned long)data[5];
}

unsigned int GetFirmwareVersion() {
  TinyWireM.beginTransmission(ADDR_6700);
  TinyWireM.write(0x04); TinyWireM.write(0x13); TinyWireM.write(0x89); TinyWireM.write(0x00); TinyWireM.write(0x01);
  TinyWireM.endTransmission();

  delay(readDelay);

  TinyWireM.requestFrom(ADDR_6700, 4);
  data[0] = TinyWireM.read();
  data[1] = TinyWireM.read();
  data[2] = TinyWireM.read();
  data[3] = TinyWireM.read();
  return ((unsigned int)data[2] << 8) | ((unsigned int)data[3]);
}

bool IsDeviceHealthy() {
  TinyWireM.beginTransmission(ADDR_6700);
  TinyWireM.write(0x04); TinyWireM.write(0x13); TinyWireM.write(0x8A); TinyWireM.write(0x00); TinyWireM.write(0x01);
  TinyWireM.endTransmission();

  delay(readDelay);

  TinyWireM.requestFrom(ADDR_6700, 4);
  data[0] = TinyWireM.read();
  data[1] = TinyWireM.read();
  data[2] = TinyWireM.read();
  data[3] = TinyWireM.read();

  return (data[0] == 0x04 && data[3] == 0x00); // response & status == ok
}

int GetCO2PPM() {
    TinyWireM.beginTransmission(ADDR_6700);
    TinyWireM.write(0x04); TinyWireM.write(0x13); TinyWireM.write(0x8B); TinyWireM.write(0x00); TinyWireM.write(0x01);
    TinyWireM.endTransmission();
    
    delay(measureDelay);
    TinyWireM.requestFrom(ADDR_6700, 4);
    data[0] = TinyWireM.read();
    data[1] = TinyWireM.read();
    data[2] = TinyWireM.read();
    data[3] = TinyWireM.read();

    return ((data[2] & 0x3F ) << 8) | data[3];
}
