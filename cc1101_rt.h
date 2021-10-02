// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CC1101_RT_
#define _CC1101_RT_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE(CC1101, RegisterTable, uint8_t, uint8_t,
  REG(IOCFG2, "IOCFG2", 0x00, RW,                                         "GDO2 output pin configuration",
    VAR(IOCFG2, GDO2_INV, "GDO2_INV", 0x40, RW,                           "");
    VAR(IOCFG2, GDO2_CFG_5_0, "GDO2_CFG[5:0]", 0x3F, RW,                  "");
  );
  REG(IOCFG1, "IOCFG1", 0x01, RW,                                         "GDO1 output pin configuration",
    VAR(IOCFG1, GDO_DS, "GDO_DS", 0x80, RW,                               "");
    VAR(IOCFG1, GDO1_INV, "GDO1_INV", 0x40, RW,                           "");
    VAR(IOCFG1, GDO1_CFG_5_0, "GDO1_CFG[5:0]", 0x3F, RW,                  "");
  );
  REG(IOCFG0, "IOCFG0", 0x02, RW,                                         "GDO0 output pin configuration",
    VAR(IOCFG0, TEMP_SENSOR_ENABLE, "TEMP_SENSOR_ENABLE", 0x80, RW,       "");
    VAR(IOCFG0, GDO0_INV, "GDO0_INV", 0x40, RW,                           "");
    VAR(IOCFG0, GDO0_CFG_5_0, "GDO0_CFG[5:0]", 0x3F, RW,                  "");
  );
  REG(FIFOTHR, "FIFOTHR", 0x03, RW,                                       "RX FIFO and TX FIFO thresholds",
    VAR(FIFOTHR, ADC_RETENTION, "ADC_RETENTION", 0x40, RW,                "");
    VAR(FIFOTHR, CLOSE_IN_RX_1_0, "CLOSE_IN_RX[1:0]", 0x30, RW,           "");
    VAR(FIFOTHR, FIFO_THR_3_0, "FIFO_THR[3:0]", 0x0F, RW,                 "");
  );
  REG(SYNC1, "SYNC1", 0x04, RW,                                           "Sync word, high byte",
    VAR(SYNC1, SYNC_15_8, "SYNC[15:8]", 0xFF, RW,                         "");
  );
  REG(SYNC0, "SYNC0", 0x05, RW,                                           "Sync word, low byte",
    VAR(SYNC0, SYNC_7_0, "SYNC[7:0]", 0xFF, RW,                           "");
  );
  REG(PKTLEN, "PKTLEN", 0x06, RW,                                         "Packet length",
    VAR(PKTLEN, PACKET_LENGTH, "PACKET_LENGTH", 0xFF, RW,                 "");
  );
  REG(PKTCTRL1, "PKTCTRL1", 0x07, RW,                                     "Packet automation control (1)",
    VAR(PKTCTRL1, PQT_2_0, "PQT[2:0]", 0xE0, RW,                          "");
    VAR(PKTCTRL1, CRC_AUTOFLUSH, "CRC_AUTOFLUSH", 0x08, RW,               "");
    VAR(PKTCTRL1, APPEND_STATUS, "APPEND_STATUS", 0x04, RW,               "");
    VAR(PKTCTRL1, ADR_CHK_1_0, "ADR_CHK[1:0]", 0x03, RW,                  "");
  );
  REG(PKTCTRL0, "PKTCTRL0", 0x08, RW,                                     "Packet automation control (0)",
    VAR(PKTCTRL0, WHITE_DATA, "WHITE_DATA", 0x40, RW,                     "");
    VAR(PKTCTRL0, PKT_FORMAT_1_0, "PKT_FORMAT[1:0]", 0x30, RW,            "");
    VAR(PKTCTRL0, CRC_EN, "CRC_EN", 0x08, RW,                             "");
    VAR(PKTCTRL0, LENGTH_CONFIG_1_0, "LENGTH_CONFIG[1:0]", 0x03, RW,      "");
  );
  REG(ADDR, "ADDR", 0x09, RW,                                             "Device address",
    VAR(ADDR, DEVICE_ADDR_7_0, "DEVICE_ADDR[7:0]", 0xFF, RW,              "");
  );
  REG(CHANNR, "CHANNR", 0x0A, RW,                                         "Channel number",
    VAR(CHANNR, CHAN_7_0, "CHAN[7:0]", 0xFF, RW,                          "");
  );
  REG(FSCTRL1, "FSCTRL1", 0x0B, RW,                                       "Frequency synthesizer control (1)",
    VAR(FSCTRL1, FREQ_IF_4_0, "FREQ_IF[4:0]", 0x1F, RW,                   "");
  );
  REG(FSCTRL0, "FSCTRL0", 0x0C, RW,                                       "Frequency synthesizer control (0)",
    VAR(FSCTRL0, FREQOFF_7_0, "FREQOFF[7:0]", 0xFF, RW,                   "");
  );
  REG(FREQ2, "FREQ2", 0x0D, RW, "Frequency control word, high byte",
    VAR(FREQ2, FREQ_32_22, "FREQ[23:22]", 0xC0, RW,                       "");
    VAR(FREQ2, FREQ_21_16, "FREQ[23:22]", 0x3F, RW,                       "");
    );
  REG(FREQ1, "FREQ1", 0x0E, RW, "Frequency control word, middle byte",
    VAR(FREQ1, FREQ_15_8, "FREQ[15:8]", 0xFF, RW,                         "");
  );
  REG(FREQ0, "FREQ0", 0x0F, RW, "Frequency control word, low byte",
    VAR(FREQ0, FREQ_7_0, "FREQ[7:0]", 0xFF, RW,                           "");
  );
  REG(MDMCFG4, "MDMCFG4", 0x10, RW, "Modem configuration",
    VAR(MDMCFG4, CHANBW_E_1_0, "CHANBW_E[1:0]", 0xC0, RW,                 "");
    VAR(MDMCFG4, CHANBW_M_1_0, "CHANBW_M[1:0]", 0x30, RW,                 "");
    VAR(MDMCFG4, DRATE_E_3_0, "DRATE_E[3:0]", 0x0F, RW,                   "");
  );
  REG(MDMCFG3, "MDMCFG3", 0x11, RW, "Modem configuration",
    VAR(MDMCFG3, DRATE_M_7_0, "DRATE_M[7:0]", 0xFF, RW,                   "");
  );
  REG(MDMCFG2, "MDMCFG2", 0x12, RW, "Modem configuration",
    VAR(MDMCFG2, DEM_DCFILT_OFF, "DEM_DCFILT_OFF", 0x80, RW,              "");
    VAR(MDMCFG2, MOD_FORMAT_2_0, "MOD_FORMAT[2:0]", 0x70, RW,             "");
    VAR(MDMCFG2, MANCHESTER_EN, "MANCHESTER_EN", 0x08, RW,                "");
    VAR(MDMCFG2, SYNC_MODE, "SYNC_MODE[2:0]", 0x07, RW,                   "");
  );
  REG(MDMCFG1, "MDMCFG1", 0x13, RW, "Modem configuration",
    VAR(MDMCFG1, FEC_EN, "FEC_EN", 0x80, RW,                              "");
    VAR(MDMCFG1, NUM_PREAMBLE_2_0, "NUM_PREAMBLE[2:0]", 0x70, RW,         "");
    VAR(MDMCFG1, CHANSPC_E_1_0, "CHANSPC_E[1:0]", 0x03, RW,               "");
  );
  REG(MDMCFG0, "MDMCFG0", 0x14, RW, "Modem configuration",
    VAR(MDMCFG0, CHANSPC_M_7_0, "CHANSPC_M[7:0]", 0xFF, RW,               "");
  );
  REG(DEVIATN, "DEVIATN", 0x15, RW, "Modem deviation setting", );
  REG(MCSM2, "MCSM2", 0x16, RW, "Main Radio Control State Machine configuration", );
  REG(MCSM1, "MCSM1", 0x17, RW, "Main Radio Control State Machine configuration", );
  REG(MCSM0, "MCSM0", 0x18, RW, "Main Radio Control State Machine configuration", );
  REG(FOCCFG, "FOCCFG", 0x19, RW, "Frequency Offset Compensation configuration", );
  REG(BSCFG, "BSCFG", 0x1A, RW, "Bit Synchronization configuration", );
  REG(AGCCTRL2, "AGCTRL2", 0x1B, RW, "AGC control", );
  REG(AGCCTRL1, "AGCTRL1", 0x1C, RW, "AGC control", );
  REG(AGCCTRL0, "AGCTRL0", 0x1D, RW, "AGC control", );
  REG(WOREVT1, "WOREVT1", 0x1E, RW, "High byte Event 0 timeout", );
  REG(WOREVT0, "WOREVT0", 0x1F, RW, "Low byte Event 0 timeout", );
  REG(WORCTRL, "WORCTRL", 0x20, RW, "Wake On Radio control", );
  REG(FREND1, "FREND1", 0x21, RW, "Front end RX configuration", );
  REG(FREND0, "FREND0", 0x22, RW, "Front end TX configuration", );
  REG(FSCAL3, "FSCAL3", 0x23, RW, "Frequency synthesizer calibration", );
  REG(FSCAL2, "FSCAL2", 0x24, RW, "Frequency synthesizer calibration", );
  REG(FSCAL1, "FSCAL1", 0x25, RW, "Frequency synthesizer calibration", );
  REG(FSCAL0, "FSCAL0", 0x26, RW, "Frequency synthesizer calibration", );
  REG(RCCTRL1, "RCCTRL1", 0x27, RW, "RC oscillator configuration", );
  REG(RCCTRL0, "RCCTRL0", 0x28, RW, "RC oscillator configuration", );
  REG(FSTEST, "FSTEST", 0x29, RW, "Frequency synthesizer calibration control", );
  REG(PTEST, "PTEST", 0x2A, RW, "Production test", );
  REG(AGCTEST, "AGCTEST", 0x2B, RW, "AGC test", );
  REG(TEST2, "TEST2", 0x2C, RW, "Various test settings", );
  REG(TEST1, "TEST1", 0x2D, RW, "Various test settings", );
  REG(TEST0, "TEST0", 0x2E, RW, "Various test settings", );

  REG(PARTNUM, "PARTNUM", 0xF0, RO, "Part number for CC1101", );
  REG(VERSION, "VERSION", 0xF1, RO, "Current version number", );
  REG(FREQEST, "FREQEST", 0xF2, RO, "Frequency Offset Estimate", );
  REG(LQI, "LQI", 0xF3, RO, "Demodulator estimate for Link Quality", );
  REG(RSSI, "RSSI", 0xF4, RO, "Received signal strength indication", );
  REG(MARCSTATE, "MARCSTATE", 0xF5, RO, "Control state machine state", );
  REG(WORTIME1, "WORTIME1", 0xF6, RO, "High byte of WOR timer", );
  REG(WORTIME0, "WORTIME0", 0xF7, RO, "Low byte of WOR timer", );
  REG(PKTSTATUS, "PKTSTATUS", 0xF8, RO, "Current GDOx status and packet status", );
  REG(VCO_VC_DAC, "VCO_VC_DAC", 0xF9, RO, "Current setting from PLL calibration module", );
  REG(TXBYTES, "TXBYTES", 0xFA, RO, "Underflow and number of bytes in the TX FIFO", );
  REG(RXBYTES, "RXBYTES", 0xFB, RO, "Overflow and number of bytes in the RX FIFO", );
  REG(RCCTRL1_STATUS, "RCCTRL1_STATUS", 0xFC, RO, "Last RC oscillator calibration result", );
  REG(RCCTRL0_STATUS, "RCCTRL0_STATUS", 0xFD, RO, "Last RC oscillator calibration result", );

  REG(PATABLE, "PATABLE", 0x3E, RW, "P/A table", );
  REG(FIFO, "FIFO", 0x3F, RW, "FIFO Buffer", );

  std::vector<uint8_t> PATableBuffer;
);

#undef RO
#undef RW
#undef VAR
#undef REG

#endif
