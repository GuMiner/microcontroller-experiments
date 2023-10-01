#include "ScrollingChart.h"

// Typical CO2 levels
#define CHART_MIN 400
#define CHART_MAX 2000

ScrollingChart::ScrollingChart(int16_t screenWidth, int16_t screenHeight, int16_t chartStart)
  : screenWidth(screenWidth), screenHeight(screenHeight), chartStart(chartStart),
    lastX(0), currentX(0), lastY(-1)
{}

void ScrollingChart::init(Adafruit_SSD1306 *display)
{
  display->drawLine(0, chartStart, screenWidth - 1, chartStart, SSD1306_WHITE);
  display->drawLine(0, screenHeight - 1, screenWidth - 1, screenHeight - 1, SSD1306_WHITE);
}

void ScrollingChart::fitToRange(int *value, int min, int max)
{
  if (*value < min) {
    *value = min;
  } else if (*value > max) {
    *value = max;
  }
}

int ScrollingChart::getChartPosition()
{
  return currentX;
}

void ScrollingChart::renderMeasurement(Adafruit_SSD1306 *display, int measurement)
{
  // Chart measurement: Because 36-64 is not erased, persist pixels in there for charting
  // Measurement values are from ~400 to 2000. Squash that range into the 27 pixels available
  // (apx 60 ppm per pixel change).
  int divisionFactor = (CHART_MAX - CHART_MIN) / 27;
  int offsetFactor = CHART_MIN / divisionFactor;

  // Even after adjustment, ensure that the measurement cannot escape its bounds
  int adjustedMeasurement = (screenHeight - ((measurement / divisionFactor) - offsetFactor));
  fitToRange(&adjustedMeasurement, chartStart + 1, screenHeight - 2);

  // First-time fix so that point-to-point lines start fine
  if (lastY == -1) {
    lastY = adjustedMeasurement;
  }

  // Wipe old chart position vertical line
  display->drawLine(currentX, chartStart + 1, currentX, screenHeight - 2, SSD1306_BLACK);

  // Render connecting line
  display->drawLine(lastX, lastY, currentX, adjustedMeasurement, SSD1306_WHITE);

  // Render current chart position vertical line
  currentX++;
  if (currentX != screenWidth) {
    display->drawLine(currentX, chartStart + 1, currentX, screenHeight - 2, SSD1306_WHITE);
  }

  lastY = adjustedMeasurement;

  // Scroll chart, resetting if necessary
  lastX = currentX;
  if (currentX == screenWidth) {
    lastX = 0;
    currentX = 0;
  }
}