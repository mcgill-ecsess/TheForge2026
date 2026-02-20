#include <Arduino.h>
#include "Controller.h"

// ---- L298N pins (change to your wiring) ----
const int ENA = 9;   // PWM
const int IN1 = 7;
const int IN2 = 6;

const int ENB = 10;  // PWM
const int IN3 = 5;
const int IN4 = 4;

Controller ctrl("RobotAP", "12345678");

// Debug controls
static const bool DEBUG_MOTORS = true;
static const uint16_t DEBUG_PRINT_MS = 150; // max print rate

static int8_t lastL = 127;   // impossible initial values to force first print
static int8_t lastR = 127;
static unsigned long lastPrintMs = 0;

// Convert speed -> direction + pwm (no IO side effects)
static void speedToCmd(int8_t spd, bool &forward, uint8_t &pwm) {
  int s = spd; // -100..100
  if (s >= 0) {
    forward = true;
  } else {
    forward = false;
    s = -s;
  }
  s = constrain(s, 0, 100);
  pwm = (uint8_t)map(s, 0, 100, 0, 255);
}

static void setMotor(int en, int inA, int inB, int8_t spd) {
  int s = spd;
  if (s > 0) {
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);
  } else if (s < 0) {
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);
    s = -s;
  } else {
    // BRAKE (stops faster than coast)
    digitalWrite(inA, HIGH);
    digitalWrite(inB, HIGH);
    analogWrite(en, 0);
    return;
  }


  int pwm = map(constrain(s, 0, 100), 0, 100, 0, 255);
  analogWrite(en, pwm);
}

static void debugMotors(int8_t left, int8_t right) {
  if (!DEBUG_MOTORS) return;

  const unsigned long now = millis();
  const bool changed = (left != lastL) || (right != lastR);
  const bool timeOk = (now - lastPrintMs) >= DEBUG_PRINT_MS;

  if (!changed && !timeOk) return;

  bool lfwd, rfwd;
  uint8_t lpwm, rpwm;
  speedToCmd(left, lfwd, lpwm);
  speedToCmd(right, rfwd, rpwm);

  Serial.print("[MOTOR] L=");
  Serial.print(left);
  Serial.print(lfwd ? " FWD " : " REV ");
  Serial.print("PWM=");
  Serial.print(lpwm);

  Serial.print(" | R=");
  Serial.print(right);
  Serial.print(rfwd ? " FWD " : " REV ");
  Serial.print("PWM=");
  Serial.println(rpwm);

  lastL = left;
  lastR = right;
  lastPrintMs = now;
}

void onDrive(int8_t left, int8_t right) {
  debugMotors(left, right);
  setMotor(ENA, IN1, IN2, left);
  setMotor(ENB, IN3, IN4, right);
}

void onPress() {
  Serial.println("Button pressed!");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  // Ensure stopped at boot (BRAKE)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 0);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 0);

  ctrl.registerButton("Press ME", onPress);
  ctrl.registerDriveCallback(onDrive);
  ctrl.setFailsafeTimeoutMs(1200);
  ctrl.beginAP();

  Serial.println("Ready. Open the controller page and move joystick.");
}


void loop() {
  ctrl.update();
}
