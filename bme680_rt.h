// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _BME680_RT_H_
#define _BME680_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

static const float lookup_k1_range[16] = {  0.0, 0.0,  0.0,  0.0, 0.0, -1.0, 0.0, -0.8,
                                            0.0, 0.0, -0.2, -0.5, 0.0, -1.0, 0.0,  0.0 };
static const float lookup_k2_range[16] = {  0.0, 0.0,  0.0,  0.0, 0.1,  0.7, 0.0, -0.8,
                                           -0.1, 0.0,  0.0,  0.0, 0.0,  0.0, 0.0,  0.0 };

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

  #define TEN(X) X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9)

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

  REG(hum_lsb, "hum_lsb", 0x26, RO,                               "hum_lsb",
    VAR(hum_lsb, hum_7_0, "hum[7:0]", 0xFF, RO,                   "");
  );
  REG(hum_msb, "hum_msb", 0x25, RO,                               "hum_msb",
    VAR(hum_msb, hum_15_8, "hum[15:8]", 0xFF, RO,                 "");
  );

  REG(temp_xlsb, "temp_xlsb", 0x24, RO,                           "temp_xlsb",
    VAR(temp_xlsb, temp_3_0, "temp[3:0]", 0xF0, RO,               "");
  );
  REG(temp_lsb, "temp_lsb", 0x23, RO,                             "temp_lsb",
    VAR(temp_lsb, temp_11_4, "temp[11:4]", 0xFF, RO,              "");
  );
  REG(temp_msb, "temp_msb", 0x22, RO,                             "temp_msb",
    VAR(temp_msb, temp_19_12, "temp[19:12]", 0xFF, RO,            "");
  );

  REG(press_xlsb, "press_xlsb", 0x21, RO,                         "press_xlsb",
    VAR(press_xlsb, press_3_0, "press[3:0]", 0xF0, RO,  "");
  );
  REG(press_lsb, "press_lsb", 0x20, RO,                           "press_lsb",
    VAR(press_lsb, press_11_4, "press[11:4]", 0xFF, RO,     "");
  );
  REG(press_msb, "press_msb", 0x1F, RO,                           "press_msb",
    VAR(press_msb, press_19_12, "press[19:12]", 0xFF, RO,     "");
  );

  REG(meas_status_0, "meas_status_0", 0x1D, RO,                   "meas_status_0",
    VAR(meas_status_0, new_data_0, "new_data_0", 0x80, RO,        "");
    VAR(meas_status_0, gas_measuring, "gas_measuring", 0x40, RO,  "");
    VAR(meas_status_0, measuring, "measuring", 0x20, RO,          "");
    VAR(meas_status_0, gas_meas_index_0_3_0, "gas_meas_index_0[3:0]", 0x0F, RO, "");
  );

  // Temperature calibration coefficients
  REG(par_t1_lsb, "par_t1_lsb", 0xE9, RO, "par_t1_lsb", );
  REG(par_t1_msb, "par_t1_msb", 0xEA, RO, "par_t1_msb", );
  uint16_t par_t1() const { return par_t1_msb() << 8 | par_t1_lsb(); }
  REG(par_t2_lsb, "par_t2_lsb", 0x8A, RO, "par_t2_lsb", );
  REG(par_t2_msb, "par_t2_msb", 0x8B, RO, "par_t2_msb", );
  int16_t par_t2() const { return par_t2_msb() << 8 | par_t2_lsb(); }
  REG(par_t3_, "par_t3", 0x8C, RO, "par_t3", );
  int8_t par_t3() const { return par_t3_(); }

  // Pressure calibration coefficients
  REG(par_p1_lsb, "par_p1_lsb", 0x8E, RO, "par_p1_lsb", );
  REG(par_p1_msb, "par_p1_msb", 0x8F, RO, "par_p1_msb", );
  uint16_t par_p1() const { return par_p1_msb() << 8 | par_p1_lsb(); }
  REG(par_p2_lsb, "par_p2_lsb", 0x90, RO, "par_p2_lsb", );
  REG(par_p2_msb, "par_p2_msb", 0x91, RO, "par_p2_msb", );
  int16_t par_p2() const { return par_p2_msb() << 8 | par_p2_lsb(); }
  REG(par_p3_, "par_p3_", 0x92, RO, "par_p3_", );
  int8_t par_p3() const { return par_p3_(); }
  REG(par_p4_lsb, "par_p4_lsb", 0x94, RO, "par_p4_lsb", );
  REG(par_p4_msb, "par_p4_msb", 0x95, RO, "par_p4_msb", );
  int16_t par_p4() const { return par_p4_msb() << 8 | par_p4_lsb(); }
  REG(par_p5_lsb, "par_p5_lsb", 0x94, RO, "par_p5_lsb", );
  REG(par_p5_msb, "par_p5_msb", 0x95, RO, "par_p5_msb", );
  int16_t par_p5() const { return par_p5_msb() << 8 | par_p5_lsb(); }
  REG(par_p6_, "par_p6_", 0x99, RO, "par_p6_", );
  int8_t par_p6() const { return par_p6_(); }
  REG(par_p7_, "par_p7_", 0x98, RO, "par_p7_", );
  int8_t par_p7() const { return par_p7_(); }
  REG(par_p8_lsb, "par_p8_lsb", 0x9C, RO, "par_p8_lsb", );
  REG(par_p8_msb, "par_p8_msb", 0x9D, RO, "par_p8_msb", );
  int16_t par_p8() const { return par_p8_msb() << 8 | par_p8_lsb(); }
  REG(par_p9_lsb, "par_p9_lsb", 0x9E, RO, "par_p9_lsb", );
  REG(par_p9_msb, "par_p9_msb", 0x9F, RO, "par_p9_msb", );
  int16_t par_p9() const { return par_p9_msb() << 8 | par_p9_lsb(); }
  REG(par_p10_, "par_p10_", 0xA0, RO, "par_p10_", );
  uint8_t par_p10() const { return par_p10_(); }

  // Humidity calibration coefficients
  REG(par_h1_2, "par_h1_2", 0xE2, RO, "par_h1_2",
    VAR(par_h1_2, par_h1_3_0, "par_h1[3:0]", 0x0F, RO, "");
    VAR(par_h1_2, par_h2_3_0, "par_h2[3:0]", 0xF0, RO, "");
  );
  REG(par_h1_, "par_h1", 0xE3, RO, "par_h1",
    VAR(par_h1_, par_h1_11_4, "par_h1[11:4]", 0xFF, RO, "");
  );
  uint16_t par_h1() { return (par_h1_11_4() << 4) | par_h1_3_0(); }
  REG(par_h2_, "par_h2", 0xE1, RO, "par_h2",
    VAR(par_h2_, par_h2_11_4, "par_h2[11:4]", 0xFF, RO, "");
  );
  uint16_t par_h2() { return (par_h2_11_4() << 4) | par_h2_3_0(); }
  REG(par_h3, "par_h3", 0xE4, RO, "par_h3",);
  REG(par_h4, "par_h4", 0xE5, RO, "par_h4",);
  REG(par_h5, "par_h5", 0xE6, RO, "par_h5",);
  REG(par_h6, "par_h6", 0xE7, RO, "par_h6",);
  REG(par_h7, "par_h7", 0xE8, RO, "par_h7",);

  // Gas pressure calibration coefficients
  REG(par_g1, "par_g1", 0xED, RO, "par_g1", );
  REG(par_g2_lsb, "par_g2_lsb", 0xEB, RO, "par_g2_lsb", );
  REG(par_g2_msb, "par_g2_msb", 0xEC, RO, "par_g2_msb", );
  int16_t par_g2() { return (par_g2_msb() << 8) | par_g2_lsb(); }
  REG(par_g3, "par_g3", 0xEE, RO, "par_g3", );
  REG(res_heat_range_, "res_heat_range_", 0x02, RO, "",
    VAR(res_heat_range_, res_heat_range, "res_heat_range", 0x30, RO, "");
  );
  REG(res_heat_val, "res_heat_val", 0x00, RO, "res_heat_val", );
  REG(range_switching_error, "range_switching_error", 0x04, RO, "range_switching_error", );


  int32_t t_fine = 0;

  float Temperature()
  {
    uint32_t temp_adc = temp_19_12() << 12 | temp_11_4() << 4 | temp_3_0();
    float var1 = ((temp_adc / 16384.0f) - (par_t1() / 1024.0f)) * par_t2();
    float var2 = (temp_adc / 131072.0f - par_t1() / 8192.0f) *
                 (temp_adc / 131072.0f - par_t1() / 8192.0f) * par_t3() * 16.0f;
    t_fine = var1 + var2;
    float calc_temp = t_fine / 5120.0f;
    return calc_temp;
  }

  float Pressure()
  {
    uint32_t press_adc = (press_19_12() << 12) | (press_11_4() << 4) | press_3_0();

    float var1 = (t_fine / 2.0f) - 64000.0f;
    float var2 = var1 * var1 * (par_p6() / 131072.0f);
    var2 = var2 + (var1 * par_p5() * 2.0f);
    var2 = (var2 / 4.0f) + (par_p4() * 65536.0f);
    var1 = (((par_p3() * var1 * var1) / 16384.0f) + (par_p2() * var1)) / 524288.0f;
    var1 = (1.0f + (var1 / 32768.0f)) * par_p1();
    float calc_pres = 1048576.0f - (float)press_adc;

    if (var1 == 0.0)
      return 0.0;

    calc_pres = (((calc_pres - (var2 / 4096.0f)) * 6250.0f) / var1);
    var1 = ((float)par_p9() * calc_pres * calc_pres) / 2147483648.0f;
    var2 = calc_pres * (par_p8() / 32768.0f);
    float var3 = (calc_pres / 256.0f) * (calc_pres / 256.0f) * (calc_pres / 256.0f) * (par_p10() / 131072.0f);
    calc_pres = (calc_pres + (var1 + var2 + var3 + (par_p7() * 128.0f)) / 16.0f);

    return calc_pres;
  }

  float Humidity()
  {
    uint16_t hum_adc = (hum_15_8() << 8) | hum_7_0();

    float temp_comp = t_fine / 5120.0f;
    float var1 = hum_adc - (par_h1() * 16.0f + (par_h3() / 2.0f * temp_comp));
    float var2 = var1 * (par_h2() / 262144.0f * (1.0f + (par_h4() / 16384.0f * temp_comp) + (par_h5() / 1048576.0f * temp_comp * temp_comp)));
    float var3 = par_h6() / 16384.0f;
    float var4 = par_h7() / 2097152.0f;
    float calc_hum = var2 + (var3 + (var4 * temp_comp)) * var2 * var2;
    if (calc_hum > 100.0f)
      return 100.0f;
    else if (calc_hum < 0.0f)
      return 0.0f;
    else
      return calc_hum;
  }

  uint8_t HeaterSetPoint(uint16_t target_temp, float ambient_temp)
  {
    if (target_temp > 400)
      target_temp = 400;

    float rng_adj = 4 / (4 + (float)res_heat_range());
    float val_adj = 1 / (1 + ((float)res_heat_val() * 0.002f));
    float var1 = (par_g1() / (16.0f)) + 49.0f;
    float var2 = ((par_g2() / 32768.0f) * 0.0005f) + 0.00235f;
    float var3 = par_g3() / 1024.0f;
    float var4 = var1 * (1.0f + (var2 * (float)target_temp));
    float var5 = var4 + (var3 * ambient_temp);
    return 3.4f * ((var5 * rng_adj * val_adj) - 25);
  }

  float HeaterSetPointTemperature(uint8_t heatr_res, float ambient_temp)
  {
    float rng_adj = 4 / (4 + (float)res_heat_range());
    float val_adj = 1 / (1 + ((float)res_heat_val() * 0.002f));
    float var5 = ((heatr_res / 3.4f) + 25) / (rng_adj * val_adj);
    float var3 = par_g3() / 1024.0f;
    float var4 = var5 - (var3 * ambient_temp);
    float var2 = ((par_g2() / 32768.0f) * 0.0005f) + 0.00235f;
    float var1 = (par_g1() / (16.0f)) + 49.0f;
    return ((var4 / var1) - 1.0f) / var2;
  }

  float GasResistance()
  {
    uint16_t gas_res_adc = (gas_r_9_2() << 2) | gas_r_1_0();
    uint8_t gas_range = gas_range_r();

    float var1 = (1340.0f + (5.0f * range_switching_error()));
    float var2 = (var1) * (1.0f + lookup_k1_range[gas_range]/100.0f);
    float var3 = 1.0f + (lookup_k2_range[gas_range]/100.0f);

    float calc_gas_res = 1.0f / (float)(var3 * (0.000000125f) * (float)(1 << gas_range) * (((((float)gas_res_adc)
      - 512.0f)/var2) + 1.0f));

    return calc_gas_res;
  }
);

#undef REG
#undef VAR

#endif // _BME680_RT_H_
