#include <SPI.h>
#include <ArduinoBLE.h>
#include <OpenBCI_32bit_Library.h>
#include <OpenBCI_32bit_Library_Definitions.h>

// Reference library's global board
extern OpenBCI_32bit_Library board;

// BLE NUS service UUIDs
static const char* BLE_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* BLE_CHAR_RX_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* BLE_CHAR_TX_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

BLEService nusService(BLE_SERVICE_UUID);
BLECharacteristic nusRxChar(BLE_CHAR_RX_UUID, BLEWriteWithoutResponse | BLEWrite, 20);
BLECharacteristic nusTxChar(BLE_CHAR_TX_UUID, BLENotify, 244);
bool isStreaming = false;

void sendBleData(const uint8_t* data, size_t len) {
  size_t offset = 0;
  while (offset < len) {
    size_t chunk = min((size_t)244, len - offset);
    nusTxChar.writeValue(data + offset, chunk);
    offset += chunk;
  }
}

void onDRDY() {
  board.channelDataAvailable = true;
}

void bleWritten(BLEDevice central, BLECharacteristic characteristic) {
  uint8_t buf[20];
  int len = nusRxChar.valueLength();
  nusRxChar.readValue(buf, len);
  for (int i = 0; i < len; i++) {
    char c = (char)buf[i];
    Serial.write(c);
    Serial1.write(c);
    nusTxChar.writeValue((uint8_t*)&c, 1);
    if (c == 'b') {
      pinMode(ADS_DRDY, OUTPUT);
      digitalWrite(ADS_DRDY, LOW);
      delayMicroseconds(4);
      pinMode(ADS_DRDY, INPUT_PULLUP);
      board.streamStart(); isStreaming = true;
    } else if (c == 's') {
      board.streamStop(); isStreaming = false;
    }
    board.processChar(c);
  }
}

void setup() {
  // ADS1299 wiring
  pinMode(BOARD_ADS, OUTPUT); digitalWrite(BOARD_ADS, HIGH);
  pinMode(ADS_DRDY, INPUT_PULLUP);
  pinMode(ADS_RST, OUTPUT);  digitalWrite(ADS_RST, HIGH);

  // Serial for debug
  Serial.begin(115200);
  while (!Serial);
  Serial1.begin(115200);
  board.beginSerial1(115200);

  // SPI + ADS init
  SPI.begin(); delay(50);
  board.begin();
  for (uint8_t ch = 5; ch <= 8; ch++) board.streamSafeChannelDeactivate(ch);
  attachInterrupt(digitalPinToInterrupt(ADS_DRDY), onDRDY, FALLING);

  // BLE init
  if (!BLE.begin()) {
    Serial.println("BLE init failed");
    while (1);
  }
  BLE.setLocalName("nuira eeg");
  BLE.setAdvertisedService(nusService);
  nusService.addCharacteristic(nusRxChar);
  nusService.addCharacteristic(nusTxChar);
  BLE.addService(nusService);
  nusRxChar.setEventHandler(BLEWritten, bleWritten);
  BLE.advertise();
  Serial.println("BLE advertise nuira eeg");
}

void loop() {
  BLE.poll();

  if (isStreaming && digitalRead(ADS_DRDY) == LOW) board.channelDataAvailable = true;
  if (isStreaming && board.channelDataAvailable) {
    board.channelDataAvailable = false;
    board.updateChannelData();

    const int numChannels = 3;  // only send first 3 channels
    const int auxBytes = OPENBCI_NUMBER_OF_BYTES_AUX;
    static uint8_t pkt[2 + numChannels*(1 + 3) + auxBytes + 1];
    int idx = 0;
    pkt[idx++] = PCKT_START;
    pkt[idx++] = board.sampleCounter;
    for (uint8_t ch = 0; ch < numChannels; ch++) {
      pkt[idx++] = ch;
      pkt[idx++] = board.boardChannelDataRaw[3*ch + 0];
      pkt[idx++] = board.boardChannelDataRaw[3*ch + 1];
      pkt[idx++] = board.boardChannelDataRaw[3*ch + 2];
    }
    for (int a = 0; a < auxBytes; a++) pkt[idx++] = board.auxData[a];
    pkt[idx++] = (uint8_t)(PCKT_END | board.curPacketType);

    sendBleData(pkt, idx);
    board.sendChannelData();  // also raw to Serial
  }

  while (Serial.available()) {
    char c = Serial.read();
    Serial1.write(c);
    char tmp[3] = { (char)c, '\r', '\n' };
    sendBleData((uint8_t*)tmp, 3);
    if (c == 'b') { pinMode(ADS_DRDY, OUTPUT); digitalWrite(ADS_DRDY, LOW);
      delayMicroseconds(4); pinMode(ADS_DRDY, INPUT_PULLUP);
      board.streamStart(); isStreaming = true; }
    else if (c == 's') { board.streamStop(); isStreaming = false; }
    board.processChar(c);
  }
  while (Serial1.available()) {
    char c = Serial1.read();
    Serial.write(c);
    char tmp[3] = { (char)c, '\r', '\n' };
    sendBleData((uint8_t*)tmp, 3);
    if (c == 'b') { pinMode(ADS_DRDY, OUTPUT); digitalWrite(ADS_DRDY, LOW);
      delayMicroseconds(4); pinMode(ADS_DRDY, INPUT_PULLUP);
      board.streamStart(); isStreaming = true; }
    else if (c == 's') { board.streamStop(); isStreaming = false; }
    board.processChar(c);
  }

  board.loop();
}
