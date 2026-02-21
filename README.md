# Controller Library for Arduino Uno R4 WiFi

A beginner-friendly wireless robot controller library for the **Arduino Uno R4 WiFi**.

This library allows you to control a robot from a phone or computer using a built-in web interface with:

- 🕹️ Joystick control
- 🎚️ Throttle slider
- 🔘 Custom buttons
- 🛑 Built-in failsafe safety
- 💡 Status LED indicators
- 🔧 Optional L298N motor driver support

The goal is to replace fragile hardware joystick controllers with a reliable wireless interface.

---

# Requirements

- Arduino **Uno R4 WiFi**
- L298N motor driver (if using motors)
- DC motors
- Robot battery
- Phone or computer with WiFi

---

# Installation

## Arduino IDE

1. Download the `Controller` folder.
2. Zip the **outer `Controller` folder**.
3. Open Arduino IDE.
4. Go to:

```
Sketch > Include Library > Add .ZIP Library
```

5. Select the zip file.

---

## Include the Library

Add this at the top of your sketch:

```cpp
#include <Controller.h>
```

---

# Basic Usage

## Create the Controller

The constructor requires:

```cpp
Controller controller("SSID", "password");
```

Example:

```cpp
Controller controller("RobotAP", "12345678");
```

This creates a WiFi network named `RobotAP`.

---

## Start the Access Point

In `setup()`:

```cpp
void setup() {
  Serial.begin(115200);
  controller.beginAP(true);   // true = enable debug
}
```

- `true` → enables debug printing
- `false` → disables debug mode

---

## Run the Controller Loop

In `loop()`:

```cpp
void loop() {
  controller.update();
}
```

This is **mandatory**.  
It handles:

- Web requests
- Joystick input
- Motor smoothing
- Failsafe safety

---

# Connecting to the Robot

1. Power the robot.
2. Connect your phone/laptop to the WiFi network you defined.
3. Open a browser.
4. Go to:

```
http://10.0.0.2
```

⚠️ Use **http**, NOT https.

---

# Using the L298N Motor Driver

## Configure the Pins

```cpp
controller.configureL298N(
  9, 7, 6,    // ENA, IN1, IN2
  10, 5, 4    // ENB, IN3, IN4
);
```

---

## Set Minimum PWM (Important)

Many motors need a minimum power level to overcome static friction.

```cpp
controller.setMotorMinPWM(90);
```

Typical values:

| Robot Type | Recommended Value |
|------------|------------------|
| Small robot | 70–90 |
| Medium robot | 90–110 |
| Heavy robot | 110–130 |

If motors buzz but do not move → increase this value.

---

## Full Motor Example

```cpp
#include <Controller.h>

Controller controller("RobotAP", "12345678");

void setup() {
  Serial.begin(115200);

  controller.configureL298N(9,7,6, 10,5,4);
  controller.setMotorMinPWM(90);
  controller.setFailsafeTimeoutMs(1000);

  controller.beginAP(true);
}

void loop() {
  controller.update();
}
```

---

# Failsafe System (Safety Feature)

The failsafe ensures the robot stops if:

- The joystick is released
- The connection drops
- The browser closes
- No drive command is received within a timeout

Set timeout:

```cpp
controller.setFailsafeTimeoutMs(1200);
```

### Recommended Range

- 800–1500 ms

Smaller value → more responsive but can feel jerky  
Larger value → smoother but robot continues longer if connection is lost

---

# Adding Buttons

You can register custom buttons in the web interface.

```cpp
void onPress() {
  Serial.println("Button pressed!");
}

void setup() {
  controller.registerButton("Press Me", onPress);
}
```

Buttons are momentary (trigger once per press).

---

# Custom Drive Callback (Without L298N)

If you want to handle motor logic yourself:

```cpp
void onDrive(int8_t left, int8_t right) {
  Serial.print("Left: ");
  Serial.print(left);
  Serial.print(" Right: ");
  Serial.println(right);
}

void setup() {
  controller.registerDriveCallback(onDrive);
  controller.beginAP(true);
}
```

`left` and `right` range from **-100 to 100**.

---

# Status LED

The onboard LED can show system states.

Enable it:

```cpp
controller.enableStatusLED(LED_BUILTIN);
```

### LED States

| Pattern | Meaning |
|----------|----------|
| Fast blink | Booting |
| Slow blink | AP ready |
| Solid ON | Client connected |
| Rapid blink | Error |
| Double blink | Failsafe active |

---

# Debug Mode

Enable debug:

```cpp
controller.beginAP(true);
```

Debug prints:

- WiFi scan results
- Drive commands
- Motor values

Disable:

```cpp
controller.beginAP(false);
```

---

# Important Power Note

If the robot is powered by battery and you connect USB to your computer:

⚠️ Turn the battery OFF first.

Otherwise, the board may soft-reboot or behave unexpectedly.

---

# Troubleshooting

## Website not loading

- Make sure you're using `http://`
- Confirm you are connected to the robot WiFi
- Try `http://10.0.0.2`

---

## Robot stops after 1 second

Increase failsafe timeout:

```cpp
controller.setFailsafeTimeoutMs(1500);
```

---

## Motors buzz but do not move

Increase minimum PWM:

```cpp
controller.setMotorMinPWM(110);
```

---

## Movement feels jerky

- Increase failsafe timeout slightly
- Reduce minimum PWM
- Check battery voltage
- Ensure good WiFi signal

---

# PlatformIO Usage

Compile:

```
pio run
```

Upload:

```
platformio run -t upload
```

Run tests:

```
pio test -e uno_r4_wifi
```

---

# WiFi Status Codes (Reference)

```
WL_IDLE_STATUS       = 0
WL_NO_SSID_AVAIL     = 1
WL_SCAN_COMPLETED    = 2
WL_CONNECTED         = 3
WL_CONNECT_FAILED    = 4
WL_CONNECTION_LOST   = 5
WL_DISCONNECTED      = 6
WL_AP_LISTENING      = 7
WL_AP_CONNECTED      = 8
```

---

# Design Goals

This library was built to:

- Replace fragile hardware joystick controllers
- Provide a customizable wireless interface
- Ensure safety through built-in failsafe
- Allow teams to easily extend functionality
- Use the Uno R4 WiFi’s ESP32-S3 module

---

Enjoy building 🚀