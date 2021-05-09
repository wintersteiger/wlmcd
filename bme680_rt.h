// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _BME680_RT_H_
#define _BME680_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE_W(BME680, RegisterTable, uint8_t, uint8_t,
  REG(Status, "Status", 0x73, RW,                                 "Status",
    VAR(Status, spi_mem_page, "spi_mem_page", 0x10, RO,           "");
  );
  REG(Reset, "Reset", 0xE0, RW,                                   "Reset",
    VAR(Reset, reset_7_0, "reset[7:0]", 0xFF, RW,                 "");
  );
  REG(Id, "Id", 0xD0, RO,                                         "Id",
    VAR(Id, chip_id_7_0, "chip_id[7:0]", 0xFF, RO,                "");
  );
  REG(Config, "Config", 0x75, RW,                                 "Config",
    VAR(Config, filter_2_0, "filter[2:0]", 0x3C, RW,              "");
    VAR(Config, spi_3w_en, "spi_3w_en", 0x01, RW,                 "");
  );
  REG(Ctrl_meas, "Ctrl_meas", 0x74, RW,                           "Ctrl_meas",
    VAR(Ctrl_meas, osrs_t_2_0, "osrs_t[2:0]", 0xE0, RW,           "");
    VAR(Ctrl_meas, osrs_p_2_0, "osrs_p[2:0]", 0x1C, RW,           "");
    VAR(Ctrl_meas, mode_1_0, "mode[1:0]", 0x03, RW,               "");
  );
  REG(Ctrl_hum, "Ctrl_hum", 0x72, RW,                             "Ctrl_hum",
    VAR(Ctrl_hum, spi_3w_int_en, "spi_3w_int_en", 0x40, RW,       "");
    VAR(Ctrl_hum, osrs_h_2_0, "osrs_h[2:0]", 0x07, RW,            "");
  );
  REG(Ctrl_gas_1, "Ctrl_gas_1", 0x71, RW,                         "Ctrl_gas_1",
    VAR(Ctrl_gas_1, run_gas, "run_gas", 0x10, RW,                 "");
    VAR(Ctrl_gas_1, nb_conv_3_0, "nb_conv[3:0]", 0x0F, RW,        "");
  );
  REG(Ctrl_gas_0, "Ctrl_gas_0", 0x70, RW,                         "Ctrl_gas_0",
    VAR(Ctrl_gas_0, heat_off, "heat_off", 0x08, RW,               "");
  );

  #define TEN(X) X(9) X(8) X(7) X(6) X(5) X(4) X(3) X(2) X(1) X(0)

  #define GAS_WAIT(X) \
    REG(Gas_wait_##X, "Gas_wait_"#X, 0x64 + (X), RW, "Gas_wait_"#X, \
      VAR(Gas_wait_##X, Gas_wait_##X##_7_0, "Gas_wait_" #X "[7:0]", 0xFF, RW, ""));

  #define RES_HEAT(X) \
    REG(Res_heat_##X, "Res_heat_"#X, 0x5A + (X), RW, "Res_heat_"#X, \
      VAR(Res_heat_##X, Res_heat_##X##_7_0, "Res_heat_" #X "[7:0]", 0xFF, RW, ""));

  #define IDAC_HEAT(X) \
    REG(Idac_heat_##X, "Idac_heat_"#X, 0x50 + (X), RW, "Idac_heat_"#X, \
      VAR(Idac_heat_##X, Idac_heat_##X##_7_0, "Idac_heat_" #X "[7:0]", 0xFF, RW, ""));

  TEN(GAS_WAIT);
  TEN(RES_HEAT);
  TEN(IDAC_HEAT);

  REG(gas_r_lsb, "gas_r_lsb", 0x2B, RO,                           "gas_r_lsb",
    VAR(gas_r_lsb, gas_r_1_0, "gas_r_[1:0]", 0xC0, RO,            "");
    VAR(gas_r_lsb, gas_valid_r, "gas_valid_r", 0x20, RO,          "");
    VAR(gas_r_lsb, heat_stab_r, "heat_stab_r", 0x10, RO,          "");
    VAR(gas_r_lsb, gas_range_r, "gas_range_r", 0x0F, RO,          "");
  );
  REG(gas_r_msb, "gas_r_msb", 0x2A, RO,                           "gas_r_msb",
    VAR(gas_r_msb, gas_r_9_2, "gas_r_[9:2]", 0xFF, RO,            "");
  );

  uint32_t GasResistance() { return gas_r_9_2() << 2 | gas_r_1_0(); }

  REG(hum_lsb, "hum_lsb", 0x26, RO,                               "hum_lsb",
    VAR(hum_lsb, hum_lsb_7_0, "hum_lsb[7:0]", 0xFF, RO,           "");
  );
  REG(hum_msb, "hum_msb", 0x25, RO,                               "hum_msb",
    VAR(hum_msb, hum_msb_7_0, "hum_msb[7:0]", 0xFF, RO,           "");
  );

  uint32_t Humidity() { return hum_msb() << 8 | hum_lsb(); }

  REG(temp_xlsb, "temp_xlsb", 0x24, RO,                           "temp_xlsb",
    VAR(temp_xlsb, temp_xlsb_7_4, "temp_xlsb[7:4]", 0xF0, RO,       "");
  );
  REG(temp_lsb, "temp_lsb", 0x23, RO,                             "temp_lsb",
    VAR(temp_lsb, temp_lsb_7_0, "temp_lsb[7:0]", 0xFF, RO,        "");
  );
  REG(temp_msb, "temp_msb", 0x22, RO,                             "temp_msb",
    VAR(temp_msb, temp_msb_7_0, "temp_msb[7:0]", 0xFF, RO,        "");
  );

  uint32_t Temperature() { return (temp_msb_7_0() << 12) | (temp_lsb_7_0() << 4) | temp_xlsb_7_4(); }

  REG(press_xlsb, "press_xlsb", 0x21, RO,                         "press_xlsb",
    VAR(press_xlsb, press_xlsb_7_4, "press_xlsb[7:4]", 0xF0, RO,  "");
  );
  REG(press_lsb, "press_lsb", 0x20, RO,                           "press_lsb",
    VAR(press_lsb, press_lsb_7_0, "press_lsb[7:0]", 0xFF, RO,     "");
  );
  REG(press_msb, "press_msb", 0x1F, RO,                           "press_msb",
    VAR(press_msb, press_msb_7_0, "press_msb[7:0]", 0xFF, RO,     "");
  );

  uint32_t Pressure() { return (press_msb_7_0() << 12) | (press_lsb_7_0() << 4) | press_xlsb_7_4(); }

  REG(eas_status_0, "eas_status_0", 0x1D, RO,                     "eas_status_0",
    VAR(eas_status_0, new_data_0, "new_data_0", 0x80, RO,         "");
    VAR(eas_status_0, gas_measuring, "gas_measuring", 0x40, RO,   "");
    VAR(eas_status_0, measuring, "measuring", 0x20, RO,           "");
    VAR(eas_status_0, gas_meas_index_0_3_0, "gas_meas_index_0[3:0]", 0x0F, RO, "");
  );
);

#undef REG
#undef VAR

#endif // _BME680_RT_H_
