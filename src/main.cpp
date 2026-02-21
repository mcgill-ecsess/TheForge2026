#include <Arduino.h>
#include "Controller.h"

// ---- L298N pins (change to your wiring) ----
const uint8_t ENA = 9;   // PWM
const uint8_t IN1 = 7;
const uint8_t IN2 = 6;

const uint8_t ENB = 10;  // PWM
const uint8_t IN3 = 5;
const uint8_t IN4 = 4;
Controller ctrl("RobotAP", "12345678");

void onPress() {
  Serial.println("Button pressed!");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // Move most L298N setup into the library:
  ctrl.configureL298N(ENA, IN1, IN2, ENB, IN3, IN4);

  ctrl.registerButton("Press ME", onPress);

  ctrl.enableStatusLED();

  // Optional: tune print throttle (default = 150ms)
  // ctrl.setMotorDebugPrintIntervalMs(150);

  ctrl.setFailsafeTimeoutMs(700);

  // Debug option is now an optional parameter in init:
  //   - true  => prints [MOTOR] debug lines
  //   - false => silent
  ctrl.beginAP(true);

  Serial.println("Ready. Open the controller page and move joystick.");
}

void loop() {
  ctrl.update();
}