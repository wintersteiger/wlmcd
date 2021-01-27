// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RFM69_NORMAL_RT_
#define _RFM69_NORMAL_RT_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE_W(RFM69, RegisterTable, uint8_t, uint8_t,

  REG(Fifo, "Fifo", 0x00, RW, "FIFO read/write access", );
  REG(OpMode, "OpMode", 0x01, RW, "Operating modes of the transceiver",
    VAR(OpMode, SequencerOff, "SequencerOff", 0x80, RW,                       "");
    VAR(OpMode, ListenOn, "ListenOn", 0x40, RW,                               "");
    VAR(OpMode, ListenAbort, "ListenAbort", 0x20, WO,                         "");
    VAR(OpMode, Mode, "Mode", 0x1C, RW,                                       "");
  );
  REG(DataModul, "DataModul", 0x02, RW, "Data operation mode and Modulation settings",
    VAR(DataModul, DataMode, "DataMode", 0x60, RW,                            "");
    VAR(DataModul, ModulationType, "ModulationType", 0x18, RW,                "");
    VAR(DataModul, ModulationShaping, "ModulationShaping", 0x03, RW,          "");
  );
  REG(BitrateMsb, "BitrateMsb", 0x03, RW, "Bit Rate setting, Most Significant Bits",
    VAR(BitrateMsb, BitRate_15_8, "BitRate(15:8)", 0xFF, RW,                  "");
  );
  REG(BitrateLsb, "BitrateLsb", 0x04, RW, "Bit Rate setting, Least Significant Bits",
    VAR(BitrateLsb, BitRate_7_0, "BitRate(7:0)", 0xFF, RW,                    "");
  );
  REG(FdevMsb, "FdevMsb", 0x05, RW, "Frequency Deviation setting, Most Significant Bits",
    VAR(FdevMsb, Fdev_13_8, "Fdev(13:8)", 0x3F, RW,                           "");
  );
  REG(FdevLsb, "FdevLsb", 0x06, RW, "Frequency Deviation setting, Least Significant Bits",
    VAR(FdevLsb, Fdev_7_0, "Fdev(7:0)", 0xFF, RW,                             "");
  );
  REG(FrfMsb, "FrfMsb", 0x07, RW, "RF Carrier Frequency, Most Significant Bits",
    VAR(FrfMsb, Frf_23_16, "Frf(23:16)", 0xFF, RW,                            "");
  );
  REG(FrfMid, "FrfMid", 0x08, RW, "RF Carrier Frequency, Intermediate Bits",
    VAR(FrfMid, Frf_15_8, "Frf(15:8)", 0xFF, RW,                              "");
  );
  REG(FrfLsb, "FrfLsb", 0x09, RW, "RF Carrier Frequency, Least Significant Bits",
    VAR(FrfLsb, Frf_7_0, "Frf(7:0)", 0xFF, RW,                                "");
  );
  REG(Osc1, "Osc1", 0x0A, RW, "RC Oscillators Settings",
    VAR(Osc1, RcCalStart, "RcCalStart", 0x80, WO,                             "");
    VAR(Osc1, RcCalDone, "RcCalDone", 0x40, RO,                               "");
  );
  REG(AfcCtrl, "AfcCtrl", 0x0B, RW, "AFC control in low modulation index situations",
    VAR(AfcCtrl, AfcLowBetaOn, "AfcLowBetaOn", 0x20, RW,                      "");
  );
  REG(Reserved0C, "Reserved0C", 0x0C, RO, "-", );
  REG(Listen1, "Listen1", 0x0D, RW, "Listen Mode settings",
    VAR(Listen1, ListenResolIdle, "ListenResolIdle", 0xC0, RW,                "");
    VAR(Listen1, ListenResolRx, "ListenResolRx", 0x30, RW,                    "");
    VAR(Listen1, ListenCriteria, "ListenCriteria", 0x10, RW,                  "");
    VAR(Listen1, ListenEnd, "ListenEnd", 0x03, RW,                            "");
  );
  REG(Listen2, "Listen2", 0x0E, RW, "Listen Mode Idle duration",
    VAR(Listen2, ListenCoefIdle, "ListenCoefIdle", 0xFF, RW,                  "");
  );
  REG(Listen3, "Listen3", 0x0F, RW, "Listen Mode Rx duration",
    VAR(Listen3, ListenCoefRx, "ListenCoefRx", 0xFF, RW,                      "");
  );
  REG(Version, "Version", 0x10, RO,                                           "Version code of the chip. Bits 7-4 give the full revision "
                                                                              "number; bits 3-0 give the metal mask revision number.", );
  REG(PaLevel, "PaLevel", 0x11, RW, "PA selection and Output Power control",
    VAR(PaLevel, Pa0On, "Pa0On", 0x80, RW,                                    "");
    VAR(PaLevel, Pa1On, "Pa1On", 0x40, RW,                                    "");
    VAR(PaLevel, Pa2On, "Pa2On", 0x20, RW,                                    "");
    VAR(PaLevel, OutputPower, "OutputPower", 0x1F, RW,                        "");
  );
  REG(PaRamp, "PaRamp", 0x12, RW, "Control of the PA ramp time in FSK mode",
    VAR(PaRamp, PaRamp_, "PaRamp", 0x0F, RW,                                  "");
  );
  REG(Ocp, "Ocp", 0x13, RW, "Over Current Protection control",
    VAR(Ocp, OcpOn, "OcpOn", 0x10, RW,                                        "");
    VAR(Ocp, OcpTrim, "OcpTrim", 0x0F, RW,                                    "");
  );
  REG(Reserved14, "Reserved14", 0x14, RO, "-", );
  REG(Reserved15, "Reserved15", 0x15, RO, "-", );
  REG(Reserved16, "Reserved16", 0x16, RO, "-", );
  REG(Reserved17, "Reserved17", 0x17, RO, "-", );
  REG(Lna, "Lna", 0x18, RW, "LNA settings",
    VAR(Lna, LnaZin, "LnaZin", 0x80, RW,                                      "");
    VAR(Lna, LnaCurrentGain, "LnaCurrentGain", 0x38, RO,                      "");
    VAR(Lna, LnaGainSelect, "LnaGainSelect", 0x07, RW,                        "");
  );
  REG(RxBw, "RxBw", 0x19, RW, "Channel Filter BW Control",
    VAR(RxBw, DccFreq, "DccFreq", 0xE0, RW,                                   "");
    VAR(RxBw, RxBwMant, "RxBwMant", 0x18, RW,                                 "");
    VAR(RxBw, RxBwExp, "RxBwExp", 0x07, RW,                                   "");
  );
  REG(AfcBw, "AfcBw", 0x1A, RW, "Channel Filter BW control during the AFC routine",
    VAR(AfcBw, DccFreqAfc, "DccFreqAfc", 0xE0, RW,                            "");
    VAR(AfcBw, RxBwMantAfc, "RxBwMantAfc", 0x18, RW,                          "");
    VAR(AfcBw, RxBwExpAfc, "RxBwExpAfc", 0x07, RW,                            "");
  );
  REG(OokPeak, "OokPeak", 0x1B, RW, "OOK demodulator selection and control in peak mode",
    VAR(OokPeak, OokThreshType, "OokThreshType", 0xC0, RW,                    "");
    VAR(OokPeak, OokPeakTheshStep, "OokPeakTheshStep", 0x38, RW,              "");
    VAR(OokPeak, OokPeakThreshDec, "OokPeakThreshDec", 0x07, RW,              "");
  );
  REG(OokAvg, "OokAvg", 0x1C, RW, "Average threshold control of the OOK demodulator",
    VAR(OokAvg, OokAverageThreshFilt, "OokAverageThreshFilt", 0xC0, RW,       "");
  );
  REG(OokFix, "OokFix", 0x1D, RW, "Fixed threshold control of the OOK demodulator",
    VAR(OokFix, OokFixedThresh, "OokFixedThresh", 0xFF, RW,                   "");
  );
  REG(AfcFei, "AfcFei", 0x1E, RW, "AFC and FEI control and status",
    VAR(AfcFei, FeiDone, "FeiDone", 0x40, RW,                                 "");
    VAR(AfcFei, FeiStart, "FeiStart", 0x20, RW,                               "");
    VAR(AfcFei, AfcDone, "AfcDone", 0x10, RW,                                 "");
    VAR(AfcFei, AfcAutoclearOn, "AfcAutoclearOn", 0x08, RW,                   "");
    VAR(AfcFei, AfcAutoOn, "AfcAutoOn", 0x04, RW,                             "");
    VAR(AfcFei, AfcClear, "AfcClear", 0x02, RW,                               "");
    VAR(AfcFei, AfcStart, "AfcStart", 0x01, RW,                               "");
  );
  REG(AfcMsb, "AfcMsb", 0x1F, RW, "MSB of the frequency correction of the AFC",
    VAR(AfcMsb, AfcValue_15_8, "AfcValue[15:8]", 0xFF, RO,                    "");
  );
  REG(AfcLsb, "AfcLsb", 0x20, RW, "LSB of the frequency correction of the AFC",
    VAR(AfcLsb, AfcValue_7_0, "AfcValue[7:0]", 0xFF, RO,                      "");
  );
  REG(FeiMsb, "FeiMsb", 0x21, RW, "MSB of the calculated frequency error",
    VAR(FeiMsb, FeiValue_15_8, "FeiValue[15:8]", 0xFF, RO,                    "");
  );
  REG(FeiLsb, "FeiLsb", 0x22, RW, "LSB of the calculated frequency error",
    VAR(FeiLsb, FeiValue_7_0, "FeiValue[7:0]", 0xFF, RO,                      "");
  );
  REG(RssiConfig, "RssiConfig", 0x23, RW, "RSSI-related settings",
    VAR(RssiConfig, RssiDone, "RssiDone", 0x02, RO,                           "");
    VAR(RssiConfig, RssiStart, "RssiStart", 0x01, WO,                         "");
  );
  REG(RssiValue, "RssiValue", 0x24, RW, "RSSI value in dBm", );
  REG(DioMapping1, "DioMapping1", 0x25, RW, "Mapping of pins DIO0 to DIO3",
    VAR(DioMapping1, Dio0Mapping, "Dio0Mapping", 0xC0, RW,                    "");
    VAR(DioMapping1, Dio1Mapping, "Dio1Mapping", 0x30, RW,                    "");
    VAR(DioMapping1, Dio2Mapping, "Dio2Mapping", 0x0C, RW,                    "");
    VAR(DioMapping1, Dio3Mapping, "Dio3Mapping", 0x03, RW,                    "");
  );
  REG(DioMapping2, "DioMapping2", 0x26, RW, "Mapping of pins DIO4 and DIO5, ClkOut frequency",
    VAR(DioMapping2, Dio4Mapping, "Dio4Mapping", 0xC0, RW,                    "");
    VAR(DioMapping2, Dio5Mapping, "Dio5Mapping", 0x30, RW,                    "");
    VAR(DioMapping2, ClkOut, "ClkOut", 0x07, RW,                              "");
  );
  REG(IrqFlags1, "", 0x27, RW,                                                  "Status register: PLL Lock state, Timeout, RSSI > Threshold...",
    VAR(IrqFlags1, ModeReady, "ModeReady", 0x80, RW,                            "");
    VAR(IrqFlags1, RxReady, "RxReady", 0x40, RW,                                "");
    VAR(IrqFlags1, TxReady, "TxReady", 0x20, RW,                                "");
    VAR(IrqFlags1, PllLock, "PllLock", 0x10, RW,                                "");
    VAR(IrqFlags1, Rssi, "Rssi", 0x08, RW,                                      "");
    VAR(IrqFlags1, Timeout, "Timeout", 0x04, RW,                                "");
    VAR(IrqFlags1, AutoMode, "AutoMode", 0x02, RW,                              "");
    VAR(IrqFlags1, SyncAddressMatch, "SyncAddressMatch", 0x01, RW,              "");
  );
  REG(IrqFlags2, "", 0x28, RW,                                                  "Status register: FIFO handling flags...",
    VAR(IrqFlags2, FifoFull, "FifoFull", 0x80, RW,                              "");
    VAR(IrqFlags2, FifoEmpty, "FifoEmpty", 0x40, RW,                            "");
    VAR(IrqFlags2, FifoLevel, "FifoLevel", 0x20, RW,                            "");
    VAR(IrqFlags2, FifoOverrun, "FifoOverrun", 0x10, RW,                        "");
    VAR(IrqFlags2, PacketSent, "PacketSent", 0x08, RW,                          "");
    VAR(IrqFlags2, PayloadReady, "PayloadReady", 0x04, RW,                      "");
    VAR(IrqFlags2, CrcOk, "CrcOk", 0x02, RW,                                    "");
  );
  REG(RssiThresh, "RssiThresh", 0x29, RW, "RSSI Threshold control",
    VAR(RssiThresh, RssiThreshold, "RssiThreshold", 0xFF, RW,                   "");
  );
  REG(RxTimeout1, "RxTimeout1", 0x2A, RW, "Timeout duration between Rx request and RSSI detection",
    VAR(RxTimeout1, TimeoutRxStart, "TimeoutRxStart", 0xFF, RW,                 "");
  );
  REG(RxTimeout2, "RxTimeout2", 0x2B, RW, "Timeout duration between RSSI detection and PayloadReady",
    VAR(RxTimeout2, TimeoutRssiThresh, "TimeoutRssiThresh", 0xFF, RW,           "");
  );
  REG(PreambleMsb, "PreambleMsb", 0x2C, RW, "Preamble length, MSB",
    VAR(PreambleMsb, PreambleSize_15_8, "PreambleSize[15:8]", 0xFF, RW,         "");
  );
  REG(PreambleLsb, "PreambleLsb", 0x2D, RW, "Preamble length, LSB",
    VAR(PreambleLsb, PreambleSize_7_0, "PreambleSize[7:0]", 0xFF, RW,           "");
  );
  REG(SyncConfig, "SyncConfig", 0x2E, RW, "Sync Word Recognition control",
    VAR(SyncConfig, SyncOn, "SyncOn", 0x80, RW,                                 "");
    VAR(SyncConfig, FifoFillCondition, "FifoFillCondition", 0x40, RW,           "");
    VAR(SyncConfig, SyncSize, "SyncSize", 0x38, RW,                             "");
    VAR(SyncConfig, SyncTol, "SyncTol", 0x07, RW,                               "");
  );
  REG(SyncValue1, "", 0x2F, RW,                                                 "1st byte of Sync word. (MSB byte).",
    VAR(SyncValue1, SyncValue_63_56, "SyncValue(63:56)", 0xFF, RW,              "");
  );
  REG(SyncValue2, "", 0x30, RW,                                                 "2nd byte of Sync word.",
    VAR(SyncValue2, SyncValue_55_48, "SyncValue(55:48)", 0xFF, RW,              "");
  );
  REG(SyncValue3, "", 0x31, RW,                                                 "3rd byte of Sync word.",
    VAR(SyncValue3, SyncValue_47_40, "SyncValue(47:40)", 0xFF, RW,              "");
  );
  REG(SyncValue4, "", 0x32, RW,                                                 "4th byte of Sync word.",
    VAR(SyncValue4, SyncValue_39_32, "SyncValue(39:32)", 0xFF, RW,              "");
  );
  REG(SyncValue5, "", 0x33, RW,                                                 "5th byte of Sync word.",
    VAR(SyncValue5, SyncValue_31_24, "SyncValue(31:24)", 0xFF, RW,              "");
  );
  REG(SyncValue6, "", 0x34, RW,                                                 "6th byte of Sync word.",
    VAR(SyncValue6, SyncValue_23_16, "SyncValue(23:16)", 0xFF, RW,              "");
  );
  REG(SyncValue7, "", 0x35, RW,                                                 "7th byte of Sync word.",
    VAR(SyncValue7, SyncValue_15_8, "SyncValue(15:8)", 0xFF, RW,                "");
  );
  REG(SyncValue8, "", 0x36, RW,                                                 "8th byte of Sync word.",
    VAR(SyncValue8, SyncValue_7_0, "SyncValue(7:0)", 0xFF, RW,                  "");
  );
  REG(PacketConfig1, "PacketConfig1", 0x37, RW, "Packet mode settings",
    VAR(PacketConfig1, PacketFormat, "PacketFormat", 0x80, RW,                  "");
    VAR(PacketConfig1, DcFree, "DcFree", 0x60, RW,                              "");
    VAR(PacketConfig1, CrcOn, "CrcOn", 0x10, RW,                                "");
    VAR(PacketConfig1, CrcAutoClearOff, "CrcAutoClearOff", 0x08, RW,            "");
    VAR(PacketConfig1, AddressFiltering, "AddressFiltering", 0x06, RW,          "");
  );
  REG(PayloadLength, "PayloadLength", 0x38, RW, "Payload length setting", );
  REG(NodeAdrs, "NodeAdrs", 0x39, RW, "Node address", );
  REG(BroadcastAdrs, "BroadcastAdrs", 0x3A, RW, "Broadcast address", );
  REG(AutoModes, "AutoModes", 0x3B, RW, "Auto modes settings",
    VAR(AutoModes, EnterCondition, "EnterCondition", 0xE0, RW,                  "");
    VAR(AutoModes, ExitCondition, "ExitCondition", 0x1C, RW,                    "");
    VAR(AutoModes, IntermediateMode, "IntermediateMode", 0x03, RW,              "");
  );
  REG(FifoThresh, "FifoThresh", 0x3C, RW, "Fifo threshold, Tx start condition",
    VAR(FifoThresh, TxStartCondition, "TxStartCondition", 0x80, RW,             "");
    VAR(FifoThresh, FifoThreshold, "FifoThreshold", 0x7F, RW,                   "");
  );
  REG(PacketConfig2, "PacketConfig2", 0x3D, RW, "Packet mode settings",
    VAR(PacketConfig2, InterPacketRxDelay, "InterPacketRxDelay", 0xF0, RW,      "");
    VAR(PacketConfig2, RestartRx, "RestartRx", 0x04, RW,                        "");
    VAR(PacketConfig2, AutoRxRestartOn, "AutoRxRestartOn", 0x02, RW,            "");
    VAR(PacketConfig2, AesOn, "AesOn", 0x01, RW,                                "");
  );
  REG(AesKey1, "AesKey1", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey1, AesKey_127_120, "AesKey(127:120)", 0xFF, WO,                   "1st byte of cipher key (MSB byte)");
  );
  REG(AesKey2, "AesKey2", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey2, AesKey_119_112, "AesKey(119:112)", 0xFF, WO,                   "2nd byte of cipher key");
  );
  REG(AesKey3, "AesKey3", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey3, AesKey_111_104, "AesKey(111:104)", 0xFF, WO,                   "3rd byte of cipher key");
  );
  REG(AesKey4, "AesKey4", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey4, AesKey_103_96, "AesKey(103:96)", 0xFF, WO,                     "4th byte of cipher key");
  );
  REG(AesKey5, "AesKey5", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey5, AesKey_95_88, "AesKey(95:88)", 0xFF, WO,                       "5th byte of cipher key");
  );
  REG(AesKey6, "AesKey6", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey6, AesKey_87_80, "AesKey(87:80)", 0xFF, WO,                       "6th byte of cipher key");
  );
  REG(AesKey7, "AesKey7", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey7, AesKey_79_72, "AesKey(79:72)", 0xFF, WO,                       "7th byte of cipher key");
  );
  REG(AesKey8, "AesKey8", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey8, AesKey_71_64, "AesKey(71:64)", 0xFF, WO,                       "8th byte of cipher key");
  );
  REG(AesKey9, "AesKey9", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey9, AesKey_63_56, "AesKey(63:56)", 0xFF, WO,                       "9th byte of cipher key");
  );
  REG(AesKey10, "AesKey10", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey10, AesKey_55_48, "AesKey(55:48)", 0xFF, WO,                      "10th byte of cipher key");
  );
  REG(AesKey11, "AesKey11", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey11, AesKey_47_40, "AesKey(47:40)", 0xFF, WO,                      "11th byte of cipher key");
  );
  REG(AesKey12, "AesKey12", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey12, AesKey_39_32, "AesKey(39:32)", 0xFF, WO,                      "12th byte of cipher key");
  );
  REG(AesKey13, "AesKey13", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey13, AesKey_31_24, "AesKey(31:24)", 0xFF, WO,                      "13th byte of cipher key");
  );
  REG(AesKey14, "AesKey14", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey14, AesKey_23_16, "AesKey(23:16)", 0xFF, WO,                      "14th byte of cipher key");
  );
  REG(AesKey15, "AesKey15", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey15, AesKey_15_8, "AesKey(15:8)", 0xFF, WO,                        "15th byte of cipher key");
  );
  REG(AesKey16, "AesKey16", 0x3E, RW, "16 bytes of the cypher key",
    VAR(AesKey16, AesKey_7_0, "AesKey(7:0)", 0xFF, WO,                          "16th byte of cipher key");
  );
  REG(Temp1, "Temp1", 0x4E, RW, "Temperature Sensor control",
    VAR(Temp1, TempMeasStart, "TempMeasStart", 0x08, WO,                        "");
    VAR(Temp1, TempMeasRunning, "TempMeasRunning", 0x04, RO,                    "");
  );
  REG(Temp2, "Temp2", 0x4F, RW, "Temperature readout",
    VAR(Temp2, TempValue, "TempValue", 0xFF, RO,                                "");
  );
  REG(TestLna, "TestLna", 0x58, RW, "Sensitivity boost",
    VAR(TestLna, SensitivityBoost, "SensitivityBoost", 0xFF, RW,                "");
  );
  REG(TestPa1, "TestPa1", 0x5A, RW, "High Power PA settings",
    VAR(TestPa1, Pa13dBm1, "Pa13dBm1", 0xFF, RW,                                "");
  );
  REG(TestPa2, "TestPa2", 0x5C, RW, "High Power PA settings",
    VAR(TestPa2, Pa13dBm2, "Pa13dBm2", 0xFF, RW,                                "");
  );
  REG(TestDagc, "TestDagc", 0x6F, RW, "Fading Margin Improvement",
    VAR(TestDagc, ContinuousDagc, "ContinuousDagc", 0xFF, RW,                   "");
  );
  REG(TestAfc, "TestAfc", 0x71, RW, "AFC offset for low modulation index AFC",
    VAR(TestAfc, LowBetaAfcOffset, "LowBetaAfcOffset", 0xFF, RW,                "");
  );

  void Set(const std::string &key, const std::string &value);
);

#undef VAR
#undef REG

#endif
