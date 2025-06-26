# Niura EEG Buds Firmware for nRF52840 XIAO (Seeed Studio)

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Pinout and Wiring](#pinout-and-wiring)
- [Firmware Structure](#firmware-structure)
- [How BLE Communication Works](#how-ble-communication-works)
- [Data Packet Structure](#data-packet-structure)
- [Supported Commands](#supported-commands)
- [Installation & Flashing](#installation--flashing)
  - [1. Bootloader Installation (Fresh nRF52840)](#1-bootloader-installation-fresh-nrf52840)
  - [2. Arduino IDE Setup](#2-arduino-ide-setup)
  - [3. Flashing the Firmware](#3-flashing-the-firmware)
- [OpenBCI GUI Integration](#openbci-gui-integration)
- [Troubleshooting](#troubleshooting)
- [Dependencies](#dependencies)
- [Related Repositories](#related-repositories)
- [Credits](#credits)
- [License](#license)
- [Contact](#contact)

---

## Overview

This repository contains firmware for **Niura EEG Buds**, custom-built for wireless EEG signal acquisition with the nRF52840 XIAO (Seeed Studio) and ADS1299-based OpenBCI-compatible front-end. This firmware enables the XIAO to stream multi-channel EEG data over BLE (Bluetooth Low Energy) and Serial, making it usable with custom OpenBCI GUIs or any BLE client. 

It includes separate firmware for the **left** and **right** buds, each using unique BLE service UUIDs to allow simultaneous operation. The code also provides robust command and data handling via UART and BLE—enabling real-time EEG visualization and experiment flexibility.

---

## Features

- **BLE Nordic UART Service (NUS) Streaming**
  - Unique service/characteristics per bud (left/right)
  - Up to 244 bytes per BLE packet (max MTU for ArduinoBLE)
  - Both command and EEG data transfer
- **ADS1299 + OpenBCI 32bit Library**
  - 3 EEG channels (expandable by code)
  - Real-time streaming, acquisition, and packetizing
  - Safe channel deactivation for unused channels (5 to 8)
- **Bi-Directional Command Handling**
  - BLE or Serial (UART) control
  - OpenBCI-compatible ASCII commands, e.g., start/stop, config, etc.
  - Immediate feedback via BLE notifications and serial prints
- **Interrupt-Driven Data Ready (DRDY)**
  - Accurate timing for EEG samples via hardware interrupt
- **Robust Initialization and Error Handling**
  - BLE error detection and recovery
  - Serial debug output for every stage
- **OpenBCI GUI & Custom GUI Compatibility**
  - Seamless integration with [@hiibrarahmad/Open_bci_GUI_with-custom](https://github.com/hiibrarahmad/Open_bci_GUI_with-custom)
  - Data can also be read by any BLE/NUS client or serial terminal

---

## Hardware Requirements

| Component               | Details                                                           |
|-------------------------|-------------------------------------------------------------------|
| **MCU**                 | nRF52840 XIAO (Seeed Studio)                                     |
| **EEG Front-End**       | ADS1299 (as used in OpenBCI 32bit)                               |
| **Power**               | 3.3V supply (battery or USB)                                     |
| **Wiring**              | SPI (SCK, MISO, MOSI, CS), DRDY, RESET                           |
| **Programming**         | USB for firmware upload; SWD (or USB) for bootloader if needed    |

---

## Pinout and Wiring

**Typical Pin Connections:**

| XIAO Pin    | ADS1299 Pin | Function         | Firmware Macro    |
|:------------|:------------|:-----------------|:-----------------|
| D2          | DRDY        | Data Ready       | ADS_DRDY         |
| D3          | RST         | Reset            | ADS_RST          |
| D10         | CS          | Chip Select      | BOARD_ADS        |
| D11         | MOSI        | SPI MOSI         |                  |
| D12         | MISO        | SPI MISO         |                  |
| D13         | SCK         | SPI Clock        |                  |
| 3V3         | VCC         | Power            |                  |
| GND         | GND         | Ground           |                  |

**Note:**  
- Pin assignments may vary based on your shield or custom board.  
- Update `BOARD_ADS`, `ADS_DRDY`, and `ADS_RST` in the firmware if your wiring is different.

---

## Firmware Structure

- `lef_eeg_air_bud.ino`: Firmware for the **left** EEG bud (`Niura EEG BUDS L`)
- `right_eeg_air_bud.ino`: Firmware for the **right** EEG bud (`Niura EEG BUDS R`)
- Both files include:
  - BLE initialization with unique UUIDs
  - Serial diagnostic output
  - SPI and ADS1299 setup
  - Interrupt-based data ready handling
  - Command parsing and data streaming logic

---

## How BLE Communication Works

- **Service Setup**: Each bud advertises a custom BLE service (NUS-like) with RX (write) and TX (notify) characteristics.
- **Command Reception**: BLE central (e.g., GUI or smartphone) writes commands (max 20 bytes) to the RX characteristic.
- **Data Transmission**: EEG data packets are sent as notifications (up to 244 bytes each) via the TX characteristic.
- **Simultaneous Serial Support**: All commands/data are mirrored over UART for debugging or legacy OpenBCI GUI use.
- **Start/Stop Streaming**: Sending `'b'` over BLE or serial starts streaming, `'s'` stops it.

### BLE UUIDs

| Bud   | Service UUID                            | RX Char UUID                            | TX Char UUID                            |
|-------|-----------------------------------------|-----------------------------------------|-----------------------------------------|
| Left  | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`  | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`  | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`  |
| Right | `6E400101-B5A3-F393-E0A9-E50E24DCCA9E`  | `6E400102-B5A3-F393-E0A9-E50E24DCCA9E`  | `6E400103-B5A3-F393-E0A9-E50E24DCCA9E`  |

---

## Data Packet Structure

- Each data packet contains a start byte, sample counter, channel data, and an end byte.
- Example for 3 channels:

| Byte Index  | Contents                                  |
|-------------|-------------------------------------------|
| 0           | Packet Start (`PCKT_START`)               |
| 1           | Sample Counter (rolling, 0-255)           |
| 2           | Channel 0 Index (0)                       |
| 3-5         | Channel 0 24-bit raw data (`[0],[1],[2]`) |
| 6           | Channel 1 Index (1)                       |
| 7-9         | Channel 1 24-bit raw data                 |
| 10          | Channel 2 Index (2)                       |
| 11-13       | Channel 2 24-bit raw data                 |
| 14          | Packet End (`PCKT_END | curPacketType`)   |

- **Note:** Each channel's data is 3 bytes (24 bits, big endian).
- **MTU:** BLE notifications are split into chunks of up to 244 bytes.

---

## Supported Commands

| Command | Description                 | Sent via       | Action                                                      |
|---------|-----------------------------|----------------|-------------------------------------------------------------|
| `b`     | Begin data streaming        | BLE/Serial     | Sets DRDY LOW, starts ADS1299 streaming, enables streaming  |
| `s`     | Stop data streaming         | BLE/Serial     | Stops ADS1299 streaming, disables streaming                 |
| Any     | OpenBCI ASCII command       | BLE/Serial     | Passed to OpenBCI parser, e.g. channel config, test mode    |

- All received commands are echoed to both BLE (as notification) and UART for debugging.

---

## Installation & Flashing

### 1. Bootloader Installation (Fresh nRF52840)

If your XIAO nRF52840 is blank or has no bootloader, you MUST flash one before uploading firmware with Arduino IDE.

- Follow the guide and use the resources in:  
  [@hiibrarahmad/bootloadnrf52840](https://github.com/hiibrarahmad/bootloadnrf52840)
- Includes:
  - UF2 bootloader binaries
  - Instructions for using a SWD debugger or USB drag-and-drop
  - Trouble recovery steps

### 2. Arduino IDE Setup

- Download & install [Arduino IDE](https://www.arduino.cc/en/software)
- Install XIAO nRF52840 board support via Boards Manager ([official guide](https://wiki.seeedstudio.com/XIAO_BLE_Board_Support_Package/))
- Install required libraries:
  - [ArduinoBLE](https://www.arduino.cc/en/Reference/ArduinoBLE)
  - [OpenBCI_32bit_Library](https://github.com/hiibrarahmad/OpenBCI_32bit_Library)
- Select the correct Board: **Seeed nRF52840 XIAO**
- Select the correct port (shows as "XIAO nRF52840" or similar)

### 3. Flashing the Firmware

- Open either `lef_eeg_air_bud.ino` or `right_eeg_air_bud.ino` in Arduino IDE.
- Compile (`Verify`) the sketch to check for errors.
- Upload (`Upload`) to the XIAO nRF52840.
- Open the Serial Monitor (baud: 115200) for debug logs.

---

## OpenBCI GUI Integration

- Use the customized GUI:  
  [@hiibrarahmad/Open_bci_GUI_with-custom](https://github.com/hiibrarahmad/Open_bci_GUI_with-custom)
- Features:
  - BLE and Serial connection support
  - Visualization tuned for Niura EEG Buds packet format
  - Start/stop and configuration commands from GUI
- You may also use any BLE terminal (e.g., nRF Connect) or serial monitor for testing
  - Ensure you use correct UUIDs for connecting

---

## Troubleshooting

**BLE Not Advertising**
- Confirm BLE.begin() returns true (check serial output)
- Try power-cycling the board
- Make sure no other device is connected to the bud

**Cannot Upload Sketch**
- Ensure correct board and port
- If upload fails repeatedly, check bootloader with [@hiibrarahmad/bootloadnrf52840](https://github.com/hiibrarahmad/bootloadnrf52840)

**No EEG Data**
- Confirm correct wiring (DRDY, CS, RESET, SPI)
- Check if DRDY interrupt is firing (add debug prints)
- Check that OpenBCI_32bit_Library is correctly initialized

**Commands Not Responding**
- Check that BLE central (app/GUI) is writing to RX characteristic
- Try sending commands over serial to verify firmware logic

**Data Corruption or Packet Loss**
- Ensure BLE MTU is set to 244 on central/client
- Reduce number of channels or data rate if necessary

---

## Dependencies

- [ArduinoBLE](https://www.arduino.cc/en/Reference/ArduinoBLE)
- [OpenBCI_32bit_Library (custom fork)](https://github.com/hiibrarahmad/OpenBCI_32bit_Library)
- [@hiibrarahmad/bootloadnrf52840](https://github.com/hiibrarahmad/bootloadnrf52840) (for bootloader installation)
- [@hiibrarahmad/Open_bci_GUI_with-custom](https://github.com/hiibrarahmad/Open_bci_GUI_with-custom) (for visualization and command interface)

---

## Related Repositories

- [@hiibrarahmad/bootloadnrf52840](https://github.com/hiibrarahmad/bootloadnrf52840): Full instructions and resources for flashing the nRF52840 XIAO bootloader for Arduino compatibility, including troubleshooting and recovery methods.
- [@hiibrarahmad/Open_bci_GUI_with-custom](https://github.com/hiibrarahmad/Open_bci_GUI_with-custom): A custom OpenBCI GUI compatible with the Niura EEG Buds' BLE/serial packet format and command set.
- [@hiibrarahmad/OpenBCI_32bit_Library](https://github.com/hiibrarahmad/OpenBCI_32bit_Library): Forked and ported library for nRF52840 and custom EEG hardware.

---

## Credits

- Firmware and porting: [@hiibrarahmad](https://github.com/hiibrarahmad)
- OpenBCI Community: For the original libraries and hardware inspiration
- Seeed Studio: For XIAO nRF52840 hardware

---

## License

MIT License (or specify your own).

---

## Contact

For support or collaboration, open an issue on this repo or contact [@hiibrarahmad](https://github.com/hiibrarahmad).

---

## Changelog

- v1.0.0: Initial public release (separate left/right bud firmware, BLE/serial, OpenBCI integration)
- v1.1.0: Improved documentation, error handling, and build instructions

---

## FAQ (Frequently Asked Questions)

**Q: Can I use more than 3 EEG channels?**  
A: Yes, change `numChannels` in the source code. Ensure BLE packet size does not exceed 244 bytes.

**Q: Can I modify the BLE names/UUIDs?**  
A: Yes, but both buds must use different UUIDs. Update the UUIDs in your GUI/client as well.

**Q: Can these buds stream to a mobile app?**  
A: Yes, use a BLE NUS-compatible app and configure UUIDs as above.

**Q: What is the default baud rate for serial?**  
A: Both Serial and Serial1 use 115200 baud.

**Q: How do I know streaming is active?**  
A: Check for serial debug prints and BLE notifications. The `isStreaming` flag in the code is set to `true`.

---