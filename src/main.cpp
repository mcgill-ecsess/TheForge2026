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

int servoAngle = 90;

void onServoSlider(int value) {
    servoAngle = value;
    Serial.print("Servo angle set to: ");
    Serial.println(servoAngle);

    // Example:
    // servo.write(value);
}

void onPress() {
    Serial.println("Button pressed!");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    // Move most L298N setup into the library:
    ctrl.configureL298N(ENA, IN1, IN2, ENB, IN3, IN4);

    ctrl.registerButton("Press ME", onPress);

    ctrl.registerButton("Another Button", onPress);

    ctrl.enableStatusLED();

    // Optional: tune print throttle (default = 150ms)
    // ctrl.setMotorDebugPrintIntervalMs(150);

    ctrl.setFailsafeTimeoutMs(700);
    ctrl.setMotorMinPWM(150);

    // Debug option is now an optional parameter in init:
    //   - true  => prints [MOTOR] debug lines
    //   - false => silent
    ctrl.beginAP(true);

    Serial.println("Ready. Open the controller page and move joystick.");

	ctrl.registerSlider("Servo Angle", onServoSlider, 0, 180, 90, 1);
}

void loop() {
    ctrl.update();
}