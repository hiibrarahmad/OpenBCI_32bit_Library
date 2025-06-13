import serial, time
from datetime import datetime

PORT = "COM6"    # ← your port
BAUD = 115200

def hexdump(data: bytes) -> str:
    return " ".join(f"{b:02X}" for b in data)

ser = serial.Serial(PORT, BAUD, timeout=0.1)
ser.write(b"b")          # start streaming
ser.flush()
print("→ Sent 'b', now streaming…\n")

try:
    while True:
        data = ser.read(ser.in_waiting or 1)
        if data:
            ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
            print(f"{ts}  {hexdump(data)}")
        time.sleep(0.01)
except KeyboardInterrupt:
    print("\n🛑 Stopped")
finally:
    ser.close()
