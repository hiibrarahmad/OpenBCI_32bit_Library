import serial
import time
import sys
import csv
from datetime import datetime

# ────────────────────────────────────────────────────────────────────────────────
# 1) EDIT THIS TO MATCH YOUR XIAO's COM/TTY PORT:
SERIAL_PORT = "COM6"       # e.g. "COM4" on Windows or "/dev/ttyACM0" on Linux/macOS
BAUD_RATE   = 115200
TIMEOUT_SEC = 0.5
PACKET_LEN  = 33           # raw packet length: 1+1+24+6+1 = 33 bytes
# ────────────────────────────────────────────────────────────────────────────────

def twos_complement_24(raw3: bytes) -> int:
    """Convert 3 big-endian bytes into a signed 24-bit integer."""
    val = (raw3[0] << 16) | (raw3[1] << 8) | (raw3[2])
    if val & 0x800000:
        val -= 1 << 24
    return val

def twos_complement_16(msb: int, lsb: int) -> int:
    """Convert two big-endian bytes into a signed 16-bit integer."""
    raw = (msb << 8) | lsb
    if raw & 0x8000:
        raw -= 1 << 16
    return raw

def hexdump_line(data: bytes) -> str:
    """Return a string of space-separated hex bytes for a short snippet."""
    return " ".join(f"{b:02X}" for b in data)

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT_SEC)
    except serial.SerialException as e:
        print(f"ERROR: cannot open port {SERIAL_PORT!r}: {e}")
        sys.exit(1)

    # Open CSV file for writing
    csv_file = open("output.csv", "w", newline="")
    csv_writer = csv.writer(csv_file)
    # Write header row: Timestamp, Sample, Ch1…Ch8, Aux1…Aux3
    header = ["Timestamp", "Sample"] + [f"Ch{i+1}" for i in range(8)] + [f"Aux{i+1}" for i in range(3)]
    csv_writer.writerow(header)

    print(f"Opened {SERIAL_PORT} @ {BAUD_RATE} baud. Waiting 0.2 s before sending ‘b’…")
    time.sleep(0.2)

    # 2) Send lowercase 'b' to start streaming
    ser.write(b"b")
    ser.flush()
    print("→ Sent 'b'. Now reading raw packets for 30 seconds…\n")

    # Print a combined header for console output
    console_header = ["Raw-Hex", "Sample"] + [f"Ch{i+1}" for i in range(8)] + [f"Aux{i+1}" for i in range(3)]
    # Adjust column widths for console print (Raw-Hex gets ~100 chars, numbers get 10-wide)
    print(f"{'Raw-Hex':<100} {'Sample':>6}", end="")
    for i in range(8):
        print(f"{('Ch'+str(i+1)):>10}", end="")
    for i in range(3):
        print(f"{('Aux'+str(i+1)):>10}", end="")
    print()
    print("-" * (100 + 6 + 10*11))

    buffer = bytearray()
    start_time = time.time()

    try:
        while True:
            # Stop after 30 seconds
            if time.time() - start_time >= 30.0:
                print("\n☑️  30 seconds elapsed—stopping recording.")
                break

            # Read until we collect at least one full 33-byte packet
            needed = PACKET_LEN - len(buffer)
            if needed > 0:
                chunk = ser.read(needed)
                if not chunk:
                    # timeout—loop back and try again
                    continue
                buffer.extend(chunk)

            # Realign on 0x41
            if len(buffer) and buffer[0] != 0x41:
                idx = buffer.find(b'\x41', 1)
                if idx < 0:
                    buffer.clear()
                    continue
                del buffer[:idx]

            # If we have ≥33 bytes, parse one packet
            if len(buffer) >= PACKET_LEN:
                pkt = bytes(buffer[:PACKET_LEN])
                del buffer[:PACKET_LEN]

                # Verify end byte (high nibble 0xC0/0xD0/0xA0 indicates data)
                end_byte = pkt[32]
                if (end_byte & 0xF0) not in (0xC0, 0xD0, 0xA0):
                    # not a valid data packet; skip it
                    continue

                # Timestamp for CSV
                ts = datetime.now().isoformat(sep=" ", timespec="milliseconds")

                # • Raw-hex (first 33 bytes)
                raw_hex_str = hexdump_line(pkt)

                # • Byte[1] = sample counter
                sample_ctr = pkt[1]

                # • Bytes[2..25] = eight 3-byte channels
                ch_vals = []
                for i in range(8):
                    start = 2 + 3*i
                    raw3 = pkt[start:start+3]
                    ch_vals.append(twos_complement_24(raw3))

                # • Bytes[26..31] = three 2-byte auxiliary values
                aux_vals = []
                for i in range(3):
                    m = pkt[26 + 2*i]
                    l = pkt[26 + 2*i + 1]
                    aux_vals.append(twos_complement_16(m, l))

                # 1) Write to CSV
                row = [ts, sample_ctr] + ch_vals + aux_vals
                csv_writer.writerow(row)
                csv_file.flush()

                # 2) Also print to console (as before)
                print(f"{raw_hex_str:<100} {sample_ctr:>6d}", end="")
                for v in ch_vals:
                    print(f"{v:>10d}", end="")
                for a in aux_vals:
                    print(f"{a:>10d}", end="")
                print()

    except KeyboardInterrupt:
        print("\n⏹ Interrupted by user—exiting.")
    except Exception as ex:
        print(f"\n⚠️ Unexpected error: {ex!r}")
    finally:
        ser.close()
        csv_file.close()

if __name__ == "__main__":
    main()
