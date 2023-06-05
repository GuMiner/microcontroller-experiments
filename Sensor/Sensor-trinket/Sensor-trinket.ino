#include <TinyWireM.h>
#include <Tiny4kOLED.h>
#include "CDSensor.h"

// Do not exceed 5258 or higher program usage, even though 5310 is supposedly supported.
// This will overwrite the bootloader, causing the program to get stuck and not be modified after-the-fact.
// Active question: Do the 105 bytes of dynamic memory overwrite the bootloader?
// 5234 (98% usage) bytew with 105 bytes of dynamic memory appear fine.
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Run Tiny4k zoomed in, for a 128x64 equivalent
  oled.begin(128, 64, sizeof(tiny4koled_init_128x64), tiny4koled_init_128x64);
  oled.enableChargePump(); 
  oled.enableZoomIn();

  oled.setFont(FONT6X8);

  // Clear the memory before turning on the display
  oled.clear();

  sendArtisticBuffer();

  // Turn on the display
  oled.on();
  oled.switchRenderFrame();

  sendArtisticBuffer();
}

void sendArtisticBuffer() {
  oled.setCursor(0, 1);
  oled.startData();
  for (uint16_t x = 0; x < 512; x++) {
    uint16_t y = x;
    y &= 0b01111110;
    oled.sendData(y);
  }
  oled.endData();
}

void loop() {
  drawMeasurement();
  drawUptime();
  drawSerial();

  oled.switchFrame();
  flashLED();
}

void drawMeasurement() {
  oled.setCursor(25, 0);
  oled.print(F("CO2: "));

  int measurement = GetCO2PPM();
  oled.print(measurement);
  oled.print(F(" ppm"));
}

void drawUptime() {
  oled.setCursor(0, 2);
  oled.print(F("uptime: "));

  int sUptime = (int)((float)millis() / 1000);
  int mUptime = sUptime / 60;
  if (mUptime > 0) {
    oled.print(mUptime);
    oled.print(F(" m "));
  }

  oled.print(sUptime - (mUptime * 60));
  oled.print(F(" s"));
}

void drawSerial() {
  oled.setCursor(0, 3);
  oled.print(F("Sensor #: "));

  unsigned long serialNumber = GetSerialNumber();
  oled.print(serialNumber);
}

void flashLED() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}