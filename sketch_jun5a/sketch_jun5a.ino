// DefaultBoard.ino
// ——————————————————————————————————————————————————————————
// This is a drop-in “Cyton-compatible” firmware for XIAO nRF52840.
// It uses OpenBCI_32bit_Library v3.1.2 with only the pin macros and
// interrupt setup changed. No DSPI.h, PIC32, or SD/WiFi code here.
//
// Wiring (XIAO nRF52840 Sense):
//   • ADS_DRDY → D1
//   • ADS_RST  → D2
//   • BOARD_ADS → D0
//
// On startup you should see (in Serial Monitor at 115200 baud):
//    $$$
//    OpenBCI V3 8-Channel
//    On Board ADS1299 Device ID: 0x3E
//    Firmware: v3.1.2
//    $$$
//
// Then “Start Streaming” in BCI-GUI will give you real 250 SPS data.
//
//——————————————————————————————————————————————————————————

#define __USER_ISR
#include <SPI.h>
#include <OpenBCI_32bit_Library.h>
#include <OpenBCI_32bit_Library_Definitions.h>

// Forward-declare the DRDY interrupt routine:
void onDRDY();



//——————————————————————————————————————————————————————————
// XIAO-nRF DRDY ISR: when ADS_DRDY pin FALLS, mark data ready.
void onDRDY() {
  board.channelDataAvailable = true;
}

//——————————————————————————————————————————————————————————
void setup() {
  // 1) Configure ADS1299 pins for XIAO:
  pinMode(BOARD_ADS, OUTPUT);
  digitalWrite(BOARD_ADS, HIGH);

  pinMode(ADS_DRDY, INPUT_PULLUP);
  pinMode(ADS_RST, OUTPUT);
  digitalWrite(ADS_RST, HIGH);

  // 2) Start Serial at 115200 (must match BCI-GUI):
  Serial.begin(115200);
  while (!Serial) { /* wait */ }

  // 3) Begin SPI bus:
  SPI.begin();

  // 4) Give ADS1299 a moment, then call board.begin():
  delay(50);
  board.begin();   // sets up ADS reset, does boardReset(), prints startup banner

  // 5) Override the PIC32 interrupt setup: attach an Arduino ISR to ADS_DRDY:
  attachInterrupt(digitalPinToInterrupt(ADS_DRDY), onDRDY, FALLING);
}

//——————————————————————————————————————————————————————————
void loop() {
  // A) If DRDY interrupt fired, read & send one packet:
  if (board.channelDataAvailable) {
    board.channelDataAvailable = false;
    board.updateChannelData();
    board.sendChannelData();
  }

  // B) Always check for incoming commands from BCI-GUI:
  if (Serial.available()) {
    char c = Serial.read();
    board.processChar(c);
  }

  // C) Let the library service any multi‐char timeouts, etc.:
  board.loop();
}
