// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SPIRIT1_RT_H_
#define _SPIRIT1_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE(SPIRIT1, RegisterTable, uint8_t, uint8_t,

  // General configuration
  REG(ANA_FUNC_CONF_1, "ANA_FUNC_CONF[1]", 0x00, RW,                      "",
    VAR(ANA_FUNC_CONF_1, GM_CONF_2_0, "GM_CONF[2:0]", 0x1C, RW,           "");
    VAR(ANA_FUNC_CONF_1, SET_BLD_LVL_1_0, "SET_BLD_LVL[1:0]", 0x03, RW,   "");
  );
  REG(ANA_FUNC_CONF_0, "ANA_FUNC_CONF[0]", 0x01, RW,                      "",
    VAR(ANA_FUNC_CONF_0, _24_26_MHz_SELECT, "24_26_MHz_SELECT", 0x40, RW, "");
    VAR(ANA_FUNC_CONF_0, AES_ON, "AES_ON", 0x20, RW,                      "");
    VAR(ANA_FUNC_CONF_0, EXT_REF, "EXT_REF", 0x10, RW,                    "");
    VAR(ANA_FUNC_CONF_0, BROWN_OUT, "BROWN_OUT", 0x04, RW,                "");
    VAR(ANA_FUNC_CONF_0, BATTERY_LEVEL, "BATTERY_LEVEL", 0x02, RW,        "");
    VAR(ANA_FUNC_CONF_0, TS, "TS", 0x01, RW,                              "");
  );
  REG(GPIO3_CONF, "GPIO3_CONF", 0x02, RW,                                 "",
    VAR(GPIO3_CONF, GPIO3_SELECT_4_0, "GPIO_SELECT[4:0]", 0xF8, RW,        "");
    VAR(GPIO3_CONF, GPIO3_MODE_1_0, "GPIO_MODE[1:0]", 0x03, RW,            "");
  );
  REG(GPIO2_CONF, "GPIO2_CONF", 0x03, RW,                                 "",
    VAR(GPIO2_CONF, GPIO2_SELECT_4_0, "GPIO_SELECT[4:0]", 0xF8, RW,        "");
    VAR(GPIO2_CONF, GPIO2_MODE_1_0, "GPIO_MODE[1:0]", 0x03, RW,            "");
  );
  REG(GPIO1_CONF, "GPIO1_CONF", 0x04, RW,                                 "",
    VAR(GPIO1_CONF, GPIO1_SELECT_4_0, "GPIO_SELECT[4:0]", 0xF8, RW,        "");
    VAR(GPIO1_CONF, GPIO1_MODE_1_0, "GPIO_MODE[1:0]", 0x03, RW,            "");
  );
  REG(GPIO0_CONF, "GPIO0_CONF", 0x05, RW,                                 "",
    VAR(GPIO0_CONF, GPIO0_SELECT_4_0, "GPIO_SELECT[4:0]", 0xF8, RW,        "");
    VAR(GPIO0_CONF, GPIO0_MODE_1_0, "GPIO_MODE[1:0]", 0x03, RW,            "");
  );
  REG(MCU_CK_CONF, "MCU_CK_CONF", 0x06, RW,                               "",
    VAR(MCU_CK_CONF, EN_MCU_CLK, "EN_MCU_CLK", 0x80, RW,                  "");
    VAR(MCU_CK_CONF, CLOCK_TAIL_1_0, "CLOCK_TAIL[1:0]", 0x60, RW,         "");
    VAR(MCU_CK_CONF, XO_RATIO_3_0, "XO_RATIO[3:0]", 0x1E, RW,             "");
    VAR(MCU_CK_CONF, RCO_RATIO, "RCO_RATIO", 0x01, RW,                    "");
  );
  REG(XO_RCO_TEST, "XO_RCO_TEST", 0xB4, RW,                               "",
    VAR(XO_RCO_TEST, PD_CLKDIV, "PD_CLKDIV", 0x08, RW,                    "");
  );
  REG(SYNTH_CONFIG_0, "SYNTH_CONFIG[0]", 0x9F, RW,                        "",
    VAR(SYNTH_CONFIG_0, SEL_TSPLIT, "SEL_TSPLIT", 0x80, RW,               "");
  );
  REG(SYNTH_CONFIG_1, "SYNTH_CONFIG[1]", 0x9E, RW,                        "",
    VAR(SYNTH_CONFIG_1, REFDIV, "REFDIV", 0x80, RW,                       "");
    VAR(SYNTH_CONFIG_1, VCO_L_SEL, "VCO_L_SEL", 0x04, RW,                 "");
    VAR(SYNTH_CONFIG_1, VCO_H_SEL, "VCO_H_SEL", 0x02, RW,                 "");
  );
  REG(IF_OFFSET_ANA, "IF_OFFSET_ANA", 0x07, RW,                           "", );


  // Radio configuration (analog)
  REG(SYNT3, "SYNT3", 0x08, RW,                                           "",
    VAR(SYNT3, WCP_2_0, "WCP[2:0]", 0xE0, RW,                             "");
    VAR(SYNT3, SYNT_25_21, "SYNT[25:21]", 0x1F, RW,                       "");
  );
  REG(SYNT2, "SYNT2", 0x09, RW,                                           "",
    VAR(SYNT2, SYNT_20_13, "SYNT[20:13]", 0xFF, RW,                       "");
  );
  REG(SYNT1, "SYNT1", 0x0A, RW,                                           "",
    VAR(SYNT1, SYNT_12_5, "SYNT[12:5]", 0xFF, RW,                         "");
  );
  REG(SYNT0, "SYNT0", 0x0B, RW,                                           "",
    VAR(SYNT0, SYNT_4_0, "SYNT[4:0]", 0xF8, RW,                           "");
    VAR(SYNT0, BS, "BS", 0x07, RW,                                        "");
  );
  REG(CHSPACE, "CHSPACE", 0x0C, RW,                                       "",
    VAR(CHSPACE, CH_SPACING, "CH_SPACING", 0xFF, RW,                      "");
  );
  REG(IF_OFFSET_DIG, "IF_OFFSET_DIG", 0x0D, RW,                           "",);
  REG(FC_OFFSET_1, "FC_OFFSET_1", 0x0E, RW,                               "",
    VAR(FC_OFFSET_1, FC_OFFSET_11_8, "FC_OFFSET[11:8]", 0x0F, RW,         "");
  );
  REG(FC_OFFSET_0, "FC_OFFSET_0", 0x0F, RW,                               "",
    VAR(FC_OFFSET_0, FC_OFFSET_7_0, "FC_OFFSET[7:0]", 0xFF, RW,           "");
  );
  REG(PA_POWER_8, "PA_POWER_8", 0x10, RW,                                 "",
    VAR(PA_POWER_8, PA_LEVEL_7, "PA_LEVEL_7", 0xFF, RW,                   "");
  );
  REG(PA_POWER_7, "PA_POWER_7", 0x11, RW,                                 "",
    VAR(PA_POWER_7, PA_LEVEL_6, "PA_LEVEL_6", 0xFF, RW,                   "");
  );
  REG(PA_POWER_6, "PA_POWER_6", 0x12, RW,                                 "",
    VAR(PA_POWER_6, PA_LEVEL_5, "PA_LEVEL_5", 0xFF, RW,                   "");
  );
  REG(PA_POWER_5, "PA_POWER_5", 0x13, RW,                                 "",
    VAR(PA_POWER_5, PA_LEVEL_4, "PA_LEVEL_4", 0xFF, RW,                   "");
  );
  REG(PA_POWER_4, "PA_POWER_4", 0x14, RW,                                 "",
    VAR(PA_POWER_4, PA_LEVEL_3, "PA_LEVEL_3", 0xFF, RW,                   "");
  );
  REG(PA_POWER_3, "PA_POWER_3", 0x15, RW,                                 "",
    VAR(PA_POWER_3, PA_LEVEL_2, "PA_LEVEL_2", 0xFF, RW,                   "");
  );
  REG(PA_POWER_2, "PA_POWER_2", 0x16, RW,                                 "",
    VAR(PA_POWER_2, PA_LEVEL_1, "PA_LEVEL_1", 0xFF, RW,                   "");
  );
  REG(PA_POWER_1, "PA_POWER_1", 0x17, RW,                                 "",
    VAR(PA_POWER_1, PA_LEVEL_0, "PA_LEVEL_0", 0xFF, RW,                   "");
  );
  REG(PA_POWER_0, "PA_POWER_0", 0x18, RW,                                 "",
    VAR(PA_POWER_0, CWC_1_0, "CWC[1:0]", 0xC0, RW,                        "");
    VAR(PA_POWER_0, PA_RAMP_ENABLE, "PA_RAMP_ENABLE", 0x20, RW,           "");
    VAR(PA_POWER_0, PA_RAMP_STEP_WIDTH_1_0, "PA_RAMP_STEP_WIDTH[1:0]", 0x18, RW, "");
    VAR(PA_POWER_0, PA_LEVEL_MAX_INDEX_2_0, "PA_LEVEL_MAX_INDEX[2:0]", 0x7, RW, "");
  );


  // Radio configuration (digital)
  REG(MOD1, "MOD1", 0x1A, RW,                                             "",
    VAR(MOD1, DATARATE_M, "DATARATE_M", 0xFF, RW,                         "");
  );
  REG(MOD0, "MOD0", 0x1B, RW,                                             "",
    VAR(MOD0, CW, "CW", 0x80, RW,                                         "");
    VAR(MOD0, BT_SEL, "BT_SEL", 0x40, RW,                                 "");
    VAR(MOD0, MOD_TYPE_1_0, "MOD_TYPE[1:0]", 0x30, RW,                    "");
    VAR(MOD0, DATARATE_E, "DATARATE_E", 0x0F, RW,                         "");
  );
  REG(FDEV0, "FDEV0", 0x1C, RW,                                           "",
    VAR(FDEV0, FDEV_E, "FDEV_E", 0xF0, RW,                                "");
    VAR(FDEV0, CLOCK_REC_ALGO_SEL, "CLOCK_REC_ALGO_SEL", 0x08, RW,        "");
    VAR(FDEV0, FDEV_M, "FDEV_M", 0x07, RW,                                "");
  );
  REG(CHFLT, "CHFLT", 0x1D, RW,                                           "",
    VAR(CHFLT, CHFLT_M_3_0, "CHFLT_M[3:0]", 0xF0, RW,                     "");
    VAR(CHFLT, CHFLT_E_3_0, "CHFLT_E[3:0]", 0x0F, RW,                     "");
  );
  REG(AFC2, "AFC2", 0x1E, RW,                                             "",);
  REG(AFC1, "AFC1", 0x1F, RW,                                             "",);
  REG(AFC0, "AFC0", 0x20, RW,                                             "",);
  REG(RSSI_FLT, "RSSI_FLT", 0x21, RW,                                     "",);
  REG(RSSI_TH, "RSSI_TH", 0x22, RW,                                       "",);
  REG(CLOCKREC, "CLOCKREC", 0x23, RW,                                     "",);
  REG(AGCCTRL2, "AGCCTRL2", 0x24, RW,                                     "",);
  REG(AGCCTRL1, "AGCCTRL1", 0x25, RW,                                     "",);
  REG(AGCCTRL0, "AGCCTRL0", 0x26, RW,                                     "",);
  REG(ANT_SELECT_CONF, "ANT_SELECT_CONF", 0x27, RW,                       "",);


  // Packet/protocol configuration
  REG(PKTCTRL4, "PKTCTRL4", 0x30, RW,                                     "",
    VAR(PKTCTRL4, ADDRESS_LEN_1_0, "ADDRESS_LEN[1:0]", 0x18, RW,          "");
    VAR(PKTCTRL4, CONTROL_LEN, "CONTROL_LEN", 0x07, RW,                   "");
  );
  REG(PKTCTRL3, "PKTCTRL3", 0x31, RW,                                     "",
    VAR(PKTCTRL3, PCKT_FRMT_1_0, "PCKT_FRMT[1:0]", 0xC0, RW,              "");
    VAR(PKTCTRL3, RX_MODE_1_0, "RX_MODE[1:0]", 0x30, RW,                  "");
    VAR(PKTCTRL3, LEN_WID, "LEN_WID", 0x0F, RW,                           "");
  );
  REG(PKTCTRL2, "PKTCTRL2", 0x32, RW,                                     "",
    VAR(PKTCTRL2, PREAMBLE_LENGTH_4_0, "PREAMBLE_LENGTH[4:0]", 0xF8, RW,  "");
    VAR(PKTCTRL2, SYNC_LENGTH_1_0, "SYNC_LENGTH[1:0]", 0x06, RW,          "");
    VAR(PKTCTRL2, FIX_VAR_LEN, "FIX_VAR_LEN", 0x01, RW,                   "");
  );
  REG(PKTCTRL1, "PKTCTRL1", 0x33, RW,                                     "",
    VAR(PKTCTRL1, CRC_MODE_2_0, "CRC_MODE[2:0]", 0xE0, RW,                "");
    VAR(PKTCTRL1, WHIT_EN_0, "WHIT_EN[0]", 0x10, RW,                      "");
    VAR(PKTCTRL1, TXSOURCE_1_0, "TXSOURCE[1:0]", 0x0C, RW,                "");
    VAR(PKTCTRL1, FEC_EN, "FEC_EN", 0x01, RW,                             "");
  );
  REG(PCKTLEN1, "PCKTLEN1", 0x34, RW,                                      "",);
  REG(PCKTLEN0, "PCKTLEN0", 0x35, RW,                                       "",);
  REG(SYNC4, "SYNC4", 0x36, RW,                                           "",);
  REG(SYNC3, "SYNC3", 0x37, RW,                                           "",);
  REG(SYNC2, "SYNC2", 0x38, RW,                                           "",);
  REG(SYNC1, "SYNC1", 0x39, RW,                                           "",);
  REG(QI, "QI", 0x3A, RW,                                                 "",);
  REG(MBUS_PRMBL, "MBUS_PRMBL", 0x3B, RW,                                 "",);
  REG(MBUS_PSTMBL, "MBUS_PSTMBL", 0x3C, RW,                               "",);
  REG(MBUS_CTRL, "MBUS_CTRL", 0x3D, RW,                                   "",);
  REG(FIFO_CONFIG_3, "FIFO_CONFIG[3]", 0x3E, RW,                          "",);
  REG(FIFO_CONFIG_2, "FIFO_CONFIG[2]", 0x3F, RW,                          "",);
  REG(FIFO_CONFIG_1, "FIFO_CONFIG[1]", 0x40, RW,                          "",);
  REG(FIFO_CONFIG_0, "FIFO_CONFIG[0]", 0x41, RW,                          "",);
  REG(PCKT_FLT_GOALS_12, "PCKT_FLT_GOALS[12]", 0x42, RW,                  "",);
  REG(PCKT_FLT_GOALS_11, "PCKT_FLT_GOALS[11]", 0x43, RW,                  "",);
  REG(PCKT_FLT_GOALS_10, "PCKT_FLT_GOALS[10]", 0x44, RW,                  "",);
  REG(PCKT_FLT_GOALS_9, "PCKT_FLT_GOALS[9]", 0x45, RW,                    "",);
  REG(PCKT_FLT_GOALS_8, "PCKT_FLT_GOALS[8]", 0x46, RW,                    "",);
  REG(PCKT_FLT_GOALS_7, "PCKT_FLT_GOALS[7]", 0x47, RW,                    "",);
  REG(PCKT_FLT_GOALS_6, "PCKT_FLT_GOALS[6]", 0x48, RW,                    "",);
  REG(PCKT_FLT_GOALS_5, "PCKT_FLT_GOALS[5]", 0x49, RW,                    "",);
  REG(PCKT_FLT_GOALS_4, "PCKT_FLT_GOALS[4]", 0x4A, RW,                    "",);
  REG(PCKT_FLT_GOALS_3, "PCKT_FLT_GOALS[3]", 0x4B, RW,                    "",);
  REG(PCKT_FLT_GOALS_2, "PCKT_FLT_GOALS[2]", 0x4C, RW,                    "",);
  REG(PCKT_FLT_GOALS_1, "PCKT_FLT_GOALS[1]", 0x4D, RW,                    "",);
  REG(PCKT_FLT_GOALS_0, "PCKT_FLT_GOALS[0]", 0x4E, RW,                    "",);

  REG(PCKT_FLT_OPTIONS, "PCKT_FLT_OPTIONS", 0x4F, RW,                     "",);
  REG(PROTOCOL_2, "PROTOCOL[2]", 0x50, RW,                                "",);
  REG(PROTOCOL_1, "PROTOCOL[1]", 0x51, RW,                                "",);
  REG(PROTOCOL_0, "PROTOCOL[0]", 0x52, RW,                                "",);
  REG(TIMERS_5, "TIMERS[5]", 0x53, RW,                                    "",);
  REG(TIMERS_4, "TIMERS[4]", 0x54, RW,                                    "",);
  REG(TIMERS_3, "TIMERS[3]", 0x55, RW,                                    "",);
  REG(TIMERS_2, "TIMERS[2]", 0x56, RW,                                    "",);
  REG(TIMERS_1, "TIMERS[1]", 0x57, RW,                                    "",);
  REG(TIMERS_0, "TIMERS[0]", 0x58, RW,                                    "",);
  REG(CSMA_CONFIG_3, "CSMA_CONFIG[3]", 0x64, RW,                          "",);
  REG(CSMA_CONFIG_2, "CSMA_CONFIG[2]", 0x65, RW,                          "",);
  REG(CSMA_CONFIG_1, "CSMA_CONFIG[1]", 0x66, RW,                          "",);
  REG(CSMA_CONFIG_0, "CSMA_CONFIG[0]", 0x67, RW,                          "",);
  REG(TX_CTRL_FIELD_3, "TX_CTRL_FIELD[3]", 0x68, RW,                      "",);
  REG(TX_CTRL_FIELD_2, "TX_CTRL_FIELD[2]", 0x69, RW,                      "",);
  REG(TX_CTRL_FIELD_1, "TX_CTRL_FIELD[1]", 0x6A, RW,                      "",);
  REG(TX_CTRL_FIELD_0, "TX_CTRL_FIELD[0]", 0x6B, RW,                      "",);
  REG(PM_CONFIG_2, "PM_CONFIG[2]", 0xA4, RW,                              "",);
  REG(PM_CONFIG_1, "PM_CONFIG[1]", 0xA5, RW,                              "",);
  REG(PM_CONFIG_0, "PM_CONFIG[0]", 0xA6, RW,                              "",);
  REG(XO_RCO_CONFIG, "XO_RCO_CONFIG", 0xA7, RW,                           "",);
  REG(TEST_SELECT, "TEST_SELECT", 0xA8, RW,                               "",);
  REG(PM_TEST, "PM_TEST", 0xB2, RW,                                       "",);


  // Frequently used registers

  REG(CHNUM, "CHNUM", 0x6C, RW,                    "",);
  REG(VCO_CONFIG, "VCO_CONFIG", 0xA1, RW,                    "",);
  REG(RCO_VCO_CALIBR_IN_2, "RCO_VCO_CALIBR_IN[2]", 0x6D, RW,              "",);
  REG(RCO_VCO_CALIBR_IN_1, "RCO_VCO_CALIBR_IN[1]", 0x6E, RW,              "",);
  REG(RCO_VCO_CALIBR_IN_0, "RCO_VCO_CALIBR_IN[0]", 0x6F, RW,              "",);

  REG(AES_KEY_IN_15, "AES_KEY_IN[15]", 0x70, RW,                          "",);
  REG(AES_KEY_IN_14, "AES_KEY_IN[14]", 0x71, RW,                          "",);
  REG(AES_KEY_IN_13, "AES_KEY_IN[13]", 0x72, RW,                          "",);
  REG(AES_KEY_IN_12, "AES_KEY_IN[12]", 0x73, RW,                          "",);
  REG(AES_KEY_IN_11, "AES_KEY_IN[11]", 0x74, RW,                          "",);
  REG(AES_KEY_IN_10, "AES_KEY_IN[10]", 0x75, RW,                          "",);
  REG(AES_KEY_IN_9, "AES_KEY_IN[9]", 0x76, RW,                            "",);
  REG(AES_KEY_IN_8, "AES_KEY_IN[8]", 0x77, RW,                            "",);
  REG(AES_KEY_IN_7, "AES_KEY_IN[7]", 0x78, RW,                            "",);
  REG(AES_KEY_IN_6, "AES_KEY_IN[6]", 0x79, RW,                            "",);
  REG(AES_KEY_IN_5, "AES_KEY_IN[5]", 0x7A, RW,                            "",);
  REG(AES_KEY_IN_4, "AES_KEY_IN[4]", 0x7B, RW,                            "",);
  REG(AES_KEY_IN_3, "AES_KEY_IN[3]", 0x7C, RW,                            "",);
  REG(AES_KEY_IN_2, "AES_KEY_IN[2]", 0x7D, RW,                            "",);
  REG(AES_KEY_IN_1, "AES_KEY_IN[1]", 0x7E, RW,                            "",);
  REG(AES_KEY_IN_0, "AES_KEY_IN[0]", 0x7F, RW,                            "",);

  REG(AES_DATA_IN_15, "AES_DATA_IN[15]", 0x80, RW,                        "",);
  REG(AES_DATA_IN_14, "AES_DATA_IN[14]", 0x81, RW,                        "",);
  REG(AES_DATA_IN_13, "AES_DATA_IN[13]", 0x82, RW,                        "",);
  REG(AES_DATA_IN_12, "AES_DATA_IN[12]", 0x83, RW,                        "",);
  REG(AES_DATA_IN_11, "AES_DATA_IN[11]", 0x84, RW,                        "",);
  REG(AES_DATA_IN_10, "AES_DATA_IN[10]", 0x85, RW,                        "",);
  REG(AES_DATA_IN_9, "AES_DATA_IN[9]", 0x86, RW,                          "",);
  REG(AES_DATA_IN_8, "AES_DATA_IN[8]", 0x87, RW,                          "",);
  REG(AES_DATA_IN_7, "AES_DATA_IN[7]", 0x88, RW,                          "",);
  REG(AES_DATA_IN_6, "AES_DATA_IN[6]", 0x89, RW,                          "",);
  REG(AES_DATA_IN_5, "AES_DATA_IN[5]", 0x8A, RW,                          "",);
  REG(AES_DATA_IN_4, "AES_DATA_IN[4]", 0x8B, RW,                          "",);
  REG(AES_DATA_IN_3, "AES_DATA_IN[3]", 0x8C, RW,                          "",);
  REG(AES_DATA_IN_2, "AES_DATA_IN[2]", 0x8D, RW,                          "",);
  REG(AES_DATA_IN_1, "AES_DATA_IN[1]", 0x8E, RW,                          "",);
  REG(AES_DATA_IN_0, "AES_DATA_IN[0]", 0x8F, RW,                          "",);

  REG(IRQ_MASK_3, "IRQ_MASK[3]", 0x90, RW,                                "",);
  REG(IRQ_MASK_2, "IRQ_MASK[2]", 0x91, RW,                                "",);
  REG(IRQ_MASK_1, "IRQ_MASK[1]", 0x92, RW,                                "",);
  REG(IRQ_MASK_0, "IRQ_MASK[0]", 0x93, RW,                                "",);

  REG(DEM_CONFIG, "DEM_CONFIG", 0xA3, RW,                                 "",);
  REG(PM_CONFIG, "PM_CONFIG", 0xA4, RW,                                   "",);
  REG(MC_STATE_1, "MC_STATE[1]", 0xC0, RO,                                "",
    VAR(MC_STATE_1, ANT_SELECT, "ANT_SELECT", 0x08, RW,                   "");
    VAR(MC_STATE_1, TX_FIFO_FULL, "TX_FIFO_FULL", 0x04, RW,               "");
    VAR(MC_STATE_1, RX_FIFO_EMPTY, "RX_FIFO_EMPTY", 0x02, RW,             "");
    VAR(MC_STATE_1, ERROR_LOCK, "ERROR_LOCK", 0x01, RW,                   "");
  );
  REG(MC_STATE_0, "MC_STATE[0]", 0xC1, RO,                                "",
    VAR(MC_STATE_0, STATE_6_0, "STATE[6:0]", 0xFE, RW,                    "");
    VAR(MC_STATE_0, XO_ON, "XO_ON", 0x01, RW,                             "");
  );
  REG(TX_PCKT_INFO, "TX_PCKT_INFO", 0xC2, RO,                             "",);
  REG(RX_PCKT_INFO, "RX_PCKT_INFO", 0xC3, RO,                             "",);
  REG(AFC_CORR, "AFC_CORR", 0xC4, RO,                                     "",);
  REG(LINK_QUALIF_2, "LINK_QUALIF[2]", 0xC5, RO,                          "",);
  REG(LINK_QUALIF_1, "LINK_QUALIF[1]", 0xC6, RO,                          "",);
  REG(LINK_QUALIF_0, "LINK_QUALIF[0]", 0xC7, RO,                          "",);
  REG(RSSI_LEVEL, "RSSI_LEVEL", 0xC8, RO,                                 "",);
  REG(RX_PCKT_LEN_1, "RX_PCKT_LEN[1]", 0xC9, RO,                          "",);
  REG(RX_PCKT_LEN_0, "RX_PCKT_LEN[0]", 0xCA, RO,                          "",);
  REG(CRC_FIELD_2, "CRC_FIELD[2]", 0xCB, RO,                              "",);
  REG(CRC_FIELD_1, "CRC_FIELD[1]", 0xCC, RO,                              "",);
  REG(CRC_FIELD_0, "CRC_FIELD[0]", 0xCD, RO,                              "",);
  REG(RX_CTRL_FIELD_3, "RX_CTRL_FIELD[3]", 0xCE, RO,                      "",);
  REG(RX_CTRL_FIELD_2, "RX_CTRL_FIELD[2]", 0xCF, RO,                      "",);
  REG(RX_CTRL_FIELD_1, "RX_CTRL_FIELD[1]", 0xD0, RO,                      "",);
  REG(RX_CTRL_FIELD_0, "RX_CTRL_FIELD[0]", 0xD1, RO,                      "",);
  REG(RX_ADDR_FIELD_1, "RX_ADDR_FIELD[1]", 0xD2, RO,                      "",);
  REG(RX_ADDR_FIELD_0, "RX_ADDR_FIELD[0]", 0xD3, RO,                      "",);

  REG(AES_DATA_OUT_15, "AES_DATA_OUT[15]", 0xD4, RO,                      "",);
  REG(AES_DATA_OUT_14, "AES_DATA_OUT[14]", 0xD5, RO,                      "",);
  REG(AES_DATA_OUT_13, "AES_DATA_OUT[13]", 0xD6, RO,                      "",);
  REG(AES_DATA_OUT_12, "AES_DATA_OUT[12]", 0xD7, RO,                      "",);
  REG(AES_DATA_OUT_11, "AES_DATA_OUT[11]", 0xD8, RO,                      "",);
  REG(AES_DATA_OUT_10, "AES_DATA_OUT[10]", 0xD9, RO,                      "",);
  REG(AES_DATA_OUT_9, "AES_DATA_OUT[9]", 0xDA, RO,                        "",);
  REG(AES_DATA_OUT_8, "AES_DATA_OUT[8]", 0xDB, RO,                        "",);
  REG(AES_DATA_OUT_7, "AES_DATA_OUT[7]", 0xDC, RO,                        "",);
  REG(AES_DATA_OUT_6, "AES_DATA_OUT[6]", 0xDD, RO,                        "",);
  REG(AES_DATA_OUT_5, "AES_DATA_OUT[5]", 0xDE, RO,                        "",);
  REG(AES_DATA_OUT_4, "AES_DATA_OUT[4]", 0xDF, RO,                        "",);
  REG(AES_DATA_OUT_3, "AES_DATA_OUT[3]", 0xE0, RO,                        "",);
  REG(AES_DATA_OUT_2, "AES_DATA_OUT[2]", 0xE1, RO,                        "",);
  REG(AES_DATA_OUT_1, "AES_DATA_OUT[1]", 0xE2, RO,                        "",);
  REG(AES_DATA_OUT_0, "AES_DATA_OUT[0]", 0xE3, RO,                        "",);

  REG(RCO_VCO_CALIBR_OUT_1, "RCO_VCO_CALIBR_OUT[1]", 0xE4, RO,            "",);
  REG(RCO_VCO_CALIBR_OUT_0, "RCO_VCO_CALIBR_OUT[0]", 0xE5, RO,            "",);
  REG(LINEAR_FIFO_STATUS_1, "LINEAR_FIFO_STATUS[1]", 0xE6, RO,            "",);
  REG(LINEAR_FIFO_STATUS_0, "LINEAR_FIFO_STATUS[0]", 0xE7, RO,            "",);

  REG(IRQ_STATUS_3, "IRQ_STATUS[3]", 0xFA, RO,                            "",);
  REG(IRQ_STATUS_2, "IRQ_STATUS[2]", 0xFB, RO,                            "",);
  REG(IRQ_STATUS_1, "IRQ_STATUS[1]", 0xFC, RO,                            "",);
  REG(IRQ_STATUS_0, "IRQ_STATUS[0]", 0xFD, RO,                            "",);


  // General information

  REG(DEVICE_INFO1, "DEVICE_INFO1", 0xF0, RO,                             "",
    VAR(DEVICE_INFO1, PARTNUM_7_0, "PARTNUM[7:0]", 0xFF, RO,              "");
  );
  REG(DEVICE_INFO2, "DEVICE_INFO2", 0xF1, RO,                             "",
    VAR(DEVICE_INFO2, VERSION_7_0, "VERSION[7:0]", 0xFF, RO,              "");
  );
);

#undef REG
#undef VAR

#endif // _SPIRIT1_RT_H_