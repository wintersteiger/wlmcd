// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SX1278_NORMAL_RT_
#define _SX1278_NORMAL_RT_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE(SX1278, NormalRegisterTable, uint8_t, uint8_t,
  REG(Fifo, "FIFO", 0x00, RW,                                                   "FIFO read/write access",);
  REG(OpMode, "OpMode", 0x01, RW,                                               "Operating mode & LoRaTM / FSK selection",
    VAR(OpMode, LongRangeMode, "Long range mode", 0x80, RW,                     "");
    VAR(OpMode, ModulationType, "ModulationType", 0x60, RW,                     "");
    VAR(OpMode, LowFrequencyModeOn, "Low Frequency Mode On", 0x08, RW,          "");
    VAR(OpMode, Mode, "Mode", 0x07, RW,                                         "");
  );
  REG(BitrateMsb, "Bitrate MSB", 0x02, RW,                                      "Bit Rate setting, Most Significant Bits",
    VAR(BitrateMsb, BitRate_15_8, "BitRate(15:8)", 0xFF, RW,                    "");
  );
  REG(BitrateLsb, "Bitrate LSB", 0x03, RW,                                      "Bit Rate setting, Least Significant Bits",
    VAR(BitrateLsb, BitRate_7_0, "BitRate(7:0)", 0xFF, RW,                      "");
  );
  REG(FdevMsb, "Deviation MSB", 0x04, RW,                                       "Frequency Deviation setting, Most Significant Bits",
    VAR(FdevMsb, Fdev_13_8, "Deviation(13:8)", 0x3F, RW,                        "");
  );
  REG(FdevLsb, "Deviation LSB", 0x05, RW,                                       "Frequency Deviation setting, Least Significant Bits",
    VAR(FdevLsb, Fdev_7_0, "Deviation(7:0)", 0xFF, RW,                          "");
  );
  REG(FrfMsb, "FrfMsb", 0x06, RW,                                               "RF Carrier Frequency, Most Significant Bits",
    VAR(FrfMsb, Frf_23_16, "Carrier freq (23:16)", 0xFF, RW,                    "");
  );
  REG(FrfMid, "FrfIsb", 0x07, RW,                                               "RF Carrier Frequency, Intermediate Bits",
    VAR(FrfMid, Frf_15_8, "Carrier freq (15:8)", 0xFF, RW,                      "");
  );
  REG(FrfLsb, "FrfLsb", 0x08, RW,                                               "RF Carrier Frequency, Least Significant Bits",
    VAR(FrfLsb, Frf_7_0, "Carrier freq (7:0)", 0xFF, RW,                        "");
  );
  REG(PaConfig, "PaConfig", 0x09, RW,                                           "PA selection and Output Power control",
    VAR(PaConfig, PaSelect, "PaSelect", 0x80, RW,                               "");
    VAR(PaConfig, MaxPower, "MaxPower", 0x70, RW,                               "");
    VAR(PaConfig, OutputPower, "OutputPower", 0x0F, RW,                         "");
  );
  REG(PaRamp, "PaRamp", 0x0A, RW,                                               "Control of PA ramp time, low phase noise PLL",
    VAR(PaRamp, ModulationShaping, "ModulationShaping", 0x60, RW,               "");
    VAR(PaRamp, PaRamp_, "PaRamp", 0x0F, RW,                                    "");
  );
  REG(Ocp, "", 0x0B, RW,                                                        "Over Current Protection control",
    VAR(Ocp, OcpOn, "OCP", 0x20, RW,                                            "");
    VAR(Ocp, OcpTrim, "OCP trim", 0x1F, RW,                                     "");
  );
  REG(Lna, "", 0x0C, RW,                                                        "LNA settings",
    VAR(Lna, LnaGain, "LnaGain", 0xE0, RW,                                      "");
    VAR(Lna, LnaBoostLf, "LnaBoostLf", 0x18, RW,                                "");
    VAR(Lna, LnaBoostHf, "LnaBoostHf", 0x03, RW,                                "");
  );
  REG(RxConfig, "", 0x0D, RW,                                                   "AFC, AGC, ctrl",
    VAR(RxConfig, RestartRxOnCollision, "RestartRxOnCollision", 0x80, RW,       "");
    VAR(RxConfig, RestartRxWithoutPllLock, "RestartRxWithoutPllLock", 0x40, RW, "");
    VAR(RxConfig, RestartRxWithPllLock, "RestartRxWithPllLock", 0x20, RW,       "");
    VAR(RxConfig, AfcAutoOn, "AfcAutoOn", 0x10, RW,                             "");
    VAR(RxConfig, AgcAutoOn, "AgcAutoOn", 0x08, RW,                             "");
    VAR(RxConfig, RxTrigger, "RxTrigger", 0x07, RW,                             "");
  );
  REG(RssiConfig, "", 0x0E, RW,                                                 "RSSI",
    VAR(RssiConfig, RssiOffset, "RssiOffset", 0xF8, RW,                         "");
    VAR(RssiConfig, RssiSmoothing, "RssiSmoothing", 0x07, RW,                   "");
  );
  REG(RssiCollision, "", 0x0F, RW,                                              "RSSI Collision detector",
    VAR(RssiCollision, RssiCollisionThreshold, "RSSI col T/H", 0xFF, RW,        "");
  );
  REG(RssiThresh, "", 0x10, RW,                                                 "RSSI Threshold control",
    VAR(RssiThresh, RssiThreshold, "RssiThreshold", 0xFF, RW,                   "");
  );
  REG(RssiValue, "", 0x11, RW,                                                  "RSSI value in dBm",
    VAR(RssiValue, RssiValue_, "RssiValue", 0xFF, RW,                           "");
  );
  REG(RxBw, "", 0x12, RW,                                                       "Channel Filter BW Control",
    VAR(RxBw, RxBwMant, "RxBwMant", 0x18, RW,                                   "");
    VAR(RxBw, RxBwExp, "RxBwExp", 0x07, RW,                                     "");
  );
  REG(AfcBw, "", 0x13, RW,                                                      "AFC Channel Filter BW",
    VAR(AfcBw, RxBwMantAfc, "RxBwMantAfc", 0x18, RW,                            "");
    VAR(AfcBw, RxBwExpAfc, "RxBwExpAfc", 0x07, RW,                              "");
  );
  REG(OokPeak, "", 0x14, RW,                                                    "OOK demodulator",
    VAR(OokPeak, BitSyncOn, "BitSyncOn", 0x20, RW,                              "");
    VAR(OokPeak, OokThreshType, "OokThreshType", 0x18, RW,                      "");
    VAR(OokPeak, OokPeakTheshStep, "OokPeakTheshStep", 0x07, RW,                "");
  );
  REG(OokFix, "", 0x15, RW,                                                     "Threshold of the OOK demod",
    VAR(OokFix, OokFixedThreshold, "OokFixedThreshold", 0xFF, RW,               "");
  );
  REG(OokAvg, "", 0x16, RW,                                                     "Average of the OOK demod",
    VAR(OokAvg, OokPeakThreshDec, "OokPeakThreshDec", 0xE0, RW,                 "");
    VAR(OokAvg, OokAverageOffset, "OokAverageOffset", 0xC0, RW,                 "");
    VAR(OokAvg, OokAverageThreshFilt, "OokAverageThreshFilt", 0x03, RW,         "");
  );
  REG(Reserved17, "Reserved17", 0x17, RW,                                       "", );
  REG(Reserved18, "Reserved18", 0x18, RW,                                       "", );
  REG(Reserved19, "Reserved19", 0x19, RW,                                       "", );
  REG(AfcFei, "", 0x1A, RW,                                                     "AFC and FEI control",
    VAR(AfcFei, AgcStart, "AgcStart", 0x10, RW,                                 "");
    VAR(AfcFei, AfcClear, "AfcClear", 0x02, RW,                                 "");
    VAR(AfcFei, AfcAutoClearOn, "AfcAutoClearOn", 0x01, RW,                     "");
  );
  REG(AfcMsb, "", 0x1B, RW,                                                     "Frequency correction value of the AFC MSB",
    VAR(AfcMsb, AfcValue_15_8, "AfcValue(15:8)", 0xFF, RW,                      "");
  );
  REG(AfcLsb, "", 0x1C, RW,                                                     "Frequency correction value of the AFC LSB",
    VAR(AfcLsb, AfcValue_7_0, "AfcValue(7:0)", 0xFF, RW,                        "");
  );
  REG(FeiMsb, "", 0x1D, RW,                                                     "Value of the calculated frequency error MSB",
    VAR(FeiMsb, FeiValue_15_8, "FeiValue(15:8)", 0xFF, RW,                      "");
  );
  REG(FeiLsb, "", 0x1E, RW,                                                     "Value of the calculated frequency error LSB",
    VAR(FeiLsb, FeiValue_7_0, "FeiValue(7:0)", 0xFF, RW,                        "");
  );
  REG(PreambleDetect, "", 0x1F, RW,                                             "Settings of the Preamble Detector",
    VAR(PreambleDetect, PreambleDetectorOn, "PreambleDetectorOn", 0x80, RW,     "");
    VAR(PreambleDetect, PreambleDetectorSize, "PreambleDetectorSize", 0x60, RW, "");
    VAR(PreambleDetect, PreambleDetectorTol, "PreambleDetectorTol", 0x1F, RW,   "");
  );
  REG(RxTimeout1, "", 0x20, RW,                                                 "Timeout Rx request and RSSI",
    VAR(RxTimeout1, TimeoutRxRssi, "TimeoutRxRssi", 0xFF, RW,                   "");
  );
  REG(RxTimeout2, "", 0x21, RW,                                                 "Timeout RSSI and PayloadReady",
    VAR(RxTimeout2, TimeoutRxPreamble, "TimeoutRxPreamble", 0xFF, RW,           "");
  );
  REG(RxTimeout3, "", 0x22, RW,                                                 "Timeout RSSI and SyncAddress",
    VAR(RxTimeout3, TimeoutSignalSync, "TimeoutSignalSync", 0xFF, RW,           "");
  );
  REG(RxDelay, "", 0x23, RW,                                                    "Delay between Rx cycles",
    VAR(RxDelay, InterPacketRxDelay, "InterPacketRxDelay", 0xFF, RW,            "");
  );
  REG(Osc, "", 0x24, RW,                                                        "RC Oscillators Settings, CLKOUT frequency",
    VAR(Osc, RcCalStart, "RcCalStart", 0x08, RW,                                "");
    VAR(Osc, ClkOut, "ClkOut", 0x07, RW,                                        "");
  );
  REG(PreambleMsb, "", 0x25, RW,                                                "Preamble length, MSB",
    VAR(PreambleMsb, PreambleSize_15_8, "PreambleSize(15:8)", 0xFF, RW,         "");
  );
  REG(PreambleLsb, "", 0x26, RW,                                                "Preamble length, LSB",
    VAR(PreambleLsb, PreambleSize_7_0, "PreambleSize(7:0)", 0xFF, RW,           "");
  );
  REG(SyncConfig, "", 0x27, RW,                                                 "Sync Word Recognition control",
    VAR(SyncConfig, AutoRestartRxMode, "AutoRestartRxMode", 0xC0, RW,           "");
    VAR(SyncConfig, PreamblePolarity, "PreamblePolarity", 0x20, RW,             "");
    VAR(SyncConfig, SyncOn, "SyncOn", 0x10, RW,                                 "");
    VAR(SyncConfig, SyncSize, "SyncSize", 0x07, RW,                             "");
  );
  REG(SyncValue1, "", 0x28, RW,                                                 "Sync Word bytes 1",
    VAR(SyncValue1, SyncValue_63_56, "SyncValue(63:56)", 0xFF, RW,              "");
  );
  REG(SyncValue2, "", 0x29, RW,                                                 "Sync Word bytes 2",
    VAR(SyncValue2, SyncValue_55_48, "SyncValue(55:48)", 0xFF, RW,              "");
  );
  REG(SyncValue3, "", 0x2A, RW,                                                 "Sync Word bytes 3",
    VAR(SyncValue3, SyncValue_47_40, "SyncValue(47:40)", 0xFF, RW,              "");
  );
  REG(SyncValue4, "", 0x2B, RW,                                                 "Sync Word bytes 4",
    VAR(SyncValue4, SyncValue_39_32, "SyncValue(39:32)", 0xFF, RW,              "");
  );
  REG(SyncValue5, "", 0x2C, RW,                                                 "Sync Word bytes 5",
    VAR(SyncValue5, SyncValue_31_24, "SyncValue(31:24)", 0xFF, RW,              "");
  );
  REG(SyncValue6, "", 0x2D, RW,                                                 "Sync Word bytes 6",
    VAR(SyncValue6, SyncValue_23_16, "SyncValue(23:16)", 0xFF, RW,              "");
  );
  REG(SyncValue7, "", 0x2E, RW,                                                 "Sync Word bytes 7",
    VAR(SyncValue7, SyncValue_15_8, "SyncValue(15:8)", 0xFF, RW,                "");
  );
  REG(SyncValue8, "", 0x2F, RW,                                                 "Sync Word byte 8",
    VAR(SyncValue8, SyncValue_7_0, "SyncValue(7:0)", 0xFF, RW,                  "");
  );
  REG(PacketConfig1, "", 0x30, RW,                                              "Packet mode settings (1)",
    VAR(PacketConfig1, PacketFormat, "PacketFormat", 0x07, RW,                  "");
    VAR(PacketConfig1, DcFree, "DcFree", 0x60, RW,                              "");
    VAR(PacketConfig1, CrcOn, "CrcOn", 0x10, RW,                                "");
    VAR(PacketConfig1, CrcAutoClearOff, "CrcAutoClearOff", 0x08, RW,            "");
    VAR(PacketConfig1, AddressFiltering, "AddressFiltering", 0x06, RW,          "");
    VAR(PacketConfig1, CrcWhiteningType, "CrcWhiteningType", 0x01, RW,          "");
  );
  REG(PacketConfig2, "", 0x31, RW,                                              "Packet mode settings (2)",
    VAR(PacketConfig2, DataMode, "DataMode", 0x40, RW,                          "");
    VAR(PacketConfig2, DcFreIoHomeOne, "IoHomeOn", 0x20, RW,                    "");
    VAR(PacketConfig2, IoHomePowerFrame, "IoHomePowerFrame", 0x10, RW,          "");
    VAR(PacketConfig2, BeaconOn, "BeaconOn", 0x08, RW,                          "");
    VAR(PacketConfig2, PayloadLength_10_8, "PayloadLength(10:8)", 0x07, RW,     "");
  );
  REG(PayloadLength, "", 0x32, RW,                                              "Payload length setting",
    VAR(PayloadLength, PayloadLength_7_0, "PayloadLength(7:0)", 0xFF, RW,       "");
  );
  REG(NodeAdrs, "", 0x33, RW,                                                   "Node address",
    VAR(NodeAdrs, NodeAddress, "NodeAddress", 0xFF, RW,                         "");
  );
  REG(BroadcastAdrs, "", 0x34, RW,                                              "Broadcast address",
    VAR(BroadcastAdrs, BroadcastAddress, "BroadcastAddress", 0xFF, RW,          "");
  );
  REG(FifoThresh, "", 0x35, RW,                                                 "Fifo threshold, Tx start condition",
    VAR(FifoThresh, TxStartCondition, "TxStartCondition", 0x80, RW,             "");
    VAR(FifoThresh, FifoThreshold, "FifoThreshold", 0x3F, RW,                   "");
  );
  REG(SeqConfig1, "", 0x36, RW,                                                 "Top level Sequencer settings (1)",
    VAR(SeqConfig1, SequencerStart, "SequencerStart", 0x80, RW,                 "");
    VAR(SeqConfig1, SequencerStop, "SequencerStop", 0x40, RW,                   "");
    VAR(SeqConfig1, IdleMode, "IdleMode", 0x20, RW,                             "");
    VAR(SeqConfig1, FromStart, "FromStart", 0x18, RW,                           "");
    VAR(SeqConfig1, LowPowerSelection, "LowPowerSelection", 0x04, RW,           "");
    VAR(SeqConfig1, FromIdle, "FromIdle", 0x02, RW,                             "");
    VAR(SeqConfig1, FromTransmit, "FromTransmit", 0x01, RW,                     "");
  );
  REG(SeqConfig2, "", 0x37, RW,                                                 "Top level Sequencer settings (2)",
    VAR(SeqConfig2, FromReceive, "FromReceive", 0xE0, RW,                       "");
    VAR(SeqConfig2, FromRxTimeout, "FromRxTimeout", 0x18, RW,                   "");
    VAR(SeqConfig2, FromPacketReceived, "FromPacketReceived", 0x07, RW,         "");
  );
  REG(TimerResol, "", 0x38, RW,                                                 "Timer 1 and 2 resolution control",
    VAR(TimerResol, Timer1Resolution, "Timer1Resolution", 0xC0, RW,             "");
    VAR(TimerResol, Timer2Resolution, "Timer2Resolution", 0x03, RW,             "");
  );
  REG(Timer1Coef, "", 0x39, RW,                                                 "Timer 1 setting",
    VAR(Timer1Coef, Timer1Coefficient, "Timer1Coefficient", 0xFF, RW,           "");
  );
  REG(Timer2Coef, "", 0x3A, RW,                                                 "Timer 2 setting",
    VAR(Timer2Coef, Timer2Coefficient, "Timer2Coefficient", 0xFF, RW,           "");
  );
  REG(ImageCal, "", 0x3B, RW,                                                   "Image calibration engine control",
    VAR(ImageCal, AutoImageCalOn, "AutoImageCalOn", 0x80, RW,                   "");
    VAR(ImageCal, ImageCalStart, "ImageCalStart", 0x40, RW,                     "");
    VAR(ImageCal, ImageCalRunning, "ImageCalRunning", 0x20, RW,                 "");
    VAR(ImageCal, TempChange, "TempChange", 0x08, RW,                           "");
    VAR(ImageCal, TempThreshold, "TempThreshold", 0x06, RW,                     "");
    VAR(ImageCal, TempMonitorOff, "TempMonitorOff", 0x01, RW,                   "");
  );
  REG(Temp, "", 0x3C, RW,                                                       "Temperature Sensor value",
    VAR(Temp, TempValue, "TempValue", 0xFF, RW,                                 "");
  );
  REG(LowBat, "", 0x3D, RW,                                                     "Low Battery Indicator Settings",
    VAR(LowBat, LowBatOn, "LowBatOn", 0x08, RW,                                 "");
    VAR(LowBat, LowBatTrim, "LowBatTrim", 0x07, RW,                             "");
  );


  // Status Registers

  REG(IrqFlags1, "", 0x3E, RW,                                                  "Status register: PLL Lock state, Timeout, RSSI",
    VAR(IrqFlags1, ModeReady, "ModeReady", 0x80, RW,                            "");
    VAR(IrqFlags1, RxReady, "RxReady", 0x40, RW,                                "");
    VAR(IrqFlags1, TxReady, "TxReady", 0x20, RW,                                "");
    VAR(IrqFlags1, PllLock, "PllLock", 0x10, RW,                                "");
    VAR(IrqFlags1, Rssi, "Rssi", 0x08, RW,                                      "");
    VAR(IrqFlags1, Timeout, "Timeout", 0x04, RW,                                "");
    VAR(IrqFlags1, PreambleDetect_, "PreambleDetect", 0x02, RW,                 "");
    VAR(IrqFlags1, SyncAddressMatch, "SyncAddressMatch", 0x01, RW,              "");
  );
  REG(IrqFlags2, "", 0x3F, RW,                                                  "Status register: FIFO handling flags, Low Battery",
    VAR(IrqFlags2, FifoFull, "FifoFull", 0x80, RW,                              "");
    VAR(IrqFlags2, FifoEmpty, "FifoEmpty", 0x40, RW,                            "");
    VAR(IrqFlags2, FifoLevel, "FifoLevel", 0x20, RW,                            "");
    VAR(IrqFlags2, FifoOverrun, "FifoOverrun", 0x10, RW,                        "");
    VAR(IrqFlags2, PacketSent, "PacketSent", 0x08, RW,                          "");
    VAR(IrqFlags2, PayloadReady, "PayloadReady", 0x04, RW,                      "");
    VAR(IrqFlags2, CrcOk, "CrcOk", 0x02, RW,                                    "");
    VAR(IrqFlags2, LowBat_, "LowBat", 0x01, RW,                                 "");
  );


  // I/O Control Registers

  REG(DioMapping1, "", 0x40, RW,                                                "Mapping of pins DIO0 to DIO3",
    VAR(DioMapping1, Dio0Mapping, "Dio0Mapping", 0xC0, RW,                      "");
    VAR(DioMapping1, Dio1Mapping, "Dio1Mapping", 0x30, RW,                      "");
    VAR(DioMapping1, Dio2Mapping, "Dio2Mapping", 0x0C, RW,                      "");
    VAR(DioMapping1, Dio3Mapping, "Dio3Mapping", 0x03, RW,                      "");
  );
  REG(DioMapping2, "", 0x41, RW,                                                "Mapping of pins DIO4 and DIO5, ClkOut frequency",
    VAR(DioMapping2, Dio4Mapping, "Dio4Mapping", 0xC0, RW,                      "");
    VAR(DioMapping2, Dio5Mapping, "Dio5Mapping", 0x30, RW,                      "");
    VAR(DioMapping2, MapPreambleDetect, "MapPreambleDetect", 0x01, RW,          "");
  );


  // Version Register

  REG(Version, "", 0x42, RW,                                                    "Semtech ID relating the silicon revision", );


  // Additional registers

  REG(PllHop, "", 0x44, RW,                                                     "Control the fast frequency hopping mode",
    VAR(PllHop, FastHopOn, "FastHopOn", 0x80, RW,                               "");
  );
  REG(Tcxo, "", 0x4B, RW,                                                       "TCXO or XTAL input setting",
    VAR(Tcxo, TcxoInputOn, "TcxoInputOn", 0x10, RW,                             "");
  );
  REG(PaDac, "", 0x4D, RW,                                                      "Higher power settings of the PA",
    VAR(PaDac, PaDac_, "PaDac", 0x07, RW,                                       "");
  );
  REG(FormerTemp, "", 0x5B, RW,                                                 "Stored temperature during the former IQ Calibration",
    VAR(FormerTemp, FormerTemp_, "FormerTemp", 0xFF, RW,                        "");
  );
  REG(BitRateFrac, "", 0x5D, RW,                                                "Fractional part in the Bit Rate division ratio",
    VAR(BitRateFrac, BitRateFrac_, "BitRateFrac", 0x0F, RW,                     "");
  );
  REG(AgcRef, "", 0x61, RW,                                                     "Adjustment of the AGC reference thresholds",
    VAR(AgcRef, AgcReferenceLevel, "AgcReferenceLevel", 0x3F, RW,               "");
  );
  REG(AgcThresh1, "", 0x62, RW,                                                 "Adjustment of the AGC thresholds (1)",
    VAR(AgcThresh1, AgcStep1, "AgcStep1", 0x1F, RW,                             "");
  );
  REG(AgcThresh2, "", 0x63, RW,                                                 "Adjustment of the AGC thresholds (2)",
    VAR(AgcThresh2, AgcStep2, "AgcStep2", 0xF0, RW,                             "");
    VAR(AgcThresh2, AgcStep3, "AgcStep3", 0x0F, RW,                             "");
  );
  REG(AgcThresh3, "", 0x64, RW,                                                 "Adjustment of the AGC thresholds (3)",
    VAR(AgcThresh3, AgcStep4, "AgcStep4", 0xF0, RW,                             "");
    VAR(AgcThresh3, AgcStep5, "AgcStep5", 0x0F, RW,                             "");
  );
  REG(Pll, "", 0x70, RW,                                                        "Control of the PLL bandwidth",
    VAR(Pll, PllBandwidth, "PllBandwidth", 0xC0, RW,                            "");
  );

  // Add high-frequency registers even though they are the same?

  // OTHERS: Internal test registers. Do not overwrite.
  // e.g. 0x5C
);

#undef VAR
#undef REG

#endif
