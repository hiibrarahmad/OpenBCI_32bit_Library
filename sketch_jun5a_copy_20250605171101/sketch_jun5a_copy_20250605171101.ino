// DefaultBoard.ino
// ——————————————————————————————————————————————————————————
// This sketch configures an XIAO‐nRF52840 Sense to drive an ADS1299 using
// the OpenBCI_32bit_Library. It echoes all output to both USB‐Serial (Serial)
// and UART1 (Serial1). Sending lowercase 'b' on either interface will cause
// the ADS to start streaming. All data packets and printAll()/printlnAll() text
// appear on both Serial and Serial1.
//
// Wiring (XIAO nRF52840 Sense):
//   • ADS_DRDY  → D1
//   • ADS_RST   → D2
//   • BOARD_ADS → D0   (chip select for on-board ADS1299)
//   • UART1 TX/RX → D11 / D12
//
// On startup (USB‐Serial and UART1 at 115200) you should see:
//    $$$
//    OpenBCI V3 8-Channel
//    On Board ADS1299 Device ID: 0x3E
//    Firmware: v3.1.2
//    $$$
//
// Sending ‘b’ (lowercase) over USB‐Serial will trigger a brief DRDY pulse,
// switch DRDY back to input, then call streamStart(). After that every time
// DRDY goes low (or we detect it via polling), we read one sample and print it.
//——————————————————————————————————————————————————————————

#define __USER_ISR
#include <SPI.h>
#include <OpenBCI_32bit_Library.h>
#include <OpenBCI_32bit_Library_Definitions.h>

// Forward‐declare the DRDY interrupt routine:
void onDRDY();

//— ADS DRDY ISR —————————————————————————————————————————————————————
// Called when the ADS_DRDY pin FALLS (HIGH→LOW). We simply mark data ready.
void onDRDY() {
  board.channelDataAvailable = true;
}

void setup() {
  // 1) Configure ADS1299 pins on the XIAO:
  pinMode(BOARD_ADS, OUTPUT);
  digitalWrite(BOARD_ADS, HIGH);   // keep CS high until SPI.beginTransaction()

  pinMode(ADS_DRDY, INPUT_PULLUP);  // DRDY idles HIGH, pulses LOW
  pinMode(ADS_RST, OUTPUT);
  digitalWrite(ADS_RST, HIGH);      // keep ADS not reset

  // 2) Start USB Serial at 115200 baud (must match BCI‐GUI):
  Serial.begin(115200);
  while (!Serial) {
    // wait for USB‐Serial to be ready
  }

  // 3) Also bring up UART1 on pins D11/D12 at 115200:
  //    (leave commented if you don’t actually need UART1)
  // Serial1.begin(115200);

  // 4) Tell the OpenBCI library to broadcast everything to Serial1, too:
  //    Internally, this calls beginSerial1(115200) and sets iSerial1.tx = true.
  //    If you don’t have a USB‐Serial on Serial1, you can skip this line.
  board.beginSerial1(115200);

  // 5) Begin SPI bus (needed by the ADS1299 library):
  SPI.begin();

  // 6) Brief delay, then call board.begin():
  delay(50);
  board.begin();
  // Because we called beginSerial1(...), the startup banner (“$$$\nOpenBCI V3…”) 
  // is echoed to BOTH Serial and Serial1.

  // 7) Attach an interrupt to ADS_DRDY for the falling edge:
  attachInterrupt(digitalPinToInterrupt(ADS_DRDY), onDRDY, FALLING);

  // 8) (Optional) Immediately start streaming so you don’t have to type ‘b’
  //    at all. If you uncomment the next two lines, you’ll begin streaming 
  //    as soon as the board boots. Otherwise, comment them out and use ‘b’.
  //
  // pinMode(ADS_DRDY, OUTPUT);       
  // digitalWrite(ADS_DRDY, LOW);     
  // pinMode(ADS_DRDY, INPUT_PULLUP);  
  // board.streamStart();
}

void loop() {
  // ----------------------------------------------------------------------------------
  // (A) POLL-BASED FALLBACK: if DRDY is LOW right now, force channelDataAvailable = true.
  //      This ensures you’ll see data even if the interrupt never fires perfectly.
  if (digitalRead(ADS_DRDY) == LOW) {
    board.channelDataAvailable = true;
  }

  // (B) If DRDY interrupt (or polling) fired, read & send exactly one packet:
  if (board.channelDataAvailable) {
    board.channelDataAvailable = false;

    // 1) Pull one sample (3 bytes × 8 channels) via SPI, convert to 32‐bit:
    board.updateChannelData();

    // 2) Send the packet out on both Serial and Serial1:
    board.sendChannelData();
  }

  // (C) Check for incoming commands from USB-Serial:
  while (Serial.available()) {
    char c = Serial.read();

    // 1) Echo it back for debugging:
    Serial.print("Got from USB: ");
    Serial.println(c);
    Serial1.print("Got from USB: ");
    Serial1.println(c);

    // 2) If it’s lowercase 'b', force a tiny DRDY pulse then start streaming:
    if (c == 'b') {
      // Temporarily drive DRDY pin LOW for ~4 μs to “fake” a data-ready pulse:
      pinMode(ADS_DRDY, OUTPUT);
      digitalWrite(ADS_DRDY, LOW);
      delayMicroseconds(4);

      // Immediately restore DRDY as input so the ADS can drive it afterward:
      pinMode(ADS_DRDY, INPUT_PULLUP);

      // Now tell the ADS1299 to start continuous conversion:
      board.streamStart();

      // (Optional) print a message so you know streamStart() was called:
      Serial.println(">> streamStart() called");
      Serial1.println(">> streamStart() called");
    }

    // 3) Hand every character to the library’s built-in parser
    //    (for commands like 's', 'x', 'c', etc.):
    board.processChar(c);
  }

  // (D) If you also want to accept commands over UART1, uncomment this block:
  // while (Serial1.available()) {
  //   char c1 = Serial1.read();
  //   Serial.print("Got from UART1: ");
  //   Serial.println(c1);
  //   Serial1.print("Got from UART1: ");
  //   Serial1.println(c1);
  //
  //   if (c1 == 'b') {
  //     pinMode(ADS_DRDY, OUTPUT);
  //     digitalWrite(ADS_DRDY, LOW);
  //     delayMicroseconds(4);
  //     pinMode(ADS_DRDY, INPUT_PULLUP);
  //     board.streamStart();
  //     Serial1.println(">> streamStart() called from UART1");
  //   }
  //
  //   board.processChar(c1);
  // }

  // (E) Let the library service any multi-character timeouts or pending commands:
  board.loop();
}
