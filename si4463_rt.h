// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SI4463_NORMAL_RT_
#define _SI4463_NORMAL_RT_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)
#define PREG(N, P, A, RW, H, V) REGDECL(uint16_t, uint8_t, N, P, A, RW, H, V)
#define PVAR(R, N, P, M, RW, D) VARDECL(uint16_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE_SET(SI4463, SI4463, RegisterTableSet,

  REGISTER_TABLE_SET_TABLE(SI4463, PartInfo, uint8_t, uint8_t,
    REG(CHIPREV,    "Chip rev",     0x01, RO, "", );
    REG(PART_15_8,  "Part[15:8]",   0x02, RO, "", );
    REG(PART_7_0,   "Part[7:0]",    0x03, RO, "", );
    REG(PBUILD,     "Part build",   0x04, RO, "", );
    REG(ID_15_8,    "ID[15:8]",     0x05, RO, "", );
    REG(ID_7_0,     "ID[7:0]",      0x06, RO, "", );
    REG(CUSTOMER,   "Customer",     0x07, RO, "", );
    REG(ROMID,      "ROM ID",       0x08, RO, "", );
  );

  REGISTER_TABLE_SET_TABLE(SI4463, FuncInfo, uint8_t, uint8_t,
    REG(REVEXT,     "Ext rev",      0x01, RO, "", );
    REG(REVBRANCH,  "Branch rev",   0x02, RO, "", );
    REG(REVINT,     "Int rev",      0x03, RO, "", );
    REG(PATCH_15_8, "Patch[15:8]",  0x04, RO, "", );
    REG(PATCH_7_0,  "Patch[7:0]",   0x05, RO, "", );
    REG(FUNC,       "ID[7:0]",      0x06, RO, "", );
  );

  REGISTER_TABLE_SET_TABLE(SI4463, GPIO, uint8_t, uint8_t,
    REG(GPIO0,      "GPIO[0]",      0x01, RO, "", );
    REG(GPIO1,      "GPIO[1]",      0x02, RO, "", );
    REG(GPIO2,      "GPIO[2]",      0x03, RO, "", );
    REG(GPIO3,      "GPIO[3]",      0x04, RO, "", );
    REG(NIRQ,       "NIRQ",         0x05, RO, "", );
    REG(SDO,        "SDO",          0x06, RO, "", );
    REG(GEN_CONFIG, "GEN_CONFIG",   0x07, RO, "", );
  );

  REGISTER_TABLE_SET_TABLE(SI4463, FIFO, uint8_t, uint8_t,
    REG(RX_FIFO_COUNT,  "RX FIFO count",  0x01, RO, "", );
    REG(TX_FIFO_SPACE,  "TX FIFO space",  0x02, RO, "", );
  );

  REGISTER_TABLE_SET_TABLE(SI4463, DeviceState, uint8_t, uint8_t,
    REG(CURR_STATE,       "State",    0x01, RO, "", );
    REG(CURRENT_CHANNEL,  "Channel",  0x02, RO, "", );
  );

  REGISTER_TABLE_SET_TABLE(SI4463, Interrupts, uint8_t, uint8_t,
    REG(INT_PEND,       "INT_PEND",     0x01, RO, "", );
    REG(INT_STATUS,     "INT_STATUS",   0x02, RO, "", );
    REG(PH_PEND,        "PH_PEND",      0x03, RO, "", );
    REG(PH_STATUS,      "PH_STATUS",    0x04, RO, "", );
    REG(MODEM_PEND,     "MODEM_PEND",   0x05, RO, "", );
    REG(MODEM_STATUS,   "MODEM_STATUS", 0x06, RO, "", );
    REG(CHIP_PEND,      "CHIP_PEND",    0x07, RO, "", );
    REG(CHIP_STATUS,    "CHIP_STATUS",  0x08, RO, "", );
  );

  REGISTER_TABLE_SET_TABLE(SI4463, PacketInfo, uint8_t, uint8_t,
    REG(LENGTH_15_8,    "LENGTH[15:8]", 0x01, RO, "", );
    REG(LENGTH_7_0,     "LENGTH[7:0]",  0x02, RO, "", );
  );

  REGISTER_TABLE_SET_TABLE(SI4463, ModemStatus, uint8_t, uint8_t,
    REG(MODEM_PEND, "MODEM_PEND", 0x01, RO, "",
      VAR(MODEM_PEND, RSSI_LATCH_PEND,        "RSSI_LATCH_PEND",        0x80, RO, "");
      VAR(MODEM_PEND, POSTAMBLE_DETECT_PEND,  "POSTAMBLE_DETECT_PEND",  0x40, RO, "");
      VAR(MODEM_PEND, INVALID_SYNC_PEND,      "INVALID_SYNC_PEND",      0x20, RO, "");
      VAR(MODEM_PEND, RSSI_JUMP_PEND,         "RSSI_JUMP_PEND",         0x10, RO, "");
      VAR(MODEM_PEND, RSSI_PEND,              "RSSI_PEND",              0x08, RO, "");
      VAR(MODEM_PEND, INVALID_PREAMBLE_PEND,  "INVALID_PREAMBLE_PEND",  0x04, RO, "");
      VAR(MODEM_PEND, PREAMBLE_DETECT_PEND,   "PREAMBLE_DETECT_PEND",   0x02, RO, "");
      VAR(MODEM_PEND, SYNC_DETECT_PEND,       "SYNC_DETECT_PEND",       0x01, RO, "");
    );
    REG(MODEM_STATUS, "MODEM_STATUS", 0x02, RO, "",
      VAR(MODEM_PEND, RSSI_LATCH,             "RSSI_LATCH",             0x80, RO, "");
      VAR(MODEM_PEND, POSTAMBLE_DETECT,       "POSTAMBLE_DETECT",       0x40, RO, "");
      VAR(MODEM_PEND, INVALID_SYNC,           "INVALID_SYNC",           0x20, RO, "");
      VAR(MODEM_PEND, RSSI_JUMP,              "RSSI_JUMP",              0x10, RO, "");
      VAR(MODEM_PEND, RSSI,                   "RSSI",                   0x08, RO, "");
      VAR(MODEM_PEND, INVALID_PREAMBLE,       "INVALID_PREAMBLE",       0x04, RO, "");
      VAR(MODEM_PEND, PREAMBLE_DETECT,        "PREAMBLE_DETECT",        0x02, RO, "");
      VAR(MODEM_PEND, SYNC_DETECT,            "SYNC_DETECT",            0x01, RO, "");
    );
    REG(CURR_RSSI,            "CURR_RSSI",              0x03, RO, "", );
    REG(LATCH_RSSI,           "LATCH_RSSI",             0x04, RO, "", );
    REG(ANT1_RSSI,            "ANT1_RSSI",              0x05, RO, "", );
    REG(ANT2_RSSI,            "ANT2_RSSI",              0x06, RO, "", );
    REG(AFC_FREQ_OFFSET_15_8, "AFC_FREQ_OFFSET[15:8]",  0x07, RO, "", );
    REG(AFC_FREQ_OFFSET_7_0,  "AFC_FREQ_OFFSET[7:0]",   0x08, RO, "", );
  );

  REGISTER_TABLE_SET_TABLE(SI4463, ADC, uint8_t, uint8_t,
    REG(GPIO_ADC_10_8,    "GPIO_ADC[10:8]",     0x01, RO, "", );
    REG(GPIO_ADC_7_0,     "GPIO_ADC[7:0]",      0x02, RO, "", );
    REG(BATTERY_ADC_10_8, "BATTERY_ADC[10:8]",  0x03, RO, "", );
    REG(BATTERY_ADC_7_0,  "BATTERY_ADC[7:0]",   0x04, RO, "", );
    REG(TEMP_10_8,        "TEMP[10:8]",         0x05, RO, "", );
    REG(TEMP_7_0,         "TEMP[7:0]",          0x06, RO, "", );
  );

  REGISTER_TABLE_SET_TABLE_W(SI4463, Property, uint16_t, uint8_t,
    std::vector<uint8_t> DeviceStateBuffer;
  );

);

#undef REG
#undef VAR
#undef PREG
#undef PVAR

#endif
