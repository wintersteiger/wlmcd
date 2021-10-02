// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _INA219_RT_H_
#define _INA219_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint16_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint16_t, R, N, P, M, RW, D)

REGISTER_TABLE(INA219, RegisterTable, uint8_t, uint16_t,
  REG(Configuration, "Configuration", 0x00, RW,               "All-register reset, settings for bus voltage range, PGA "
                                                              "Gain, ADC resolution/averaging.",
    VAR(Configuration, RST, "RST", 0x8000, RW,                "");
    VAR(Configuration, BRNG, "BRNG", 0x2000, RW,              "");
    VAR(Configuration, PG, "PG", 0x1800, RW,                  "");
    VAR(Configuration, BADC, "BADC", 0x0780, RW,              "");
    VAR(Configuration, SADC, "SADC", 0x0078, RW,              "");
    VAR(Configuration, Mode, "Mode", 0x0007, RW,              "");
  );
  REG(ShuntVoltage, "Shunt voltage", 0x01, RO,                "Shunt voltage measurement data.",
  );
  REG(BusVoltage, "Bus voltage", 0x02, RO,                    "Bus voltage measurement data.",
    VAR(BusVoltage, BD_12_0, "BD[12:0]", 0xFFF8, RO,          "");
    VAR(BusVoltage, CNVR, "CNVR", 0x0002, RO,                 "");
    VAR(BusVoltage, OVF, "OVF", 0x0001, RO,                   "");
  );
  REG(Power, "Power", 0x03, RO,                               "Power measurement data.",
  );
  REG(Current, "Current", 0x04, RO,                           "Contains the value of the current flowing through the shunt resistor.",
  );
  REG(Calibration, "Calibration", 0x05, RW,                   "Sets full-scale range and LSB of current and power measurements. Overall system calibration.",
    VAR(Calibration, FS_15_1, "FS[15:1]", 0xFFFE, RW,         "");
  );
);

#undef REG
#undef VAR

#endif // _INA219_RT_H_
