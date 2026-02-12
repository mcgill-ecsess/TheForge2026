//
// Created by Oscar Tesniere on 11/02/2026.
//

#include "Controller.h"

Controller::Controller(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {}

void Controller::registerCallback(void (*callback)(const String&)) {
    _onMessage = callback;
}

bool Controller::beginAP() {
    // Firmware check (optional but useful)
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Warning: WiFi firmware may be outdated. Consider upgrading.");
    }

    Serial.print("Starting AP: ");
    Serial.println(_ssid);

    _status = WiFi.beginAP(_ssid, _password);
    if (_status != WL_AP_LISTENING) {
        Serial.println("Failed to start AP mode");
        return false;
    }

    delay(500); // small settle time
    _server.begin();

    Serial.println("AP mode started");
    printWiFiStatus();
    return true;
}

void Controller::update() {
    WiFiClient client = _server.available();
    if (!client) {
        return;
    }

    handleClient(client);

    // Ensure the client is closed cleanly
    delay(1);
    client.stop();
}

void Controller::setSpeeds(int8_t left, int8_t right) {
    _speedLeft = left;
    _speedRight = right;
}

int8_t Controller::speedLeft() const {
    return _speedLeft;
}

int8_t Controller::speedRight() const {
    return _speedRight;
}

void Controller::printWiFiStatus() const {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    Serial.print("To control: http://");
    Serial.print(ip);
    Serial.println("/");
}

String Controller::readRequestLine(WiFiClient& client) {
    // Read the first line: "GET /path?query HTTP/1.1"
    String line = client.readStringUntil('\n');
    line.trim();
    return line;
}

void Controller::sendHttpOk(WiFiClient& client, const char* contentType, const String& body) {
    client.println("HTTP/1.1 200 OK");
    client.print("Content-Type: ");
    client.println(contentType);
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println();
    client.print(body);
}

void Controller::handleClient(WiFiClient& client) {
    // Basic HTTP: read request line + ignore headers
    String requestLine = readRequestLine(client);

    // Drain headers
    while (client.connected()) {
        String h = client.readStringUntil('\n');
        if (h == "\r" || h.length() == 0) break;
    }

    // Very small router:
    // 1) GET / -> simple page
    // 2) GET /control?msg=... -> call callback with msg
    // You can replace this later with JSON POST, etc.

    if (requestLine.startsWith("GET / ")) {
        String page =
            "<!doctype html><html><head><meta charset='utf-8'/>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'/>"
            "<title>Robot Controller</title></head><body>"
            "<h1>Robot Controller</h1>"
            "<p>Try: <code>/control?msg=hello</code></p>"
            "</body></html>";

        sendHttpOk(client, "text/html; charset=utf-8", page);
        return;
    }

    if (requestLine.startsWith("GET /control?msg=")) {
        // Extract msg up to space (before HTTP/1.1)
        int start = String("GET /control?msg=").length();
        int end = requestLine.indexOf(' ', start);
        String msg = (end > start) ? requestLine.substring(start, end) : "";

        // Minimal URL decode for + (space). For full decoding, implement %xx decoding.
        msg.replace("+", " ");

        if (_onMessage) {
            _onMessage(msg);
        }

        sendHttpOk(client, "text/plain; charset=utf-8", "OK");
        return;
    }

    if (requestLine.startsWith("GET /health ")) {
        sendHttpOk(client, "text/plain; charset=utf-8", "OK");
        return;
    }

    // Default 404
    client.println("HTTP/1.1 404 Not Found");
    client.println("Connection: close");
    client.println();
    client.println("Not Found");
}
