//
// Created by Oscar Tesniere on 11/02/2026.
//

#include "Controller.h"

Controller::Controller(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {}

bool Controller::registerButton(const char* label, void (*cb)()) {
    if (_buttonCount >= MAX_BUTTONS) return false;
    _buttons[_buttonCount].label = label;
    _buttons[_buttonCount].cb = cb;
    _buttonCount++;
    return true;
}

void Controller::clearButtons() {
    _buttonCount = 0;
}

void Controller::registerCallback(void (*callback)(const String&)) {
    _onMessage = callback;
}

void Controller::registerDriveCallback(void (*callback)(int8_t left, int8_t right)) {
    _onDrive = callback;
}

void Controller::setFailsafeTimeoutMs(uint16_t ms) {
    _failsafeTimeoutMs = ms;
}

bool Controller::beginAP() {
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Warning: WiFi firmware may be outdated. Consider upgrading.");
    }

    Serial.print("Starting AP: ");
    Serial.println(_ssid);

    WiFi.config(IPAddress(10, 0, 0, 2));

    _status = WiFi.beginAP(_ssid, _password);

    if (_status != WL_AP_LISTENING && _status != WL_AP_CONNECTED) {
        Serial.println("Failed to start AP mode");
        return false;
    }

    delay(2000);
    _server.begin();

    _lastDriveMs = millis();
    _failsafeStopped = false;

    Serial.println("AP mode started");
    printWiFiStatus();
    return true;
}

void Controller::update() {
    // Handle ONE incoming client per loop; keep loop fast
    WiFiClient client = _server.available();
    if (client) {
        client.setTimeout(30);
        handleClient(client);
        delay(1);
        client.stop();
    }

    // Failsafe check
    const unsigned long now = millis();
    if (_failsafeTimeoutMs > 0 && (now - _lastDriveMs) > _failsafeTimeoutMs) {
        _failsafeStopped = true;
    }

    // Apply smoothing and notify motors (also handles failsafe)
    applySmoothingAndNotify();
}

void Controller::applySmoothingAndNotify() {
    auto applyDeadband = [&](int8_t v) -> int8_t {
        if (abs((int)v) < (int)_deadband) return 0;
        return v;
    };

    int8_t targetL = _failsafeStopped ? 0 : applyDeadband(_cmdLeft);
    int8_t targetR = _failsafeStopped ? 0 : applyDeadband(_cmdRight);

    auto stepToward = [&](int8_t cur, int8_t tgt) -> int8_t {
    int d = (int)tgt - (int)cur;

    // Use a bigger step when we are braking toward zero
    int step = (tgt == 0) ? (int)_slewPerUpdateStop : (int)_slewPerUpdate;

    if (d > step) d = step;
    if (d < -step) d = -step;
    return (int8_t)((int)cur + d);
};

    int8_t newL = stepToward(_outLeft, targetL);
    int8_t newR = stepToward(_outRight, targetR);

    if (newL == _outLeft && newR == _outRight) return;

    _outLeft = newL;
    _outRight = newR;

    if (_onDrive) {
        _onDrive(_outLeft, _outRight);
    }
}

int8_t Controller::speedLeft() const {
    return _outLeft;
}

int8_t Controller::speedRight() const {
    return _outRight;
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
    unsigned long start = millis();
    while (client.connected() && !client.available()) {
        if (millis() - start > 30) return "";
        delay(1);
    }
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

void Controller::sendHttpNotFound(WiFiClient& client) {
    const String body = "Not Found";
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain; charset=utf-8");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println();
    client.print(body);
}

int Controller::clampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

bool Controller::extractQueryInt(const String& requestLine, const char* key, int& outValue) {
    int q = requestLine.indexOf('?');
    if (q < 0) return false;

    int end = requestLine.indexOf(' ', q);
    if (end < 0) return false;

    String query = requestLine.substring(q + 1, end);

    String k = String(key) + "=";
    int pos = query.indexOf(k);
    if (pos < 0) return false;

    int valStart = pos + k.length();
    int amp = query.indexOf('&', valStart);
    String valStr = (amp >= 0) ? query.substring(valStart, amp) : query.substring(valStart);

    valStr.replace("+", " ");
    outValue = valStr.toInt();
    return true;
}

void Controller::handleClient(WiFiClient& client) {
    String requestLine = readRequestLine(client);
    if (requestLine.length() == 0) return;

    // Drain headers
    while (client.connected()) {
        String h = client.readStringUntil('\n');
        if (h == "\r" || h.length() == 0) break;
    }

    if (requestLine.startsWith("GET / ") || requestLine.startsWith("GET /?")) {
        handleRoot(client);
        return;
    }

    if (requestLine.startsWith("GET /drive")) {
        handleDrive(client, requestLine);
        return;
    }

    if (requestLine.startsWith("GET /btn?")) {
        handleBtn(client, requestLine);
        return;
    }

    if (requestLine.startsWith("GET /control?msg=")) {
        handleControlMsg(client, requestLine);
        return;
    }

    if (requestLine.startsWith("GET /health ")) {
        handleHealth(client);
        return;
    }

    sendHttpNotFound(client);
}

void Controller::handleHealth(WiFiClient& client) {
    sendHttpOk(client, "text/plain; charset=utf-8", "OK");
}

void Controller::handleControlMsg(WiFiClient& client, const String& requestLine) {
    int start = String("GET /control?msg=").length();
    int end = requestLine.indexOf(' ', start);
    String msg = (end > start) ? requestLine.substring(start, end) : "";
    msg.replace("+", " ");

    if (_onMessage) _onMessage(msg);

    sendHttpOk(client, "text/plain; charset=utf-8", "OK");
}

void Controller::handleBtn(WiFiClient& client, const String& requestLine) {
    int id = -1;
    if (!extractQueryInt(requestLine, "id", id)) {
        sendHttpOk(client, "text/plain; charset=utf-8", "Missing id");
        return;
    }

    if (id < 0 || id >= (int)_buttonCount) {
        sendHttpOk(client, "text/plain; charset=utf-8", "Bad id");
        return;
    }

    if (_buttons[id].cb) _buttons[id].cb();

    if (_onMessage) _onMessage(String("btn:") + _buttons[id].label);

    sendHttpOk(client, "text/plain; charset=utf-8", "OK");
}

void Controller::handleDrive(WiFiClient& client, const String& requestLine) {
    int x = 0;   // -100..100
    int y = 0;   // -100..100
    int t = 100; // 0..100

    extractQueryInt(requestLine, "x", x);
    extractQueryInt(requestLine, "y", y);
    extractQueryInt(requestLine, "t", t);

    x = clampInt(x, -100, 100);
    y = clampInt(y, -100, 100);
    t = clampInt(t, 0, 100);

    int left  = clampInt(y + x, -100, 100);
    int right = clampInt(y - x, -100, 100);

    left  = (left * t) / 100;
    right = (right * t) / 100;

    _cmdLeft = (int8_t)left;
    _cmdRight = (int8_t)right;

    _lastDriveMs = millis();
    _failsafeStopped = false;

    sendHttpOk(client, "text/plain; charset=utf-8", "OK");
}

void Controller::handleRoot(WiFiClient& client) {
    String buttonsHtml;
    for (uint8_t i = 0; i < _buttonCount; i++) {
        buttonsHtml += "<button class='uBtn' data-id='";
        buttonsHtml += i;
        buttonsHtml += "'>";
        buttonsHtml += _buttons[i].label;
        buttonsHtml += "</button> ";
    }
    if (_buttonCount == 0) {
        buttonsHtml = "<div style='opacity:.7'>No buttons registered</div>";
    }

    String page;
    page.reserve(6500);

    page += "<!doctype html><html><head><meta charset='utf-8'/>";
    page += "<meta name='viewport' content='width=device-width,initial-scale=1'/>";
    page += "<title>Robot Controller</title>";
    page += "<style>";
    page += "body{font-family:system-ui,Arial;margin:16px;}";
    page += "#wrap{max-width:520px;margin:0 auto;}";
    page += ".row{margin:14px 0;}";
    page += "button{padding:12px 16px;font-size:16px;border-radius:12px;border:1px solid #333;background:#f2f2f2;}";
    page += ".uBtn{margin:6px 8px 6px 0;}";
    page += "#joy{width:260px;height:260px;border:2px solid #333;border-radius:18px;";
    page += "touch-action:none; position:relative; user-select:none; -webkit-user-select:none;}";
    page += "#stick{width:70px;height:70px;border-radius:50%;background:#333;opacity:.85;";
    page += "position:absolute;left:95px;top:95px;}";
    page += "label{display:block;margin-bottom:6px;}";
    page += "input[type=range]{width:100%;}";
    page += "#status{font-family:ui-monospace,Menlo,monospace; white-space:pre;}";
    page += "</style></head><body><div id='wrap'>";
    page += "<h2>Robot Controller</h2>";

    page += "<div class='row' id='buttons'>";
    page += buttonsHtml;
    page += "</div>";

    page += "<div class='row'><div id='joy'><div id='stick'></div></div></div>";

    page += "<div class='row'>";
    page += "<label>Throttle: <span id='tval'>100</span>%</label>";
    page += "<input id='thr' type='range' min='0' max='100' value='100'/>";
    page += "</div>";

    page += "<div class='row'><div id='status'>loading...</div></div>";

     // --- JS (updated: "one request in flight", send-on-change, no backlog) ---
    // --- JS (updated: STOP priority even if a request is in-flight) ---
    page += "<script>";
    page += "let x=0,y=0,t=100;";
    page += "const joy=document.getElementById('joy');";
    page += "const stick=document.getElementById('stick');";
    page += "const thr=document.getElementById('thr');";
    page += "const tval=document.getElementById('tval');";
    page += "const status=document.getElementById('status');";

    page += "function clamp(v,a,b){return Math.max(a,Math.min(b,v));}";
    page += "function setStick(px,py){stick.style.left=(px-35)+'px'; stick.style.top=(py-35)+'px';}";
    page += "function updateStatus(extra=''){status.textContent=`x=${x} y=${y} t=${t}` + (extra?('\\n'+extra):'');}";

    // Dynamic buttons
    page += "document.querySelectorAll('.uBtn').forEach(b=>{";
    page += "  b.addEventListener('click',()=>{";
    page += "    const id=b.getAttribute('data-id');";
    page += "    fetch(`/btn?id=${id}&_=${Date.now()}`, {cache:'no-store'}).catch(()=>{});";
    page += "    updateStatus('btn id=' + id);";
    page += "  });";
    page += "});";

    // --- Drive send logic: 1 in-flight, BUT stop (x=0,y=0) has priority ---
    page += "let inFlight=false;";
    page += "let pending=false;";
    page += "let lastSentX=999,lastSentY=999,lastSentT=999;";

    page += "function sendDriveNow(){";
    page += "  if (x===lastSentX && y===lastSentY && t===lastSentT) return;";
    page += "  const isStop = (x===0 && y===0);";
    page += "  if (inFlight && !isStop){ pending=true; return; }";
    page += "  if (!isStop){ inFlight=true; pending=false; }";
    page += "  const url=`/drive?x=${x}&y=${y}&t=${t}&_=${Date.now()}`;";
    page += "  fetch(url,{cache:'no-store', keepalive:true})";
    page += "    .catch(()=>{})";
    page += "    .finally(()=>{";
    page += "      lastSentX=x; lastSentY=y; lastSentT=t;";
    page += "      if (!isStop){";
    page += "        inFlight=false;";
    page += "        if (pending) sendDriveNow();";
    page += "      }";
    page += "    });";
    page += "}";

    // Joystick mapping
    page += "function posToXY(clientX,clientY){";
    page += "  const r=joy.getBoundingClientRect();";
    page += "  const cx=clientX - r.left;";
    page += "  const cy=clientY - r.top;";
    page += "  const dx=cx - r.width/2;";
    page += "  const dy=cy - r.height/2;";
    page += "  const max=r.width/2 - 35;";
    page += "  const ndx=clamp(dx,-max,max);";
    page += "  const ndy=clamp(dy,-max,max);";
    page += "  x=Math.round((ndx/max)*100);";
    page += "  y=Math.round((-ndy/max)*100);";
    page += "  if (Math.abs(x) < 4) x=0;";
    page += "  if (Math.abs(y) < 4) y=0;";
    page += "  setStick(r.width/2 + ndx, r.height/2 + ndy);";
    page += "  updateStatus();";
    page += "  sendDriveNow();";
    page += "}";

    page += "let dragging=false;";
    page += "joy.addEventListener('pointerdown',(e)=>{";
    page += "  dragging=true;";
    page += "  joy.setPointerCapture(e.pointerId);";
    page += "  posToXY(e.clientX,e.clientY);";
    page += "});";
    page += "joy.addEventListener('pointermove',(e)=>{";
    page += "  if(!dragging) return;";
    page += "  posToXY(e.clientX,e.clientY);";
    page += "});";
    page += "joy.addEventListener('pointerup',()=>{";
    page += "  dragging=false;";
    page += "  x=0; y=0;";
    page += "  setStick(130,130);";
    page += "  updateStatus('released');";
    page += "  sendDriveNow();";
    page += "});";
    page += "joy.addEventListener('pointercancel',()=>{";
    page += "  dragging=false;";
    page += "  x=0; y=0;";
    page += "  setStick(130,130);";
    page += "  updateStatus('cancel');";
    page += "  sendDriveNow();";
    page += "});";

    // Slider
    page += "thr.addEventListener('input',()=>{";
    page += "  t=parseInt(thr.value,10)||0;";
    page += "  tval.textContent=t;";
    page += "  updateStatus('slider');";
    page += "  sendDriveNow();";
    page += "});";

    page += "updateStatus('ready');";
    page += "sendDriveNow();";
    page += "</script>";



    page += "</div></body></html>";

    sendHttpOk(client, "text/html; charset=utf-8", page);
}
