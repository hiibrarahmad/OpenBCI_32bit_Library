/*
insert header here

*/
#ifndef _____OpenBCI_32bit__
#define _____OpenBCI_32bit__

#include <Arduino.h>
#include <OpenBCI_Wifi_Master.h>
#include <OpenBCI_Wifi_Master_Definitions.h>
#include "OpenBCI_32bit_Library_Definitions.h"

//-----------------------------------------------------------------------------------
// Under mbed/Seeeduino (XIAO nRF52840) we do _not_ use PIC32 interrupts.
// Stub out the PIC32-only ISR declaration:
#if defined(__PIC32MX__)
void __USER_ISR ADS_DRDY_Service(void);
#endif

//-----------------------------------------------------------------------------------


class OpenBCI_32bit_Library {
public:
  // ENUMS
  typedef enum ACCEL_MODE {
    ACCEL_MODE_ON,
    ACCEL_MODE_OFF
  } ACCEL_MODE;

  typedef enum TIME_SYNC_MODE {
    TIME_SYNC_MODE_ON,
    TIME_SYNC_MODE_OFF
  } TIME_SYNC_MODE;

  typedef enum MARKER_MODE {
    MARKER_MODE_ON,
    MARKER_MODE_OFF
  } MARKER_MODE;

  typedef enum DEBUG_MODE {
    DEBUG_MODE_ON,
    DEBUG_MODE_OFF
  } DEBUG_MODE;

  typedef enum BOARD_MODE {
    BOARD_MODE_DEFAULT,
    BOARD_MODE_DEBUG,
    BOARD_MODE_ANALOG,
    BOARD_MODE_DIGITAL,
    BOARD_MODE_MARKER,
    BOARD_MODE_BLE,
    BOARD_MODE_END_OF_MODES  // This must be the last entry—insert any new board modes above this line
  } BOARD_MODE;

  typedef enum MULTI_CHAR_COMMAND {
    MULTI_CHAR_CMD_NONE,
    MULTI_CHAR_CMD_PROCESSING_INCOMING_SETTINGS_CHANNEL,
    MULTI_CHAR_CMD_PROCESSING_INCOMING_SETTINGS_LEADOFF,
    MULTI_CHAR_CMD_SERIAL_PASSTHROUGH,
    MULTI_CHAR_CMD_SETTINGS_BOARD_MODE,
    MULTI_CHAR_CMD_SETTINGS_SAMPLE_RATE,
    MULTI_CHAR_CMD_INSERT_MARKER
  } MULTI_CHAR_COMMAND;

  typedef enum PACKET_TYPE {
    PACKET_TYPE_ACCEL,
    PACKET_TYPE_RAW_AUX,
    PACKET_TYPE_USER_DEFINED,
    PACKET_TYPE_ACCEL_TIME_SET,
    PACKET_TYPE_ACCEL_TIME_SYNC,
    PACKET_TYPE_RAW_AUX_TIME_SET,
    PACKET_TYPE_RAW_AUX_TIME_SYNC
  } PACKET_TYPE;

  typedef enum SAMPLE_RATE {
    SAMPLE_RATE_16000,
    SAMPLE_RATE_8000,
    SAMPLE_RATE_4000,
    SAMPLE_RATE_2000,
    SAMPLE_RATE_1000,
    SAMPLE_RATE_500,
    SAMPLE_RATE_250
  } SAMPLE_RATE;

  // STRUCTS
  typedef struct {
    uint32_t baudRate;
    boolean  rx;
    boolean  tx;
  } SerialInfo;

  typedef struct {
    boolean rx;
    boolean tx;
  } SpiInfo;

  typedef struct {
    uint8_t  sampleNumber;
    uint8_t  data[BLE_TOTAL_DATA_BYTES];
    boolean  ready;
    boolean  flushing;
    uint8_t  bytesFlushed;
    uint8_t  bytesLoaded;
  } BLE;

  // Start up functions
  OpenBCI_32bit_Library();
  boolean accelHasNewData(void);
  void    accelUpdateAxisData(void);
  void    accelWriteAxisDataSerial(void);
  void    activateAllChannelsToTestCondition(byte testInputCode, byte amplitudeCode, byte freqCode);
  void    activateChannel(byte);                  // enable the selected channel
  void    ADS_writeChannelData(void);
  void    ADS_writeChannelDataAvgDaisy(void);
  void    ADS_writeChannelDataNoAvgDaisy(void);
  void    attachDaisy(void);
  void    begin(void);
  void    beginDebug(void);
  void    beginDebug(uint32_t);
  void    beginPinsAnalog(void);
  void    beginPinsDebug(void);
  void    beginPinsDefault(void);
  void    beginPinsDigital(void);
  void    beginSerial(void);
  void    beginSerial(uint32_t);
  void    beginSerial1(void);
  void    beginSerial1(uint32_t);
  void    boardReset(void);
  void    changeChannelLeadOffDetect(void);
  void    changeChannelLeadOffDetect(byte N);
  void    channelSettingsArraySetForAll(void);
  void    channelSettingsArraySetForChannel(byte N);
  void    channelSettingsSetForChannel(byte, byte, byte, byte, byte, byte, byte);
  boolean checkMultiCharCmdTimer(void);
  void    csLow(int);
  void    csHigh(int);
  void    configureInternalTestSignal(byte, byte);
  void    configureLeadOffDetection(byte, byte);
  void    deactivateChannel(byte);                // disable given channel 1-8 (or 16)
  void    disable_accel(void);                     // stop data acquisition and go into low power mode
  void    enable_accel(byte);                      // start accelerometer with default settings
  void    endMultiCharCmdTimer(void);
  void    endSerial(void);
  void    endSerial1(void);
  const char* getBoardMode(void);
  char    getChannelCommandForAsciiChar(char);
  char    getCharSerial(void);
  char    getCharSerial1(void);
  char    getConstrainedChannelNumber(byte);
  byte    getDefaultChannelSettingForSetting(byte);
  char    getDefaultChannelSettingForSettingAscii(byte);
  char    getGainForAsciiChar(char);
  uint8_t* getGains(void);
  char    getMultiCharCommand(void);
  char    getNumberForAsciiChar(char);
  const char* getSampleRate(void);
  char    getTargetSSForConstrainedChannelNumber(byte);
  char    getYesOrNoForAsciiChar(char);
  boolean hasDataSerial(void);
  boolean hasDataSerial1(void);
  boolean isADSDataAvailable(void);
  boolean isProcessingMultibyteMsg(void);
  void    leadOffConfigureSignalForAll(byte, byte);
  void    leadOffConfigureSignalForTargetSS(byte, byte, byte);
  void    leadOffSetForAllChannels(void);
  void    leadOffSetForChannel(byte, byte, byte);
  void    ledFlash(int);
  void    loop(void);
  void    printAll(char);
  void    printAll(const char*);
  void    printlnAll(void);
  void    printlnAll(const char*);
  void    printlnSerial(void);
  void    printlnSerial(char);
  void    printlnSerial(int);
  void    printlnSerial(int, int);
  void    printlnSerial(const char*);
  void    printSerial(int);
  void    printSerial(char);
  void    printSerial(int, int);
  void    printSerial(const char*);
  boolean processChar(char);
  boolean processCharWifi(char);
  void    processIncomingBoardMode(char);
  void    processIncomingSampleRate(char);
  void    processInsertMarker(char);
  void    processIncomingChannelSettings(char);
  void    processIncomingLeadOffSettings(char);
  void    reportDefaultChannelSettings(void);
  void    removeDaisy(void);
  void    resetADS(int);     // reset all the ADS1299's settings
  void    resetChannelSettingsArrayToDefault(byte channelSettingsArray[][OPENBCI_NUMBER_OF_CHANNEL_SETTINGS]);
  void    resetLeadOffArrayToDefault(byte leadOffArray[][OPENBCI_NUMBER_OF_LEAD_OFF_SETTINGS]);
  void    startADS(void);
  void    stopADS(void);
  void    sendChannelData(void);
  void    sendChannelData(PACKET_TYPE);
  void    setBoardMode(uint8_t);
  void    setChannelsToDefault(void);
  void    setCurPacketType(void);
  void    setSampleRate(uint8_t newSampleRateCode);
  void    sendEOT(void);
  void    setSerialInfo(SerialInfo, boolean, boolean, uint32_t);
  boolean smellDaisy(void);
  void    startMultiCharCmdTimer(char);
  void    streamSafeChannelDeactivate(byte);
  void    streamSafeChannelActivate(byte);
  void    streamSafeSetSampleRate(SAMPLE_RATE);
  void    streamSafeChannelSettingsForChannel(byte, byte, byte, byte, byte, byte, byte);
  void    streamSafeSetAllChannelsToDefault(void);
  void    streamSafeReportAllChannelDefaults(void);
  void    streamSafeLeadOffSetForChannel(byte, byte, byte);
  void    streamSafeTimeSendSyncSetPacket(void);
  void    streamStart(void);
  void    streamStop(void);
  void    tryMultiAbort(void);
  void    updateBoardData(void);
  void    updateBoardData(boolean);
  void    updateChannelData(void);   // retrieve data from ADS
  void    updateDaisyData(void);
  void    updateDaisyData(boolean);
  void    useAccel(boolean);
  void    useTimeStamp(boolean);
  void    write(uint8_t);
  void    writeAuxDataSerial(void);
  void    writeChannelSettings(void);
  void    writeChannelSettings(byte);
  void    writeSerial(uint8_t);
  void    writeSpi(uint8_t);
  void    writeTimeCurrent(void);
  void    writeTimeCurrentSerial(uint32_t newTime);
  void    writeZeroAux(void);
  void    zeroAuxData(void);

  // Variables
  boolean boardUseSRB1;  // keep track of whether we are using SRB1
  boolean daisyPresent;
  boolean daisyUseSRB1;
  boolean streaming;
  boolean useInBias[OPENBCI_NUMBER_OF_CHANNELS_DEFAULT]; // now size 4
  boolean useSRB2[OPENBCI_NUMBER_OF_CHANNELS_DEFAULT];
  boolean verbosity;  // turn on/off Serial verbosity

  byte boardChannelDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];     // raw channel data buffer
  byte channelSettings[OPENBCI_NUMBER_OF_CHANNELS_DAISY][OPENBCI_NUMBER_OF_CHANNEL_SETTINGS];  // current channel settings
  byte daisyChannelDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
  byte defaultChannelSettings[OPENBCI_NUMBER_OF_CHANNEL_SETTINGS];   // default channel settings
  byte lastBoardDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
  byte lastDaisyDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
  byte leadOffSettings[OPENBCI_NUMBER_OF_CHANNELS_DAISY][OPENBCI_NUMBER_OF_LEAD_OFF_SETTINGS];  // impedance measure settings
  byte meanBoardDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
  byte meanDaisyDataRaw[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
  byte sampleCounter;
  byte sampleCounterBLE;

  int ringBufBLEHead;
  int ringBufBLETail;
  int boardChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];    // channel data as ints
  int daisyChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];    // daisy channel data as ints
  int lastBoardChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];
  int lastDaisyChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];
  int meanBoardChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];
  int meanDaisyChannelDataInt[OPENBCI_NUMBER_CHANNELS_PER_ADS_SAMPLE];
  int numChannels;

  short auxData[3];   // auxiliary data
  short axisData[3];  // accelerometer data

  unsigned long lastSampleTime;

  volatile boolean channelDataAvailable;

  // ENUM STATES
  ACCEL_MODE     curAccelMode;
  BOARD_MODE     curBoardMode;
  DEBUG_MODE     curDebugMode;
  PACKET_TYPE    curPacketType;
  SAMPLE_RATE    curSampleRate;
  TIME_SYNC_MODE curTimeSyncMode;

  // Struct instances
  BLE bufferBLE[BLE_RING_BUFFER_SIZE];

  SerialInfo iSerial;
  SerialInfo iSerial1;

  // Class-level SPI will use Arduino’s SPI.transfer() instead of DSPI
  // (No DSPI0 spi; member here)

#ifdef __OpenBCI_Wifi_Master__
  void accelWriteAxisDataWifi(void);
  void ADS_writeChannelDataWifi(boolean daisy);
  void writeAuxDataWifi(void);
  void writeTimeCurrentWifi(uint32_t newTime);
#endif

private:
  byte    ADS_getDeviceID(int);
  void    boardBeginADSInterrupt(void);
  boolean boardBegin(void);
  boolean boardBeginDebug(void);
  boolean boardBeginDebug(int);
  void    changeInputType(byte);
  int     getX(void);
  int     getY(void);
  int     getZ(void);
  void    bufferBLEReset(void);
  void    bufferBLEReset(BLE*);
  void    initialize(void);
  void    initialize_accel(byte);
  void    initialize_ads(void);
  void    initializeSerialInfo(SerialInfo);
  void    initializeVariables(void);
  void    initializeSpiInfo(SpiInfo);
  byte    LIS3DH_getDeviceID(void);
  byte    LIS3DH_read(byte);
  int     LIS3DH_read16(byte);
  void    LIS3DH_write(byte, byte);
  boolean LIS3DH_DataReady(void);
  boolean LIS3DH_DataAvailable(void);
  void    LIS3DH_readAllRegs(void);
  void    LIS3DH_writeAxisDataSerial(void);
  void    LIS3DH_writeAxisDataForAxisSerial(uint8_t);
  void    LIS3DH_updateAxisData(void);
  void    LIS3DH_zeroAxisData(void);
  void    printADSregisters(int);
  void    printAllRegisters(void);
  void    printFailure(void);
  void    printHex(byte);
  void    printlnHex(byte);
  void    printRegisterName(byte);
  void    printSuccess(void);
  void    RDATA(int);
  void    RDATAC(int);
  void    RESET(int);
  byte    RREG(byte, int);
  void    RREGS(byte, byte, int);
  void    SDATAC(int);
  void    sendChannelDataSerial(PACKET_TYPE);
  void    sendChannelDataSerialBLE(PACKET_TYPE packetType);
  void    sendTimeWithAccelSerial(void);
  void    sendTimeWithRawAuxSerial(void);
  void    STANDBY(int);
  void    START(int);
  void    STOP(int);
  void    WAKEUP(int);
  void    WREG(byte, byte, int);
  void    WREGS(byte, byte, int);
  byte    xfer(byte);  // SPI transfer function (maps to SPI.transfer)

  // Variables
  boolean commandFromSPI;
  boolean firstDataPacket;
  boolean isMultiCharCmd;
  boolean isRunning;
  boolean settingBoardMode;
  boolean settingSampleRate;
  boolean newMarkerReceived;
  byte    regData[24];
  char    buffer[1];
  char    markerValue;
  char    multiCharCommand;
  char    currentChannelSetting;
  char    optionalArgBuffer5[5];
  char    optionalArgBuffer6[6];
  char    optionalArgBuffer7[7];
  int     boardStat;
  int     daisyStat;
  int     DRDYpinValue;
  int     lastDRDYpinValue;
  int     numberOfIncomingSettingsProcessedChannel;
  int     numberOfIncomingSettingsProcessedLeadOff;
  int     numberOfIncomingSettingsProcessedBoardType;
  uint8_t optionalArgCounter;
  unsigned long multiCharCmdTimeout;
  unsigned long timeOfLastRead;
  unsigned long timeOfMultiByteMsgStart;

#ifdef __OpenBCI_Wifi_Master__
  void LIS3DH_writeAxisDataWifi(void);
  void LIS3DH_writeAxisDataForAxisWifi(uint8_t);
  void sendChannelDataWifi(PACKET_TYPE, boolean);
  void sendRawAuxWifi(void);
  void sendTimeWithAccelWifi(void);
  void sendTimeWithRawAuxWifi(void);
#endif
};

// This allows the library to expose a single global instance
extern OpenBCI_32bit_Library board;

#endif
