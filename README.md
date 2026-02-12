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
* 
