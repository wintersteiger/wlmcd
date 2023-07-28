// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _S2LP_RT_H_
#define _S2LP_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE(S2LP, RegisterTable, uint8_t, uint8_t,

  REG(GPIO0_CONF, "GPIO0_CONF", 0x00, RW,                                 "",
    VAR(GPIO0_CONF, GPIO0_SELECT, "GPIO_SELECT", 0xF8, RW,                "");
    VAR(GPIO0_CONF, GPIO0_MODE, "GPIO_MODE", 0x03, RW,                    "");
  );
  REG(GPIO1_CONF, "GPIO1_CONF", 0x01, RW,                                 "",
    VAR(GPIO1_CONF, GPIO1_SELECT, "GPIO_SELECT", 0xF8, RW,                "");
    VAR(GPIO1_CONF, GPIO1_MODE, "GPIO_MODE", 0x03, RW,                    "");
  );
  REG(GPIO2_CONF, "GPIO2_CONF", 0x02, RW,                                 "",
    VAR(GPIO2_CONF, GPIO2_SELECT, "GPIO_SELECT", 0xF8, RW,                "");
    VAR(GPIO2_CONF, GPIO2_MODE, "GPIO_MODE", 0x03, RW,                    "");
  );
  REG(GPIO3_CONF, "GPIO3_CONF", 0x03, RW,                                 "",
    VAR(GPIO3_CONF, GPIO3_SELECT, "GPIO_SELECT", 0xF8, RW,                "");
    VAR(GPIO3_CONF, GPIO3_MODE, "GPIO_MODE", 0x03, RW,                    "");
  );

  REG(SYNT3, "SYNT3", 0x05, RW,                                           "",
    VAR(SYNT3, PLL_CP_ISEL, "PLL_CP_ISEL", 0xE0, RW,                      "");
    VAR(SYNT3, BS, "BS", 0x10, RW,                                        "");
    VAR(SYNT3, SYNT_27_24, "SYNT[27:24]", 0x0F, RW,                       "");
  );
  REG(SYNT2, "SYNT2", 0x06, RW,                                           "",
    VAR(SYNT2, SYNT_23_16, "SYNT[23:16]", 0xFF, RW,                       "");
  );
  REG(SYNT1, "SYNT1", 0x07, RW,                                           "",
    VAR(SYNT1, SYNT_15_8, "SYNT[15:8]", 0xFF, RW,                         "");
  );
  REG(SYNT0, "SYNT0", 0x08, RW,                                           "",
    VAR(SYNT0, SYNT_7_0, "SYNT[7:0]", 0xFF, RW,                           "");
  );

  REG(IF_OFFSET_ANA, "IF_OFFSET_ANA", 0x09, RW,                           "",);
  REG(IF_OFFSET_DIG, "IF_OFFSET_DIG", 0x0A, RW,                           "",);

  REG(CHSPACE, "CHSPACE", 0x0C, RW,                                       "",
    VAR(CHSPACE, CH_SPACE, "CH_SPACE", 0xFF, RW,                          "");
  );

  REG(CHNUM, "CHNUM", 0x0D, RW,                    "",);

  REG(MOD4, "MOD4", 0x0E, RW,                                             "",
      VAR(MOD4, DATARATE_M_15_8, "DATARATE_M[15:8]", 0xFF, RW,            "");
    );
  REG(MOD3, "MOD3", 0x0F, RW,                                             "",
      VAR(MOD3, DATARATE_M_7_0, "DATARATE_M[7:0]", 0xFF, RW,              "");
    );
  REG(MOD2, "MOD2", 0x10, RW,                                             "",
      VAR(MOD2, MOD_TYPE, "MOD_TYPE", 0xF0, RW,                           "");
      VAR(MOD2, DATARATE_E, "DATARATE_E", 0x0F, RW,                       "");
    );
  REG(MOD1, "MOD1", 0x11, RW,                                             "",
      VAR(MOD1, PA_INTERP_EN, "PA_INTERP_EN", 0x80, RW,                   "");
      VAR(MOD1, MOD_INTERP_EN, "MOD_INTERP_EN", 0x40, RW,                 "");
      VAR(MOD1, CONST_MAP, "CONST_MAP", 0x3, RW,                          "");
      VAR(MOD1, FDEV_E, "FDEV_E", 0x0F, RW,                               "");
    );
  REG(MOD0, "MOD0", 0x12, RW,                                             "",
    VAR(MOD0, FDEV_M, "FDEV_M", 0xFF, RW,                                 "");
  );

  REG(CHFLT, "CHFLT", 0x13, RW,                                           "",
    VAR(CHFLT, CHFLT_M, "CHFLT_M", 0xF0, RW,                              "");
    VAR(CHFLT, CHFLT_E, "CHFLT_E", 0x0F, RW,                              "");
  );

  REG(AFC2, "AFC2", 0x14, RW,                                             "",
    VAR(AFC2, AFC_FREEZE_ON_SYNC, "AFC_FREEZE_ON_SYNC", 0x80, RW,         "");
    VAR(AFC2, AFC_ENABLED, "AFC_ENABLED", 0x40, RW,                       "");
    VAR(AFC2, AFC_MODE, "AFC_MODE", 0x20, RW,                             "");
  );
  REG(AFC1, "AFC1", 0x15, RW,                                             "",
    VAR(AFC1, AFC_FAST_PERIOD, "AFC_FAST_PERIOD", 0xFF, RW,               "");
  );
  REG(AFC0, "AFC0", 0x16, RW,                                             "",
    VAR(AFC0, AFC_FAST_GAIN, "AFC_FAST_GAIN", 0xF0, RW,                   "");
    VAR(AFC0, AFC_SLOW_GAIN, "AFC_SLOW_GAIN", 0x0F, RW,                   "");
  );

  REG(RSSI_FLT_, "RSSI_FLT", 0x17, RW,                                     "",
    VAR(RSSI_FLT_, RSSI_FLT, "RSSI_FLT", 0xF0, RW,                         "");
    VAR(RSSI_FLT_, CS_MODE, "CS_MODE", 0x0C, RW,                           "");
  );
  REG(RSSI_TH, "RSSI_TH", 0x18, RW,                                       "",);

  REG(AGCCTRL4, "AGCCTRL4", 0x1A, RW,                                     "",
    VAR(AGCCTRL4, LOW_THRESHOLD_0, "LOW_THRESHOLD_0", 0xF0, RW,           "");
    VAR(AGCCTRL4, LOW_THRESHOLD_1, "LOW_THRESHOLD_1", 0x0F, RW,           "");
  );
  REG(AGCCTRL3, "AGCCTRL3", 0x1B, RW,                                     "",
    VAR(AGCCTRL3, LOW_THRESHOLD_SEL, "LOW_THRESHOLD_SEL", 0xFF, RW,       "");
  );
  REG(AGCCTRL2, "AGCCTRL2", 0x1C, RW,                                     "",
    VAR(AGCCTRL2, FREEZE_ON_SYNC, "FREEZE_ON_SYNC", 0x20, RW,             "");
    VAR(AGCCTRL2, MEAS_TIME, "MEAS_TIME", 0x0F, RW,                       "");
  );
  REG(AGCCTRL1, "AGCCTRL1", 0x1D, RW,                                     "",
    VAR(AGCCTRL1, HIGH_THRESHOLD, "HIGH_THRESHOLD", 0xF0, RW,             "");
  );
  REG(AGCCTRL0, "AGCCTRL0", 0x1E, RW,                                     "",
    VAR(AGCCTRL0, AGC_ENABLE, "AGC_ENABLE", 0x80, RW,                     "");
    VAR(AGCCTRL0, HOLD_TIME, "HOLD_TIME", 0x3F, RW,                       "");
  );

  REG(ANT_SELECT_CONF, "ANT_SELECT_CONF", 0x1F, RW,                       "",
    VAR(ANT_SELECT_CONF, EQU_CTRL, "EQU_CTRL", 0x60, RW,                  "");
    VAR(ANT_SELECT_CONF, CS_BLANKING, "CS_BLANKING", 0x10, RW,            "");
    VAR(ANT_SELECT_CONF, AS_ENABLE, "AS_ENABLE", 0x80, RW,                "");
    VAR(ANT_SELECT_CONF, AS_MEAS_TIME, "AS_MEAS_TIME", 0x07, RW,          "");
  );

  REG(CLOCKREC2, "CLOCKREC2", 0x20, RW,                                   "",
    VAR(CLOCKREC2, CLK_REC_P_GAIN_SLOW, "CLK_REC_P_GAIN_SLOW", 0xE0, RW,  "");
    VAR(CLOCKREC2, CLK_REC_ALGO_SEL, "CLK_REC_ALGO_SEL", 0x10, RW,        "");
    VAR(CLOCKREC2, CLK_REC_I_GAIN_SLOW, "CLK_REC_I_GAIN_SLOW", 0x0F, RW,  "");
  );
  REG(CLOCKREC1, "CLOCKREC1", 0x21, RW,                                   "",
    VAR(CLOCKREC1, CLK_REC_P_GAIN_FAST, "CLK_REC_P_GAIN_FAST", 0xE0, RW,  "");
    VAR(CLOCKREC1, PSTFLT_LEN, "PSTFLT_LEN", 0x10, RW,                    "");
    VAR(CLOCKREC1, CLK_REC_I_GAIN_FAST, "CLK_REC_I_GAIN_FAST", 0x0F, RW,  "");
  );

  REG(PKTCTRL6, "PKTCTRL6", 0x2B, RW,                                     "",
    VAR(PKTCTRL6, SYNC_LEN, "SYNC_LEN", 0xFC, RW,                         "");
    VAR(PKTCTRL6, PREAMBLE_LEN_9_8, "PREAMBLE_LEN[9:8]", 0x03, RW,        "");
  );
  REG(PKTCTRL5, "PKTCTRL5", 0x2C, RW,                                     "",
    VAR(PKTCTRL5, PREAMBLE_LEN_7_0, "PREAMBLE_LEN[7:0]", 0xFF, RW,        "");
  );
  REG(PKTCTRL4, "PKTCTRL4", 0x2D, RW,                                     "",
    VAR(PKTCTRL4, LEN_WID, "LEN_WID", 0x80, RW,          "");
    VAR(PKTCTRL4, ADDRESS_LEN, "ADDRESS_LEN", 0x08, RW,                   "");
  );
  REG(PKTCTRL3, "PKTCTRL3", 0x2E, RW,                                     "",
    VAR(PKTCTRL3, PCKT_FRMT, "PCKT_FRMT", 0xC0, RW,                       "");
    VAR(PKTCTRL3, RX_MODE, "RX_MODE", 0x30, RW,                           "");
    VAR(PKTCTRL3, FSK4_SYM_SWAP, "FSK4_SYM_SWAP", 0x08, RW,               "");
    VAR(PKTCTRL3, BYTE_SWAP, "BYTE_SWAP", 0x04, RW,                       "");
    VAR(PKTCTRL3, PREAMBLE_SEL, "PREAMBLE_SEL", 0x03, RW,                 "");
  );
  REG(PKTCTRL2, "PKTCTRL2", 0x2F, RW,                                     "",
    VAR(PKTCTRL2, FCS_TYPE_4G, "FCS_TYPE_4G", 0x20, RW,                   "");
    VAR(PKTCTRL2, FEC_TYPE_4G_STOP_BIT, "FEC_TYPE_4G/STOP_BIT", 0x10, RW, "");
    VAR(PKTCTRL2, INT_EN_4G_START_BIT, "INT_EN_4G/START_BIT", 0x08, RW,   "");
    VAR(PKTCTRL2, MBUS_3OF6_EN, "MBUS_3OF6_EN", 0x04, RW,                 "");
    VAR(PKTCTRL2, MANCHESTER_EN, "MANCHESTER_EN", 0x02, RW,               "");
    VAR(PKTCTRL2, FIX_VAR_LEN, "FIX_VAR_LEN", 0x01, RW,                   "");
  );
  REG(PKTCTRL1, "PKTCTRL1", 0x30, RW,                                     "",
    VAR(PKTCTRL1, CRC_MODE, "CRC_MODE", 0xE0, RW,                         "");
    VAR(PKTCTRL1, WHIT_EN, "WHIT_EN", 0x10, RW,                           "");
    VAR(PKTCTRL1, TXSOURCE, "TXSOURCE", 0x0C, RW,                         "");
    VAR(PKTCTRL1, SECOND_SYNC_SEL, "SECOND_SYNC_SEL", 0x02, RW,           "");
    VAR(PKTCTRL1, FEC_EN, "FEC_EN", 0x01, RW,                             "");
  );
  REG(PCKTLEN1, "PCKTLEN1", 0x31, RW,                                     "",);
  REG(PCKTLEN0, "PCKTLEN0", 0x32, RW,                                     "",);

  REG(SYNC4, "SYNC4", 0x33, RW,                                           "",);
  REG(SYNC3, "SYNC3", 0x34, RW,                                           "",);
  REG(SYNC2, "SYNC2", 0x35, RW,                                           "",);
  REG(SYNC1, "SYNC1", 0x36, RW,                                           "",);

  REG(QI, "QI", 0x37, RW,                                                 "",
    VAR(QI, SQI_TH, "SQI_TH", 0xE0, RW,                                   "");
    VAR(QI, PQI_TH, "PQI_TH", 0x1E, RW,                                   "");
    VAR(QI, SQI_EN, "SQI_EN", 0x01, RW,                                   "");
  );

  REG(PCKT_PSTMBL, "PCKT_PSTMBL", 0x38, RW,                               "",);

  REG(PROTOCOL2, "PROTOCOL2", 0x39, RW,                                   "",
    VAR(PROTOCOL2, CS_TIMEOUT_MASK, "CS_TIMEOUT_MASK", 0x80, RW,          "");
    VAR(PROTOCOL2, SQI_TIMEOUT_MASK, "SQI_TIMEOUT_MASK", 0x40, RW,        "");
    VAR(PROTOCOL2, PQI_TIMEOUT_MASK, "PQI_TIMEOUT_MASK", 0x20, RW,        "");
    VAR(PROTOCOL2, TX_SEQ_NUM_RELOAD, "TX_SEQ_NUM_RELOAD", 0x18, RW,      "");
    VAR(PROTOCOL2, FIFO_GPIO_OUT_MUX_SEL, "FIFO_GPIO_OUT_MUX_SEL", 0x04, RW, "");
    VAR(PROTOCOL2, LDC_TIMER_MULT, "LDC_TIMER_MULT", 0x03, RW,            "");
  );
  REG(PROTOCOL1, "PROTOCOL1", 0x3A, RW,                                   "",
    VAR(PROTOCOL1, LDC_MODE, "LDC_MODE", 0x80, RW,                        "");
    VAR(PROTOCOL1, LDC_RELOAD_ON_SYNC, "LDC_RELOAD_ON_SYNC", 0x40, RW,    "");
    VAR(PROTOCOL1, PIGGYBACKING, "PIGGYBACKING", 0x20, RW,                "");
    VAR(PROTOCOL1, FAST_CS_TERM_EN, "FAST_CS_TERM_EN", 0x10, RW,          "");
    VAR(PROTOCOL1, SEED_RELOAD, "SEED_RELOAD", 0x08, RW,                  "");
    VAR(PROTOCOL1, CSMA_ON, "CSMA_ON", 0x04, RW,                          "");
    VAR(PROTOCOL1, CSMA_PERS_ON, "CSMA_PERS_ON", 0x02, RW,                "");
    VAR(PROTOCOL1, AUTO_PCKT_FLT, "AUTO_PCKT_FLT", 0x01, RW,              "");
  );
  REG(PROTOCOL0, "PROTOCOL0", 0x3B, RW,                                   "",
    VAR(PROTOCOL0, NMAX_RETX, "NMAX_RETX", 0xF0, RW,                      "");
    VAR(PROTOCOL0, NACK_TX, "NACK_TX", 0x08, RW,                          "");
    VAR(PROTOCOL0, AUTO_ACK, "AUTO_ACK", 0x04, RW,                        "");
    VAR(PROTOCOL0, PERS_RX, "PERS_RX", 0x02, RW,                          "");
  );

  REG(FIFO_CONFIG3, "FIFO_CONFIG3", 0x3C, RW,                             "",
    VAR(FIFO_CONFIG3, RX_AFTHR, "RX_AFTHR", 0x7F, RW,                     "");
  );
  REG(FIFO_CONFIG2, "FIFO_CONFIG2", 0x3D, RW,                             "",
    VAR(FIFO_CONFIG2, RX_AETHR, "RX_AETHR", 0x7F, RW,                     "");
  );
  REG(FIFO_CONFIG1, "FIFO_CONFIG1", 0x3E, RW,                             "",
    VAR(FIFO_CONFIG1, TX_AFTHR, "TX_AFTHR", 0x7F, RW,                     "");
  );
  REG(FIFO_CONFIG0, "FIFO_CONFIG0", 0x3F, RW,                             "",
    VAR(FIFO_CONFIG3, TX_AETHR, "TX_AETHR", 0x7F, RW,                     "");
  );

  REG(PCKT_FLT_OPTIONS, "PCKT_FLT_OPTIONS", 0x40, RW,                     "",
    VAR(PCKT_FLT_OPTIONS, RX_TIMEOUT_AND_OR_SEL, "RX_TIMEOUT_AND_OR_SEL", 0x40, RW, "");
    VAR(PCKT_FLT_OPTIONS, SOURCE_ADDR_FLT, "SOURCE_ADDR_FLT", 0x10, RW,   "");
    VAR(PCKT_FLT_OPTIONS, DEST_VS_BROADCAST_ADDR, "DEST_VS_BROADCAST_ADDR", 0x08, RW, "");
    VAR(PCKT_FLT_OPTIONS, DEST_VS_MULTICAST_ADDR, "DEST_VS_MULTICAST_ADDR", 0x04, RW, "");
    VAR(PCKT_FLT_OPTIONS, DEST_VS_SOURCE_ADDR, "DEST_VS_SOURCE_ADDR", 0x02, RW, "");
    VAR(PCKT_FLT_OPTIONS, CRC_FLT, "CRC_FLT", 0x01, RW,                   "");
  );

  REG(PCKT_FLT_GOALS4, "PCKT_FLT_GOALS4", 0x41, RW,                       "",
    VAR(PCKT_FLT_GOALS4, RX_SOURCE_MASK, "RX_SOURCE_MASK", 0xFF, RW,      "");
  );
  REG(PCKT_FLT_GOALS3, "PCKT_FLT_GOALS3", 0x42, RW,                       "",
    VAR(PCKT_FLT_GOALS3, RX_SOURCE_ADDR_DUAL_SYNC3, "RX_SOURCE_ADDR/DUAL_SYNC3", 0xFF, RW, "");
  );
  REG(PCKT_FLT_GOALS2, "PCKT_FLT_GOALS2", 0x43, RW,                       "",
    VAR(PCKT_FLT_GOALS2, BROADCAST_ADDR_DUAL_SYNC2, "BROADCAST_ADDR/DUAL_SYNC2", 0xFF, RW, "");
  );
  REG(PCKT_FLT_GOALS1, "PCKT_FLT_GOALS1", 0x44, RW,                       "",
    VAR(PCKT_FLT_GOALS1, MULTICAST_ADDR_DUAL_SYNC1, "MULTICAST_ADDR/DUAL_SYNC1", 0xFF, RW, "");
  );
  REG(PCKT_FLT_GOALS0, "PCKT_FLT_GOALS0", 0x45, RW,                       "",
    VAR(PCKT_FLT_GOALS0, TX_SOURCE_ADDR_DUAL_SYNC0, "TX_SOURCE_ADDR/DUAL_SYNC0", 0xFF, RW, "");
  );

  REG(TIMERS5, "TIMERS5", 0x46, RW,                                       "",
    VAR(TIMERS5, RX_TIMER_CNTR, "RX_TIMER_CNTR", 0xFF, RW,                "");
  );
  REG(TIMERS4, "TIMERS4", 0x47, RW,                                       "",
    VAR(TIMERS4, RX_TIMER_PRESC, "RX_TIMER_PRESC", 0xFF, RW,              "");
  );
  REG(TIMERS3, "TIMERS3", 0x48, RW,                                       "",
    VAR(TIMERS3, LDC_TIMER_PRESC, "LDC_TIMER_PRESC", 0xFF, RW,            "");
  );
  REG(TIMERS2, "TIMERS2", 0x49, RW,                                       "",
    VAR(TIMERS2, LDC_TIMER_CNTR, "LDC_TIMER_CNTR", 0xFF, RW,              "");
  );
  REG(TIMERS1, "TIMERS1", 0x4A, RW,                                       "",
    VAR(TIMERS1, LDC_RELOAD_PRSC, "LDC_RELOAD_PRSC", 0xFF, RW,            "");
  );
  REG(TIMERS0, "TIMERS0", 0x4B, RW,                                       "",
    VAR(TIMERS0, LDC_RELOAD_CNTR, "LDC_RELOAD_CNTR", 0xFF, RW,            "");
  );

  REG(CSMA_CONFIG3, "CSMA_CONFIG3", 0x4C, RW,                             "",
    VAR(CSMA_CONFIG3, BU_CNTR_SEED_14_8, "BU_CNTR_SEED[14:8]", 0xFF, RW,  "");
  );
  REG(CSMA_CONFIG2, "CSMA_CONFIG2", 0x4D, RW,                             "",
    VAR(CSMA_CONFIG2, BU_CNTR_SEED_7_0, "BU_CNTR_SEED[7:0]", 0xFF, RW,    "");
  );
  REG(CSMA_CONFIG1, "CSMA_CONFIG1", 0x4E, RW,                             "",
    VAR(CSMA_CONFIG1, BU_PRSC, "BU_PRSC", 0xFC, RW,                       "");
    VAR(CSMA_CONFIG1, CCA_PERIOD, "CCA_PERIOD", 0x03, RW,                 "");
  );
  REG(CSMA_CONFIG0, "CSMA_CONFIG0", 0x4F, RW,                             "",
    VAR(CSMA_CONFIG0, CCA_LEN, "CCA_LEN", 0xF0, RW,                       "");
    VAR(CSMA_CONFIG0, NBACKOFF_MAX, "NBACKOFF_MAX", 0x07, RW,             "");
  );

  REG(IRQ_MASK3, "IRQ_MASK3", 0x50, RW,                                   "",
    VAR(IRQ_MASK3, INT_MASK_31_24, "INT_MASK[31:24]", 0xFF, RW,           "");
  );
  REG(IRQ_MASK2, "IRQ_MASK2", 0x51, RW,                                   "",
    VAR(IRQ_MASK2, INT_MASK_23_16, "INT_MASK[23:16]", 0xFF, RW,           "");
  );
  REG(IRQ_MASK1, "IRQ_MASK1", 0x52, RW,                                   "",
    VAR(IRQ_MASK1, INT_MASK_15_8, "INT_MASK[15:8]", 0xFF, RW,             "");
  );
  REG(IRQ_MASK0,"IRQ_MASK0", 0x53, RW,                                    "",
    VAR(IRQ_MASK0, INT_MASK_7_0, "INT_MASK[7:0]", 0xFF, RW,               "");
  );

  REG(FAST_RX_TIMER,"FAST_RX_TIMER", 0x54, RW,                            "",
    VAR(FAST_RX_TIMER, RSSI_SETTLING_LIMIT, "RSSI_SETTLING_LIMIT]", 0xFF, RW, "");
  );

  REG(PA_POWER8, "PA_POWER8", 0x5A, RW,                                   "",
    VAR(PA_POWER8, PA_LEVEL_8, "PA_LEVEL_8", 0xFF, RW,                    "");
  );
  REG(PA_POWER7, "PA_POWER7", 0x5B, RW,                                   "",
    VAR(PA_POWER7, PA_LEVEL_7, "PA_LEVEL_7", 0xFF, RW,                    "");
  );
  REG(PA_POWER6, "PA_POWER6", 0x5C, RW,                                   "",
    VAR(PA_POWER6, PA_LEVEL_6, "PA_LEVEL_6", 0xFF, RW,                    "");
  );
  REG(PA_POWER5, "PA_POWER5", 0x5D, RW,                                   "",
    VAR(PA_POWER5, PA_LEVEL_5, "PA_LEVEL_5", 0xFF, RW,                    "");
  );
  REG(PA_POWER4, "PA_POWER4", 0x5E, RW,                                   "",
    VAR(PA_POWER4, PA_LEVEL_4, "PA_LEVEL_4", 0xFF, RW,                    "");
  );
  REG(PA_POWER3, "PA_POWER3", 0x5F, RW,                                   "",
    VAR(PA_POWER3, PA_LEVEL_3, "PA_LEVEL_3", 0xFF, RW,                    "");
  );
  REG(PA_POWER2, "PA_POWER2", 0x60, RW,                                   "",
    VAR(PA_POWER2, PA_LEVEL_2, "PA_LEVEL_2", 0xFF, RW,                    "");
  );
  REG(PA_POWER1, "PA_POWER1", 0x61, RW,                                   "",
    VAR(PA_POWER1, PA_LEVEL_1, "PA_LEVEL_1", 0xFF, RW,                    "");
  );
  REG(PA_POWER0, "PA_POWER0", 0x62, RW,                                   "",
    VAR(PA_POWER0, DIG_SMOOTH_EN, "DIG_SMOOTH_EN", 0x80, RW,              "");
    VAR(PA_POWER0, PA_MAXDBM, "PA_MAXDBM", 0x40, RW,                      "");
    VAR(PA_POWER0, PA_RAMP_EN, "PA_RAMP_EN", 0x20, RW,                    "");
    VAR(PA_POWER0, PA_RAMP_STEP_LEN, "PA_RAMP_STEP_LEN", 0x18, RW,        "");
    VAR(PA_POWER0, PA_LEVEL_MAX_IDX, "PA_LEVEL_MAX_IDX", 0x7, RW,         "");
  );

  REG(PA_CONFIG1, "PA_CONFIG1", 0x63, RW,                                 "",
    VAR(PA_CONFIG1, FIR_CFG, "FIR_CFG", 0x0C, RW,                         "");
    VAR(PA_CONFIG1, FIR_EN, "FIR_EN", 0x02, RW,                           "");
  );
  REG(PA_CONFIG0, "PA_CONFIG0", 0x64, RW,                                 "",
    VAR(PA_CONFIG0, PA_DEGEN_TRIM, "PA_DEGEN_TRIM", 0xF0, RW,             "");
    VAR(PA_CONFIG0, PA_DEGEN_ON, "PA_DEGEN_ON", 0x08, RW,                 "");
    VAR(PA_CONFIG0, SAFE_ASK_CAL, "SAFE_ASK_CAL", 0x04, RW,               "");
    VAR(PA_CONFIG0, PA_FC, "PA_FC", 0x03, RW,                             "");
  );

  REG(SYNTH_CONFIG2, "SYNTH_CONFIG2", 0x65, RW,                           "",
    VAR(SYNTH_CONFIG2, PLL_PFD_SPLIT_EN, "PLL_PFD_SPLIT_EN", 0x04, RW,    "");
  );

  REG(VCO_CONFIG, "VCO_CONFIG", 0x68, RW,                                 "",
    VAR(VCO_CONFIG, VCO_CALAMP_EXT_SEL, "VCO_CALAMP_EXT_SEL", 0x20, RW,   "");
    VAR(VCO_CONFIG, VCO_CALFREQ_EXT_SEL, "VCO_CALFREQ_EXT_SEL", 0x10, RW, "");
  );
  REG(VCO_CALIBR_IN2, "VCO_CALIBR_IN2", 0x69, RW,                         "",);
  REG(VCO_CALIBR_IN1, "VCO_CALIBR_IN1", 0x6A, RW,                         "",);
  REG(VCO_CALIBR_IN0, "VCO_CALIBR_IN0", 0x6B, RW,                         "",);

  REG(XO_RCO_CONFIG1, "XO_RCO_CONFIG1", 0x6C, RW,                         "",
    VAR(XO_RCO_CONFIG1, PD_CLKDIV, "PD_CLKDIV", 0x10, RW,                 "");
  );
  REG(XO_RCO_CONFIG0, "XO_RCO_CONFIG0", 0x6C, RW,                         "",
    VAR(XO_RCO_CONFIG0, EXT_REF, "EXT_REF", 0x80, RW,                     "");
    VAR(XO_RCO_CONFIG0, GM_CONF, "GM_CONF", 0x70, RW,                     "");
    VAR(XO_RCO_CONFIG0, REFDIV, "REFDIV", 0x08, RW,                       "");
    VAR(XO_RCO_CONFIG0, EXT_RCO_OSC, "EXT_RCO_OSC", 0x02, RW,             "");
    VAR(XO_RCO_CONFIG0, RCO_CALIBRATION, "RCO_CALIBRATION", 0x01, RW,     "");
  );

  REG(RCO_CALIBR_CONF3, "RCO_CALIBR_CONF3", 0x6E, RW,                     "",
  VAR(RCO_CALIBR_CONF3, RWT_IN, "RWT_IN", 0xF0, RW,                       "");
    VAR(RCO_CALIBR_CONF3, RFB_IN_4_1, "RFB_IN[4:1]", 0x0F, RW,            "");
  );
  REG(RCO_CALIBR_CONF2, "RCO_CALIBR_CONF2", 0x6F, RW,                     "",
    VAR(RCO_CALIBR_CONF2, RFB_IN_0_0, "RFB_IN[0:0]", 0x80, RW,            "");
  );

// TODO

  REG(PM_CONF4, "PM_CONF4", 0x75, RW,                                     "",
    VAR(PM_CONF4, EXT_SMPS, "EXT_SMPS", 0x20, RW,                         "");
  );
  REG(PM_CONF3, "PM_CONF3", 0x76, RW,                                     "",
    VAR(PM_CONF3, KRM_EN, "KRM_EN", 0x80, RW,                             "");
    VAR(PM_CONF3, KRM_14_8, "KRM[14:8]", 0x7F, RW,                        "");
  );
  REG(PM_CONF2, "PM_CONF2", 0x77, RW,                                     "",
    VAR(PM_CONF2, KRM_7_0, "KRM[7:0]", 0xFF, RW,                          "");
  );
  REG(PM_CONF1, "PM_CONF1", 0x78, RW,                                     "",
    VAR(PM_CONF1, BATTERY_LVL_EN, "BATTERY_LVL_EN", 0x40, RW,             "");
    VAR(PM_CONF1, SET_BLD_TH, "SET_BLD_TH", 0x30, RW,                     "");
    VAR(PM_CONF1, SMPS_LVL_MODE, "SMPS_LVL_MODE", 0x80, RW,               "");
    VAR(PM_CONF1, BYPASS_LDO, "BYPASS_LDO", 0x40, RW,                     "");
  );
  REG(PM_CONF0, "PM_CONF0", 0x79, RW,                                     "",
    VAR(PM_CONF0, SET_SMPS_LVL, "SET_SMPS_LVL", 0x70, RW,                 "");
    VAR(PM_CONF0, SLEEP_MODE_SEL, "SLEEP_MODE_SEL", 0x01, RW,             "");
  );

  REG(MC_STATE1, "MC_STATE1", 0x8D, RO,                                   "",
    VAR(MC_STATE1, RCO_CAL_OK, "RCO_CAL_OK", 0x10, RO,                    "");
    VAR(MC_STATE1, ANT_SEL, "ANT_SEL", 0x08, RO,                          "");
    VAR(MC_STATE1, TX_FIFO_FULL, "TX_FIFO_FULL", 0x04, RO,                "");
    VAR(MC_STATE1, RX_FIFO_EMPTY, "RX_FIFO_EMPTY", 0x02, RO,              "");
    VAR(MC_STATE1, ERROR_LOCK, "ERROR_LOCK", 0x01, RO,                    "");
  );
  REG(MC_STATE0, "MC_STATE0", 0x8E, RO,                                   "",
    VAR(MC_STATE0, STATE, "STATE", 0xFE, RO,                              "");
    VAR(MC_STATE0, XO_ON, "XO_ON", 0x01, RO,                              "");
  );

  REG(TX_FIFO_STATUS, "TX_FIFO_STATUS", 0x8F, RO,                         "",
    VAR(TX_FIFO_STATUS, NELEM_TXFIFO, "NELEM_TXFIFO", 0xFF, RO,           "");
  );
  REG(RX_FIFO_STATUS, "RX_FIFO_STATUS", 0x90, RO,                         "",
    VAR(RX_FIFO_STATUS, NELEM_RXFIFO, "NELEM_RXFIFO", 0xFF, RO,           "");
  );

  REG(RCO_CALIBR_OUT4, "RCO_CALIBR_OUT4", 0x94, RW,                       "",
    VAR(RCO_CALIBR_OUT4, RFB_OUT_4_1, "RFB_OUT[4:1]", 0x07, RW,           "");
  );
  REG(RCO_CALIBR_OUT3, "RCO_CALIBR_OUT3", 0x95, RW,                       "",
    VAR(RCO_CALIBR_OUT3, RFB_OUT_0_0, "RFB_OUT[0:0]", 0xFF, RW,           "");
  );
  REG(VCO_CALIBR_OUT1, "VCO_CALIBR_OUT1", 0x99, RW,                       "",
    VAR(VCO_CALIBR_OUT1, VCO_CAL_AMP_OUT, "VCO_CAL_AMP_OUT", 0x0F, RW,    "");
  );
  REG(VCO_CALIBR_OUT0, "VCO_CALIBR_OUT1", 0x9A, RW,                       "",
    VAR(VCO_CALIBR_OUT0, VCO_CAL_FREQ_OUT, "VCO_CAL_FREQ_OUT", 0x7F, RW,  "");
  );

  REG(TX_PCKT_INFO, "TX_PCKT_INFO", 0x9C, RO,                             "",
    VAR(TX_PCKT_INFO, TX_SEQ_NUM, "TX_SEQ_NUM", 0x30, RW,                 "");
    VAR(TX_PCKT_INFO, N_RETX, "N_RETX", 0x0F, RW,                         "");
  );
  REG(RX_PCKT_INFO, "RX_PCKT_INFO", 0x9D, RO,                             "",
    VAR(RX_PCKT_INFO, NACK_RX, "NACK_RX", 0x04, RW,                       "");
    VAR(RX_PCKT_INFO, RX_SEQ_NUM, "RX_SEQ_NUM", 0x03, RW,                 "");
  );

  REG(AFC_CORR, "AFC_CORR", 0x9E, RO,                                     "",);

  REG(LINK_QUALIF2, "LINK_QUALIF2", 0x9F, RO,                             "",
    VAR(LINK_QUALIF2, PQI, "PQI", 0xFF, RW,                               "");
  );
  REG(LINK_QUALIF1, "LINK_QUALIF1", 0xA0, RO,                             "",
    VAR(LINK_QUALIF1, CS, "CS", 0x80, RW,                                 "");
    VAR(LINK_QUALIF1, SQI, "SQI", 0x7F, RW,                               "");
  );

  REG(RSSI_LEVEL, "RSSI_LEVEL", 0xA2, RO,                                 "",);

  REG(RX_PCKT_LEN1, "RX_PCKT_LEN1", 0xA4, RO,                             "",
    VAR(RX_PCKT_LEN1, RX_PCKT_LEN_14_8, "RX_PCKT_LEN[14:8]", 0x7F, RW,    "");
  );
  REG(RX_PCKT_LEN0, "RX_PCKT_LEN0", 0xA5, RO,                             "",
    VAR(RX_PCKT_LEN0, RX_PCKT_LEN_7_0, "RX_PCKT_LEN[7:0]", 0x7F, RW,      "");
  );

  REG(CRC_FIELD3, "CRC_FIELD3", 0xA6, RO,                                 "",);
  REG(CRC_FIELD2, "CRC_FIELD2", 0xA7, RO,                                 "",);
  REG(CRC_FIELD1, "CRC_FIELD1", 0xA8, RO,                                 "",);
  REG(CRC_FIELD0, "CRC_FIELD0", 0xA9, RO,                                 "",);

  REG(RX_ADDRE_FIELD1, "RX_ADDRE_FIELD1", 0xAA, RO,                       "",);
  REG(RX_ADDRE_FIELD0, "RX_ADDRE_FIELD0", 0xAB, RO,                       "",);

  REG(RSSI_LEVEL_RUN, "RSSI_LEVEL_RUN", 0xEF, RO,                         "",);

  REG(DEVICE_INFO1, "DEVICE_INFO1", 0xF0, RO,                             "",
    VAR(DEVICE_INFO1, PARTNUM, "PARTNUM", 0xFF, RO,                       "");
  );
  REG(DEVICE_INFO0, "DEVICE_INFO0", 0xF1, RO,                              "",
    VAR(DEVICE_INFO0, VERSION, "VERSION", 0xFF, RO,                       "");
  );

  REG(IRQ_STATUS3, "IRQ_STATUS3", 0xFA, RO,                               "",
    VAR(IRQ_STATUS3, INT_LEVEL_31_24, "INT_LEVEL[31:24]", 0xFF, RO,       "");
  );
  REG(IRQ_STATUS2, "IRQ_STATUS2", 0xFB, RO,                               "",
    VAR(IRQ_STATUS2, INT_LEVEL_23_16, "INT_LEVEL[23:16]", 0xFF, RO,       "");
  );
  REG(IRQ_STATUS1, "IRQ_STATUS1", 0xFC, RO,                               "",
    VAR(IRQ_STATUS1, INT_LEVEL_15_8, "INT_LEVEL[15:8]", 0xFF, RO,         "");
  );
  REG(IRQ_STATUS0, "IRQ_STATUS0", 0xFD, RO,                               "",
    VAR(IRQ_STATUS0, INT_LEVEL_7_0, "INT_LEVEL[7:0]", 0xFF, RO,           "");
  );
);

#undef REG
#undef VAR

#endif // _S2LP_RT_H_