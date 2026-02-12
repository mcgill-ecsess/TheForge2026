#include <Arduino.h>
#include <unity.h>
#include <WiFiS3.h>

#include "Controller.h"

// Match your ctor usage
static Controller ctrl("TeamRobot", "12345678");

// For verifying /control callback
static volatile bool g_cb_called = false;
static String g_last_msg;

static void onMessage(const String& msg) {
  g_cb_called = true;
  g_last_msg = msg;
}

// Unity required hooks
void setUp(void) {
  g_cb_called = false;
  g_last_msg = "";
}

void tearDown(void) {}

static String httpGetAndPump(const char* path, uint32_t pumpMs = 300) {
  IPAddress ip = WiFi.localIP();
  TEST_ASSERT_TRUE_MESSAGE(ip[0] != 0, "WiFi.localIP() invalid (AP not started?)");

  WiFiClient c;
  TEST_ASSERT_TRUE_MESSAGE(c.connect(ip, 80), "Failed to connect to server:80");

  c.print("GET ");
  c.print(path);
  c.println(" HTTP/1.1");
  c.print("Host: ");
  c.println(ip);
  c.println("Connection: close");
  c.println();

  // Important: your server only responds when update() runs
  const unsigned long t0 = millis();
  while (millis() - t0 < pumpMs) {
    ctrl.update();
    delay(1);
  }

  // Read response (up to ~1s)
  String resp;
  const unsigned long t1 = millis();
  while (millis() - t1 < 1000) {
    while (c.available()) resp += (char)c.read();
    if (!c.connected()) break;
    delay(1);
  }
  c.stop();
  return resp;
}

void test_begin_ap_starts_listening(void) {
  bool ok = ctrl.beginAP();
  TEST_ASSERT_TRUE_MESSAGE(ok, "beginAP() returned false");
  TEST_ASSERT_EQUAL_INT_MESSAGE(WL_AP_LISTENING, WiFi.status(), "WiFi.status() != WL_AP_LISTENING");
}

void test_root_returns_html(void) {
  // Ensure AP is running (if this test runs alone)
  if (WiFi.status() != WL_AP_LISTENING) {
    TEST_ASSERT_TRUE(ctrl.beginAP());
  }

  String resp = httpGetAndPump("/");
  TEST_ASSERT_TRUE_MESSAGE(resp.indexOf("200 OK") >= 0, "No 200 OK");
  TEST_ASSERT_TRUE_MESSAGE(resp.indexOf("Robot Controller") >= 0, "HTML page content missing");
}

void test_control_triggers_callback(void) {
  if (WiFi.status() != WL_AP_LISTENING) {
    TEST_ASSERT_TRUE(ctrl.beginAP());
  }

  ctrl.registerCallback(onMessage);

  String resp = httpGetAndPump("/control?msg=hello+world");
  TEST_ASSERT_TRUE_MESSAGE(resp.indexOf("200 OK") >= 0, "No 200 OK for /control");
  TEST_ASSERT_TRUE_MESSAGE(resp.indexOf("OK") >= 0, "Body missing OK");

  TEST_ASSERT_TRUE_MESSAGE(g_cb_called, "Callback not called");
  TEST_ASSERT_EQUAL_STRING_MESSAGE("hello world", g_last_msg.c_str(), "Message mismatch");
}

// If you added /health endpoint
void test_health_endpoint_ok(void) {
  if (WiFi.status() != WL_AP_LISTENING) {
    TEST_ASSERT_TRUE(ctrl.beginAP());
  }

  String resp = httpGetAndPump("/health");
  TEST_ASSERT_TRUE_MESSAGE(resp.indexOf("200 OK") >= 0, "No 200 OK for /health");
  TEST_ASSERT_TRUE_MESSAGE(resp.indexOf("OK") >= 0, "Body missing OK");
}

int runUnityTests() {
  UNITY_BEGIN();
  RUN_TEST(test_begin_ap_starts_listening);
  RUN_TEST(test_root_returns_html);
  RUN_TEST(test_control_triggers_callback);

  // Comment this out if you didn't add /health
  // RUN_TEST(test_health_endpoint_ok);

  return UNITY_END();
}

void setup() {
  delay(2000);          // recommended for embedded Unity so serial is ready
  Serial.begin(115200);
  runUnityTests();
}

void loop() {}
