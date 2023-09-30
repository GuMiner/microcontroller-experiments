#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPL3115A2.h>

#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>


#include "CDSensor.h"
#include "NetworkConfig.h"


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// On an arduino UNO:       A4(SDA), A5(SCL)
#define OLED_RESET     -1 // No reset pin
#define SCREEN_ADDRESS 0x3C // For DIYMall's 128x64 display

// Display: SSD1306 monocolor, 0.96-inch display with 128×64 pixels 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MPL3115A2 baro;

// Timing for saving the data. Saves every 10 minutes -- 144 measurements / day.
const unsigned long DelayIntervalMs = 5000; // 5 seconds
const unsigned long DelayIntervalsBeforeReporting = (10 * 60 * 1000) / DelayIntervalMs;
unsigned long delayIntervalsRemaining = 0;

float lastPressure;
float lastAltitude;
float lastTemperature;
int lastCo2;

void setup() {
  Wire.begin(); // Setup display / sensor I2C
  Serial.begin(9600); // For diagnostics

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.display();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text (B/W display)
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  baro.begin();
  baro.setSeaPressure(1013.26);

  // Draw first-time-only device info
  drawMeasurementBounds();
}

void loop() {
  display.fillRect(0, 0, SCREEN_WIDTH, 11 + 24, SSD1306_BLACK);

  display.setCursor(0, 0);
  drawDeviceInfo();
  drawMeasurement();

  // Flash the LED for a half-second after each measurement update
  flashLED(500);

  // Connect to network and save measurement
  if (delayIntervalsRemaining == 0)
  {
    Serial.println("Uploading to network...");
    enableWifi();

    // Compute JSON document of measurements and upload.
    StaticJsonDocument<200> doc;
    doc["pressureHpa"] = lastPressure;
    doc["altitudeFt"] = lastAltitude;
    doc["temperatureC"] = lastTemperature;
    doc["co2Ppm"] = lastCo2;
    
    String json;
    serializeJson(doc, json);
    serializeJson(doc, Serial);

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, SAVE_API);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(json);
    Serial.println(httpCode);

    http.end();

    delayIntervalsRemaining = DelayIntervalsBeforeReporting;
    disableWiFi();
  }
  else
  {
    delayIntervalsRemaining--;
    Serial.println(delayIntervalsRemaining);
  }

  delay(DelayIntervalMs);
}

void enableWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, SSID_PASSWORD);

    // Wait for network connection
    while (WiFi.status() != WL_CONNECTED) {
      flashLED(250);
    }
}

void disableWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

int lastX = 0;
int currentX = 0;
int nextX = 1;
int lastY = -1;

#define CHART_START 35

void drawMeasurementBounds() {
  display.drawLine(0, CHART_START, SCREEN_WIDTH - 1, CHART_START, SSD1306_WHITE);
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
}

void drawMeasurement() {
  // Text measurement. Ensure text roughly follows the current chart
  int displayOffset = currentX;
  if (displayOffset > (SCREEN_WIDTH * 3 / 4)) {
    displayOffset = SCREEN_WIDTH * 3 / 4;
  }
  display.setCursor(displayOffset, 24);

  lastCo2 = GetCO2PPM();
  display.print(lastCo2);
  display.println(F(SENSOR_UNITS));
  display.display();

  // Chart measurement: Because 36-64 is not erased, persist pixels in there for charting/
  // Measurement values are from ~400 to 2000. Squash that 16,000 range into the 27 pixels available
  // (apx 60 ppm per pixel change). 
  int adjustedMeasurement = (SCREEN_HEIGHT - ((lastCo2 / 60) - 6)); // Approximately now from 0 to 27
  adjustedMeasurement = fitToRange(adjustedMeasurement, CHART_START + 1, SCREEN_HEIGHT - 2);
  if (lastY == -1) {
    lastY = adjustedMeasurement; // First-time fix so that point-to-point lines start fine
  }

  display.drawLine(currentX, CHART_START + 1, currentX, SCREEN_HEIGHT - 2, SSD1306_BLACK); // Wipe line of old measurement
  display.drawLine(lastX, lastY, currentX, adjustedMeasurement, SSD1306_WHITE);

  // Render current chart position
  if (nextX != SCREEN_WIDTH) {
    display.drawLine(nextX, CHART_START + 1, nextX, SCREEN_HEIGHT - 2, SSD1306_WHITE);
  }

  lastY = adjustedMeasurement;

  // Scroll chart
  lastX = currentX;
  currentX += 1;
  nextX += 1;
  if (currentX == SCREEN_WIDTH) {
    lastX = 0;
    currentX = 0;
    nextX = 1;
  }
}

int fitToRange(int value, int min, int max)
{
  if (value < min) {
    value = min;
  } else if (value > max) {
    value = max;
  }

  return value;
}

void drawDeviceInfo() {
  unsigned long serialNumber = GetSerialNumber();
  unsigned int firmwareVersion = GetFirmwareVersion();
  bool isDeviceHealthy = IsDeviceHealthy();

  lastPressure = baro.getPressure();
  lastAltitude = baro.getAltitude()/3.2808;
  lastTemperature = ((baro.getTemperature() * 9.0f) / 5.0f) + 32.0f;

  display.print(F(SENSOR_NAME));
  display.print(isDeviceHealthy ? "+ " : "- ");
  display.print(lastPressure);
  display.println(" hPa");

  display.print(F("S: "));
  display.print(serialNumber);
  display.print(" ");
  display.printf("%.1f", lastAltitude);
  display.println(" ft");

  display.print(F("F: "));
  display.print(firmwareVersion);
  display.print(" ");
  display.printf("%.1f", lastTemperature);
  display.println(" F");
}

void flashLED(unsigned long delayMs) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delayMs);
  digitalWrite(LED_BUILTIN, LOW);
  delay(delayMs);
}
