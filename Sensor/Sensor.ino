#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "CDSensor.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// On an arduino UNO:       A4(SDA), A5(SCL)
#define OLED_RESET     -1 // No reset pin
#define SCREEN_ADDRESS 0x3C // For DIYMall's 128x64 display

// Display: SSD1306 monocolor, 0.96-inch display with 128×64 pixels 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Wire.begin(); // Setup display / sensor I2C

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.display();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text (B/W display)
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Draw first-time-only device info
  drawDeviceInfo();
  drawMeasurementBounds();
}

void loop() {
  drawMeasurement();

  // Flash the LED for a half-second after each measurement update
  flashLED();

  delay(2000);
}

int currentX = 0;

#define CHART_START 35

void drawMeasurementBounds() {
  display.drawLine(0, CHART_START, SCREEN_WIDTH - 1, CHART_START, SSD1306_WHITE);
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
}

void drawMeasurement() {
  // Text measurement
  display.fillRect(0, 24, SCREEN_WIDTH, 11, SSD1306_BLACK);
  display.setCursor(currentX % (SCREEN_WIDTH * 3 / 4), 24);

  int measurement = GetCO2PPM();
  display.print(measurement);
  display.println(F(SENSOR_UNITS));
  display.display();

  // Chart measurement: Because 36-64 is not erased, persist pixels in there for charting/
  // Measurement values are from ~400 to 2000. Squash that 16,000 range into the 27 pixels available
  // (apx 60 ppm per pixel change). 
  int adjustedMeasurement = (measurement / 60) - 6; // Approximately now from 0 to 27
  display.drawLine(currentX, CHART_START + 1, currentX, SCREEN_HEIGHT - 2, SSD1306_BLACK); // Wipe line of old measurement
  display.drawPixel(currentX, SCREEN_HEIGHT - adjustedMeasurement, SSD1306_WHITE);

  // Scroll chart
  currentX += 1;
  if (currentX == SCREEN_WIDTH) {
    currentX = 0;
  }
}

void drawDeviceInfo() {
  unsigned long serialNumber = GetSerialNumber();
  unsigned int firmwareVersion = GetFirmwareVersion();
  bool isDeviceHealthy = IsDeviceHealthy();

  display.print(F(SENSOR_NAME));
  display.println(isDeviceHealthy ? "Online" : "OFFLINE");

  display.print(F("Serial: "));
  display.println(serialNumber);

  display.print(F("Firmware: "));
  display.println(firmwareVersion);
}

void flashLED() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
