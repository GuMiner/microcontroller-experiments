#define SENSOR_NAME "T6703 "
#define SENSOR_UNITS " ppm"

// Pin connections from the flat side: 1 (SDA), 2 (SCL), 3 (5V), 4 (GND), 5 (NC), 6 (GND - I2C mode)
// Modified somewhat from https://github.com/AmphenolAdvancedSensors/Telaire
unsigned long GetSerialNumber();
unsigned int GetFirmwareVersion();
bool IsDeviceHealthy();

int GetCO2PPM();