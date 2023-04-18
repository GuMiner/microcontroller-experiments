#include <SPI.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(); // Default 0x40 address

https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library/blob/master/examples/servo/servo.ino
// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  600 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2400 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

void setup() {
  //https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library/blob/master/examples/gpiotest/gpiotest.ino
  pwm.begin(); // Setup Motor 

  pwm.setPWMFreq(SERVO_FREQ);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}


void flashLED() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}

void loop() {
  // Flash the LED for a half-second after each measurement update
  flashLED();
  for (uint8_t servonum = 3; servonum <= 13; servonum++) {
  for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
    pwm.setPWM(servonum, 0, pulselen);
    delay(1);
  }

  for (uint16_t pulselen = SERVOMAX; pulselen >= SERVOMIN; pulselen--) {
    pwm.setPWM(servonum, 0, pulselen);
    delay(1);
  }
  }
}
