// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _BME280_RT_H_
#define _BME280_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE_W(BME280, RegisterTable, uint8_t, uint8_t,
  REG(hum_lsb, "hum_lsb", 0xFE, RO,                               "hum_lsb",
    VAR(hum_lsb, hum_7_0, "hum[7:0]", 0xFF, RO,                   "");
  );
  REG(hum_msb, "hum_msb", 0xFD, RO,                               "hum_msb",
    VAR(hum_msb, hum_15_8, "hum[15:8]", 0xFF, RO,                 "");
  );
  REG(temp_xlsb, "temp_xlsb", 0xFC, RO,                           "temp_xlsb",
    VAR(temp_xlsb, temp_3_0, "temp[3:0]", 0xF0, RO,               "");
  );
  REG(temp_lsb, "temp_lsb", 0xFB, RO,                             "temp_lsb",
    VAR(temp_lsb, temp_11_4, "temp[11:4]", 0xFF, RO,              "");
  );
  REG(temp_msb, "temp_msb", 0xFA, RO,                             "temp_msb",
    VAR(temp_msb, temp_19_12, "temp[19:12]", 0xFF, RO,            "");
  );
  REG(press_xlsb, "press_xlsb", 0xF9, RO,                         "press_xlsb",
    VAR(press_xlsb, press_3_0, "press[3:0]", 0xF0, RO,  "");
  );
  REG(press_lsb, "press_lsb", 0xF8, RO,                           "press_lsb",
    VAR(press_lsb, press_11_4, "press[11:4]", 0xFF, RO,     "");
  );
  REG(press_msb, "press_msb", 0xF7, RO,                           "press_msb",
    VAR(press_msb, press_19_12, "press[19:12]", 0xFF, RO,     "");
  );
  // 0xF6?
  REG(config, "config", 0xF5, RW,                                 "config",
    VAR(config, t_sb_2_0, "t_sb[2:0]", 0xE0, RW,                  "");
    VAR(config, filter_2_0, "filter[2:0]", 0x1C, RW,              "");
    VAR(config, spi_3w_en, "spi_3w_en", 0x01, RW,                 "");
  );
  REG(ctrl_meas, "ctrl_meas", 0xF4, RW,                           "ctrl_meas",
    VAR(ctrl_meas, osrs_t_2_0, "osrs_t[2:0]", 0xE0, RW,           "");
    VAR(ctrl_meas, osrs_p_2_0, "osrs_p[2:0]", 0x1C, RW,           "");
    VAR(ctrl_meas, mode_1_0, "mode[1:0]", 0x03, RW,               "");
  );
  REG(status, "status", 0xF3, RO,                                 "status",
    VAR(status, measuring, "measuring", 0x08, RO,                 "");
    VAR(status, im_update, "im_update", 0x01, RO,                 "");
  );
  REG(ctrl_hum, "ctrl_hum", 0xF2, RW,                             "ctrl_hum",
    VAR(ctrl_hum, osrs_h_2_0, "osrs_h[2:0]", 0x07, RW,            "");
  );

  REG(reset, "reset", 0xE0, RW,                                   "reset",
    VAR(reset, reset_7_0, "reset[7:0]", 0xFF, RW,                 "");
  );
  REG(id, "id", 0xD0, RO,                                         "id",
    VAR(id, chip_id_7_0, "chip_id[7:0]", 0xFF, RO,                "");
  );

  #define CALIB_00_25(X) \
    REG(calib##X, "calib"#X, 0x88 + (X), RW, "calib"#X, \
      VAR(calib##X, calib##X##_7_0, "calib" #X "[7:0]", 0xFF, RW, ""));
  #define CALIB_26_41(X) \
    REG(calib##X, "calib"#X, 0xE1 + (X) - 26, RW, "calib"#X, \
      VAR(calib##X, calib##X##_7_0, "calib" #X "[7:0]", 0xFF, RW, ""));

  #define TWENTYFIVE(X) X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9) X(10) X(11) X(12) X(13) X(14) X(15) X(16) X(17) X(18) X(19) X(20) X(21) X(22) X(23) X(24) X(25)
  #define SIXTEEN(X) X(26) X(27) X(28) X(29) X(30) X(31) X(32) X(33) X(34) X(35) X(36) X(37) X(38) X(39) X(40) X(41)

  TWENTYFIVE(CALIB_00_25)
  SIXTEEN(CALIB_26_41)

  uint16_t dig_T1() const { return (calib1() << 8) | calib0(); }
  int16_t dig_T2() const { return (calib3() << 8) | calib2(); }
  int16_t dig_T3() const { return (calib5() << 8) | calib4(); }

  uint16_t dig_P1() const { return (calib7() << 8) | calib6(); }
  int16_t dig_P2() const { return (calib9() << 8) | calib8(); }
  int16_t dig_P3() const { return (calib11() << 8) | calib10(); }
  int16_t dig_P4() const { return (calib13() << 8) | calib12(); }
  int16_t dig_P5() const { return (calib15() << 8) | calib14(); }
  int16_t dig_P6() const { return (calib17() << 8) | calib16(); }
  int16_t dig_P7() const { return (calib19() << 8) | calib18(); }
  int16_t dig_P8() const { return (calib21() << 8) | calib20(); }
  int16_t dig_P9() const { return (calib23() << 8) | calib22(); }

  uint8_t dig_H1() const { return calib25(); }
  int16_t dig_H2() const { return (calib27() << 8) | calib26(); }
  uint8_t dig_H3() const { return calib28(); }
  int16_t dig_H4() const { return (calib29() << 4) | (calib30() & 0x0F); }
  int16_t dig_H5() const { return (calib31() << 4) | (calib30() >> 4); }
  int8_t dig_H6() const { return calib32(); }

  int32_t t_fine = 0;

  float Temperature()
  {
    static constexpr float t_min = -40.0f;
    static constexpr float t_max = 85.0f;

    int32_t dac_t = temp_19_12() << 12 | temp_11_4() << 4 | temp_3_0();

    float var1 = (((float)dac_t) / 16384.0 - ((float)dig_T1()) / 1024.0) * (float)dig_T2();
    float var2 = ((float)dac_t) / 131072.0 - ((float)dig_T1()) / 8192.0;
    var2 = var2 * var2 * ((float)dig_T3());
    t_fine = var1 + var2;
    float temperature = (var1 + var2) / 5120.0;

    if (temperature < t_min)
      return t_min;
    else if (temperature > t_max)
      return t_max;
    else
      return temperature;
  }

  float Pressure()
  {
    static constexpr float p_min = 30000.0f;
    static constexpr float p_max = 110000.0f;

    float var1 = ((float)t_fine / 2.0f) - 64000.0f;
    float var2 = var1 * var1 * ((float)dig_P6()) / 32768.0f;
    var2 = var2 + var1 * ((float)dig_P5()) * 2.0f;
    var2 = (var2 / 4.0f) + (((float)dig_P4()) * 65536.0f);
    float var3 = ((float)dig_P3()) * var1 * var1 / 524288.0f;
    var1 = (var3 + ((float)dig_P2()) * var1) / 524288.0f;
    var1 = (1.0f + var1 / 32768.0f) * ((float)dig_P1());

    if (var1 <= 0.0f)
      return p_min;
    else {
        uint32_t press_adc = (press_19_12() << 12) | (press_11_4() << 4) | press_3_0();
        float pressure = 1048576.0f - (float)press_adc;
        pressure = (pressure - (var2 / 4096.0f)) * 6250.0f / var1;
        var1 = ((float)dig_P9() * pressure * pressure) / 2147483648.0f;
        var2 = (pressure * (float)dig_P8()) / 32768.0f;
        pressure += (var1 + var2 + ((float)dig_P7())) / 16.0f;

        if (pressure < p_min)
          return p_min;
        else if (pressure > p_max)
          return p_max;
        else
          return pressure;
    }
  }

  float Humidity()
  {
    static constexpr float h_min = 0.0f;
    static constexpr float h_max = 100.0f;

    uint16_t hum_adc = (hum_15_8() << 8) | hum_7_0();

    float var1 = ((float)t_fine) - 76800.0;
    float var2 = ((float)dig_H4()) * 64.0 + (((float)dig_H5()) / 16384.0) * var1;
    float var3 = hum_adc - var2;
    float var4 = ((float)dig_H2()) / 65536.0;
    float var5 = 1.0 + (((float)dig_H3()) / 67108864.0) * var1;
    float var6 = 1.0 + (((float)dig_H6()) / 67108864.0) * var1 * var5;
    var6 = var3 * var4 * (var5 * var6);
    float humidity = var6 * (1.0 - ((float)dig_H1()) * var6 / 524288.0);

    if (humidity > h_max)
      return h_max;
    else if (humidity < h_min)
      return h_min;
    else
      return humidity;
  }
);

#undef REG
#undef VAR

#endif // _BME280_RT_H_
