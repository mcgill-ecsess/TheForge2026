Import("env")
import glob

def pick_port():
    # Prefer Arduino native USB modem ports
    ports = glob.glob("/dev/cu.usbmodem*")
    if ports:
        return sorted(ports)[0]

    # Fallback for some USB-serial adapters
    ports = glob.glob("/dev/cu.usbserial*")
    if ports:
        return sorted(ports)[0]

    return None

port = pick_port()
if not port:
    print("ERROR: No Arduino serial port found. Expected /dev/cu.usbmodem* or /dev/cu.usbserial*")
else:
    print("Auto-selected upload/test port:", port)
    env.Replace(UPLOAD_PORT=port)
    env.Replace(MONITOR_PORT=port)
    # Unit tests use the monitor port for output; this helps too:
    env.Replace(TEST_PORT=port)
