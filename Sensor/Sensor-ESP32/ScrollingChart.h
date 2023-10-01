#pragma once
#include <Adafruit_SSD1306.h>

class ScrollingChart
{
  int16_t screenWidth;
  int16_t screenHeight;
  int16_t chartStart;

  int lastX;
  int currentX;
  int lastY;

  void fitToRange(int *value, int min, int max);

public:
  ScrollingChart(int16_t screenWidth, int16_t screenHeight, int16_t chartStart);
  void init(Adafruit_SSD1306 *display);
  void renderMeasurement(Adafruit_SSD1306 *display, int measurement);
  int getChartPosition();
};