// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _MCP9808_RT_H_
#define _MCP9808_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint16_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint16_t, R, N, P, M, RW, D)

REGISTER_TABLE_W(MCP9808, RegisterTable, uint8_t, uint16_t,
  REG(RFU, "RFU", 0x00, RO,                                     "Reserved for future use", );
  REG(Configuration, "Configuration", 0x01, RW,                 "Configuration",
    VAR(Configuration, T_HYST, "T_HYST", 0x0600, RW,            "");
    VAR(Configuration, SHDN, "SHDN", 0x0100, RW,                "");
    VAR(Configuration, CritLock, "Crit. Lock", 0x0080, RW,      "");
    VAR(Configuration, WinLock, "Win. Lock", 0x0040, RW,        "");
    VAR(Configuration, IntClear, "Int. Clear", 0x0020, RW,      "");
    VAR(Configuration, AlertStat, "Alert Stat.", 0x0010, RW,    "");
    VAR(Configuration, AlertCnt, "Alert Cnt.", 0x0008, RW,      "");
    VAR(Configuration, AlertSel, "Alert Sel.", 0x0004, RW,      "");
    VAR(Configuration, AlertPol, "Alert Pol.", 0x0002, RW,      "");
    VAR(Configuration, AlertMod, "Alert Mod.", 0x0001, RW,      "");
  );
  REG(T_UPPER_, "T_UPPER", 0x02, RW,                            "Alert Temperature Upper Boundary",
    VAR(T_UPPER_, T_UPPER, "T_UPPER", 0x1FFC, RW,               "");
  );
  REG(T_LOWER_, "T_LOWER", 0x03, RW,                            "Alert Temperature Lower Boundary",
    VAR(T_LOWER_, T_LOWER, "T_LOWER", 0x1FFC, RW,               "");
  );
  REG(T_CRIT_, "T_CRIT", 0x04, RW,                              "Critical Temperature",
    VAR(T_CRIT_, T_CRIT, "T_CRIT", 0x1FFC, RW,                  "");
  );
  REG(T_A, "T_A", 0x05, RO,                                     "Ambient Temperature",
    VAR(T_A, T_A_GTE_T_CRIT, "T_A >= T_CRIT", 0x8000, RO,       "");
    VAR(T_A, T_A_GT_T_UPPER, "T_A > T_UPPER", 0x4000, RO,       "");
    VAR(T_A, T_A_GT_T_LOWER, "T_A > T_LOWER", 0x2000, RO,       "");
    VAR(T_A, Temperature, "Temperature", 0x1FFF, RO,            "");
  );
  REG(ManufacturerID, "Manufacturer ID", 0x06, RO,              "Manufacturer ID", );
  REG(DeviceIDRev, "Device ID/Revision", 0x07, RO,              "Device ID/Revision",
    VAR(DeviceIDRev, DeviceID, "Device ID", 0xFF00, RO,         "");
    VAR(DeviceIDRev, DeviceRev, "Device Revision", 0x00FF, RO,  "");
  );
  REG(Resolution_, "Resolution_", 0x08, RW,                      "Resolution",
    VAR(Resolution_, Resolution, "Resolution", 0x0300, RW,      "");
  );

  // 0x09-0xFF: Reserved; do not read or write.
);

#endif // _MCP9808_RT_H_
