//
// Created by Oscar Tesniere on 11/02/2026.
//

#ifndef THEFORGE2026_CONTROLLER_H
#define THEFORGE2026_CONTROLLER_H

#include <Arduino.h>

class Controller {
    public:

        Controller(int pin);
        bool begin(String SSID, String password); // the wifi will be uniquely identified by the team. In implementation make sure to check against conflicting SSID on network
        void registerCallback(void (*callback)(String));
        signed char SpeedLeft; // should be a signed char from -128 t0 127
        signed char SpeedRight; // should be a signed char from -128 t0 127

        struct Button{
            String name;
            void (*callback)();
            bool isPressed;
            bool isToggle; // Toggle -> state will simply change from true to false and vice versa every time the button is pressed, Non Toggle -> state will be true when the button is pressed and false when the button is released
        };
        // define a Button that user can use to control the robot, for example a button to stop the robot, a button to change the direction of the robot, etc.
    private:
        int _pin;

}

// The controller should be able to dynamically receive the informations from the web controller to update the speed of the motors. It should also be able to send back informations to the web controller, such as the battery level, the temperature of the motors, etc. This will allow the user to have a better control over the robot and to be able to react quickly in case of any problem.
// How to dynamically link variables to the web controller ? We can use a callback function that will be called every time the web controller sends new informations. This way, we can update the variables in the controller class and use them to drive the motors. We can also use a timer to periodically send back informations to the web controller, such as the battery level, the temperature of the motors, etc. This way, the user will always have up-to-date informations about the robot.

#endif //THEFORGE2026_CONTROLLER_H