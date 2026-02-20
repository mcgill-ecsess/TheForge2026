This repository will contain the source code for implementing our own controller library. 

the goal is for teams to interface their robots wirelessly through a web interface that can be cutomized. Our goal is to replace the hardware controller that used joysticks (from last year) which were failing

# Requirements

* The user should be able to control a Joystick, some buttons (Toggle / momentary), and a Slider eventually. 
* Each component (Button, Joystick,Slider) should be configurable as add-ons
* The components should be able to update the robot in real time (wirelessly)
* The constructor of the controller should take a SSID and password for the wifi connection to the robot, and perform a check to make sure no conflicts with the current network.
* The Arduino R4's onboard matrix display should be used to display eventual error codes and warnings (which will be duly defined in the documentation)

# Todo 
* Have a look at available libraries to make a web app on arduino R4, and use them to implement the web interface for the controller.
* Look at how to implement error codes and easily display them on the Uno
* Setup an automatized testing environment that's able to access both the R4's Wifi and see the live Serial output. 

The Arduino Uno features the ESP32-S3 module for Wifi and Bluetooth 
* lkibrary for wifi : WifiS3

# Notes 
* To export the library into Arduino IDE Zip the outer `Controller` folder and import it into Sketch > Include Library > Add .ZIP Library
* For the local testing will use the Router IEEEMcGill (password : `password`)
* The arduino R4 should do in order : On board testing of setting up AP mode (Platformio Test unit) then (only in debugging mode) switch to client mode and connect to the WiFi for performing the Python HTTP tests
* Look for how to make global variables that can be edited ? 
* * reference library for WifiS3 can be found at `https://github.com/arduino/ArduinoCore-renesas/tree/main/libraries/WiFiS3`

# Technical Notes for Platformio

* To compile all the code under the `src` folder, the `src/main.cpp` file should be used as the entry point for the code. Run `pio run` to compile the code and `platformio run -t upload` to upload it to the Arduino R4.
* To run the debug configuration run `pio test -e uno_r4_wifi`


WL_IDLE_STATUS       = 0
WL_NO_SSID_AVAIL     = 1
WL_SCAN_COMPLETED    = 2
WL_CONNECTED         = 3
WL_CONNECT_FAILED    = 4
WL_CONNECTION_LOST   = 5
WL_DISCONNECTED      = 6
WL_AP_LISTENING      = 7
WL_AP_CONNECTED      = 8

* The motor control is implemented using failsafe : whenever the joystick is released, the motors will stop. This is done by sending a 0 value to the motors when the joystick is released. The motors will only move when the joystick is being moved, and will stop immediately when the joystick is released. This is a safety feature to prevent the robot from moving uncontrollably if the connection is lost or if the user accidentally releases the joystick.

# Troobleshooting

* If the website is unacessible make sure you're using http not https
* The mobile version doesn't seem to be super reactive