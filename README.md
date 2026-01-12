# microcontroller-experiments

Experiments with microcontrollers (Arduino / ATTiny devices) that aren't large enough to deserve their own repository

## Sensor

Charts CO2 readings from an Amphenol CO2 sensor to a small OLED display.

This sensor (T6703) was pulled out of an abandonware Awair 2nd generation air quality monitor. Both the sensor and display (SSD1306) are directly hooked up via I2C, greatly simplifying wiring.

## Mover

Tests 10 small servos connected via I2C with a PCA9682 16-channel PWM driver

## MusicPlayer

Plays music stored on a microSD card using an Arduino (control), VS1053 (audio decoder/player), PN532 (NFC card reader to determine what song to play), and MAX98306 (audio amplifier).

