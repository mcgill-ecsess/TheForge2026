//
// Created by Oscar Tesniere on 11/02/2026.
//

#ifndef THEFORGE2026_CONTROLLER_H
#define THEFORGE2026_CONTROLLER_H

#include <Arduino.h>
#include <WiFiS3.h>

class Controller {
public:
    // Constructor only stores credentials / initial state (no hardware work)
    Controller(const char* ssid, const char* password);

    // Call in setup() once Serial is ready
    bool beginAP();

    // Call repeatedly in loop() to handle HTTP requests
    void update();

    // Optional: user callback invoked when a message is received (e.g. /control?msg=...)
    void registerCallback(void (*callback)(const String&));

    // Motor command interface (kept simple)
    void setSpeeds(int8_t left, int8_t right);
    int8_t speedLeft() const;
    int8_t speedRight() const;

    struct Button {
        String name;
        void (*callback)() = nullptr;
        bool isPressed = false;
        bool isToggle = false;
    };

private:
    void printWiFiStatus() const;
    void handleClient(WiFiClient& client);
    String readRequestLine(WiFiClient& client);
    void sendHttpOk(WiFiClient& client, const char* contentType, const String& body);

private:
    const char* _ssid;
    const char* _password;

    WiFiServer _server{80};
    int _status = WL_IDLE_STATUS;

    void (*_onMessage)(const String&) = nullptr;

    int8_t _speedLeft = 0;   // -128..127
    int8_t _speedRight = 0;  // -128..127
};

#endif // THEFORGE2026_CONTROLLER_H
