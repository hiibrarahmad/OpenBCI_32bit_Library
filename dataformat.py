import serial
import time
import sys
from datetime import datetime

# ────────────────────────────────────────────────────────────────────────────────
# 1) EDIT THIS TO MATCH YOUR CYTON's COM/TTY PORT:
SERIAL_PORT = "COM6"       # e.g. "COM4" on Windows or "/dev/ttyACM0" on Linux/macOS
BAUD_RATE   = 115200
TIMEOUT_SEC = 0.5
PACKET_LEN  = 33           # Cyton: 1 start + 1 counter + 24 data + 6 aux + 1 stop
START_BYTE  = 0xA0         # Cyton packet start
RAW_ONLY    = True         # If True, only dump raw hex (no CSV parsing)
LOG_RAW     = True         # If True, append raw hex to raw_packets.log
# ────────────────────────────────────────────────────────────────────────────────

def hexdump_line(data: bytes) -> str:
    """Return a string of space‐separated hex bytes for a snippet."""
    return " ".join(f"{b:02X}" for b in data)

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT_SEC)
    except serial.SerialException as e:
        print(f"ERROR: cannot open port {SERIAL_PORT!r}: {e}")
        sys.exit(1)

    if LOG_RAW:
        raw_log = open("raw_packets.log", "a")

    print(f"Opened {SERIAL_PORT} @ {BAUD_RATE} baud. Waiting 0.2 s before sending ‘b’…")
    time.sleep(0.2)

    # Tell the Cyton to start streaming
    ser.write(b"b")
    ser.flush()
    print("→ Sent 'b'. Now reading raw Cyton packets for 30 seconds…\n")

    buffer = bytearray()
    start_time = time.time()

    try:
        while time.time() - start_time < 30.0:
            # Fill buffer until we have at least one packet
            needed = PACKET_LEN - len(buffer)
            if needed > 0:
                chunk = ser.read(needed)
                if not chunk:
                    continue
                buffer.extend(chunk)

            # Realign on Cyton START_BYTE (0xA0)
            if buffer and buffer[0] != START_BYTE:
                idx = buffer.find(bytes([START_BYTE]), 1)
                if idx < 0:
                    buffer.clear()
                    continue
                del buffer[:idx]

            # If we have a full packet, process it
            if len(buffer) >= PACKET_LEN:
                pkt = bytes(buffer[:PACKET_LEN])
                del buffer[:PACKET_LEN]

                raw_hex = hexdump_line(pkt)
                ts = datetime.now().isoformat(sep=" ", timespec="milliseconds")

                # 1) Print raw hex
                print(f"{ts}  {raw_hex}")

                # 2) Optionally log it
                if LOG_RAW:
                    raw_log.write(f"{ts}  {raw_hex}\n")

                if RAW_ONLY:
                    continue

                # ─── BELOW HERE: if you still want to parse channels ───

                # Verify stop byte (high‐nibble 0xC0/0xD0/0xA0)
                end_byte = pkt[-1]
                if (end_byte & 0xF0) not in (0xC0, 0xD0, 0xA0):
                    continue

                # Sample counter
                sample_ctr = pkt[1]

                # 8 × 3-byte channels
                def tc24(raw3):
                    v = (raw3[0]<<16)|(raw3[1]<<8)|raw3[2]
                    return v - (1<<24) if (v & 0x800000) else v
                ch = [tc24(pkt[2+3*i:5+3*i]) for i in range(8)]

                # 3 × 2-byte aux
                def tc16(msb, lsb):
                    v = (msb<<8)|lsb
                    return v - (1<<16) if (v & 0x8000) else v
                aux = [tc16(pkt[26+2*i], pkt[27+2*i]) for i in range(3)]

                # Print parsed values
                print(f"  Sample #{sample_ctr}:", end="")
                for v in ch:  print(f"  Ch={v}", end="")
                for a in aux: print(f"  Aux={a}", end="")
                print()

    except KeyboardInterrupt:
        print("\n⏹ Interrupted by user—exiting.")
    finally:
        ser.close()
        if LOG_RAW:
            raw_log.close()

if __name__ == "__main__":
    main()
