//
// Created by Oscar Tesniere on 11/02/2026.
//

#ifndef THEFORGE2026_CONTROLLER_H
#define THEFORGE2026_CONTROLLER_H

#include <Arduino.h>
#include <WiFiS3.h>

class Controller {
public:
    Controller(const char* ssid, const char* password);

    bool beginAP();
    void update();

    // Generic message callback (optional)
    void registerCallback(void (*callback)(const String&));

    // Called whenever smoothed motor outputs change
    void registerDriveCallback(void (*callback)(int8_t left, int8_t right));

    // Smoothed motor outputs (-100..100)
    int8_t speedLeft() const;
    int8_t speedRight() const;

    void setFailsafeTimeoutMs(uint16_t ms);

    // Register a button shown on the UI; callback called on press
    bool registerButton(const char* label, void (*cb)());
    void clearButtons();

private:
    void printWiFiStatus() const;

    void handleClient(WiFiClient& client);
    String readRequestLine(WiFiClient& client);

    void sendHttpOk(WiFiClient& client, const char* contentType, const String& body);
    void sendHttpNotFound(WiFiClient& client);

    void handleRoot(WiFiClient& client);
    void handleDrive(WiFiClient& client, const String& requestLine);
    void handleBtn(WiFiClient& client, const String& requestLine);
    void handleControlMsg(WiFiClient& client, const String& requestLine);
    void handleHealth(WiFiClient& client);

    static bool extractQueryInt(const String& requestLine, const char* key, int& outValue);
    static int clampInt(int v, int lo, int hi);

    void applySmoothingAndNotify();

private:
    const char* _ssid;
    const char* _password;

    WiFiServer _server{80};
    int _status = WL_IDLE_STATUS;

    void (*_onMessage)(const String&) = nullptr;
    void (*_onDrive)(int8_t left, int8_t right) = nullptr;

    // Network target (set by /drive)
    int8_t _cmdLeft  = 0;
    int8_t _cmdRight = 0;

    // Smoothed output (what you apply to motors)
    int8_t _outLeft  = 0;
    int8_t _outRight = 0;

    // Params for smoothing
    uint8_t _deadband = 6;        // +/-6 => treat as 0
    uint8_t _slewPerUpdate = 8;   // max change per update() call

    // Failsafe
    uint16_t _failsafeTimeoutMs = 1200;
    uint8_t _slewPerUpdateStop = 30;  // faster ramp-down
    unsigned long _lastDriveMs = 0;
    bool _failsafeStopped = false;

    // Button registry
    static constexpr uint8_t MAX_BUTTONS = 8;

    struct ButtonReg {
        String label;
        void (*cb)() = nullptr;
    };

    ButtonReg _buttons[MAX_BUTTONS];
    uint8_t _buttonCount = 0;
};

#endif // THEFORGE2026_CONTROLLER_H
