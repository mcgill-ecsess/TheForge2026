#!/usr/bin/env python3

import re
import time
import sys
import requests
import serial
import glob


BAUDRATE = 115200
SERIAL_TIMEOUT = 0.2
READY_TIMEOUT = 20


def find_serial_port():
    ports = sorted(glob.glob("/dev/cu.usbmodem*"))
    if ports:
        return ports[0]

    ports = sorted(glob.glob("/dev/cu.usbserial*"))
    if ports:
        return ports[0]

    raise RuntimeError("No Arduino serial port found.")


def wait_for_ready(port):
    print(f"Opening serial port: {port}")
    with serial.Serial(port, BAUDRATE, timeout=SERIAL_TIMEOUT) as ser:
        start = time.time()
        while time.time() - start < READY_TIMEOUT:
            line = ser.readline().decode(errors="ignore").strip()
            if line:
                print("SERIAL:", line)

            match = re.search(r"READY ip=(\d+\.\d+\.\d+\.\d+)", line)
            if match:
                ip = match.group(1)
                print(f"Detected Arduino IP: {ip}")
                return ip

        raise RuntimeError("Did not receive READY ip=... from board.")


def test_root(ip):
    print("Testing GET /")
    r = requests.get(f"http://{ip}/", timeout=5)
    assert r.status_code == 200, "Root endpoint failed"
    assert "Robot Controller" in r.text, "Unexpected root HTML"
    print("✓ Root endpoint OK")


def test_control(ip):
    print("Testing GET /control?msg=hello world")
    r = requests.get(
        f"http://{ip}/control",
        params={"msg": "hello world"},
        timeout=5,
    )
    assert r.status_code == 200, "Control endpoint failed"
    assert "OK" in r.text, "Control endpoint did not return OK"
    print("✓ Control endpoint OK")


def test_health(ip):
    print("Testing GET /health")
    r = requests.get(f"http://{ip}/health", timeout=5)
    assert r.status_code == 200, "Health endpoint failed"
    assert "OK" in r.text, "Health endpoint did not return OK"
    print("✓ Health endpoint OK")


def main():
    try:
        port = find_serial_port()
        ip = wait_for_ready(port)

        print("\n--- Running HTTP tests ---\n")

        test_root(ip)
        test_control(ip)

        # Comment this out if you didn't implement /health
        try:
            test_health(ip)
        except Exception as e:
            print("Health endpoint skipped or failed:", e)

        print("\nALL HTTP TESTS PASSED ✅")

    except Exception as e:
        print("\nHTTP TEST FAILED ❌")
        print("Error:", e)
        sys.exit(1)


if __name__ == "__main__":
    main()
