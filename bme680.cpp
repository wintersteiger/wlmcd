// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>

#include "bme680.h"
#include "bme680_rt.h"

BME680::BME680(const std::string &bus, uint8_t device_address) :
  I2CDevice<uint8_t, uint8_t>(bus, device_address),
  RT(new RegisterTable(*this))
{
  Reset();
  if (RT->Id() != 0x61)
    throw std::runtime_error("Invalid chip ID or unresponsive BME680");
}

BME680::~BME680()
{
  delete(RT);
}

void BME680::Reset()
{
  RT->Initialize();
  I2CDevice::Reset();
  Write(RT->_rReset, 0xB6);
  RT->Refresh(false);
}

void BME680::Write(std::ostream &os)
{
  RT->Write(os);
}

void BME680::Read(std::istream &is)
{
  RT->Read(is);
}

std::vector<uint8_t> BME680::GetCalibData()
{
  size_t BME68X_LEN_COEFF_ALL = 42;

  uint8_t BME68X_REG_COEFF1 = 0x8A;
  size_t BME68X_LEN_COEFF1 = 23;

  uint8_t BME68X_REG_COEFF2 = 0xE1;
  size_t BME68X_LEN_COEFF2 = 14;

  uint8_t BME68X_REG_COEFF3 = 0;
  size_t BME68X_LEN_COEFF3 = 5;

  uint8_t coeff_array[BME68X_LEN_COEFF_ALL];

  auto coeffs1 = I2CDevice::Read(BME68X_REG_COEFF1, BME68X_LEN_COEFF1);
  auto coeffs2 = I2CDevice::Read(BME68X_REG_COEFF2, BME68X_LEN_COEFF2);
  auto coeffs3 = I2CDevice::Read(BME68X_REG_COEFF3, BME68X_LEN_COEFF3);

  coeffs1.insert(coeffs1.begin(), coeffs2.begin(), coeffs2.end());
  coeffs1.insert(coeffs1.begin(), coeffs3.begin(), coeffs3.end());
  return coeffs1;
}

int8_t BME680::ComputeHeaterTemp(uint16_t target_temp, int32_t ambient_temp)
{
  if (target_temp > 400)
    target_temp = 400;

  auto cd = GetCalibData();
  int8_t par_gh1 = cd[35];
  int16_t par_gh2 = cd[34] << 8 | cd[33];
  int8_t par_gh3 = cd[36];
  uint8_t res_heat_range = (cd[39] & 0x30) / 16;
  int8_t res_heat_val = cd[37];

  int32_t var1 = ((ambient_temp * par_gh3) / 1000) * 256;
  int32_t var2 = (par_gh1 + 784) * (((((par_gh2 + 154009) * target_temp * 5) / 100) + 3276800) / 10);
  int32_t var3 = var1 + (var2 / 2);
  int32_t var4 = (var3 / (res_heat_range + 4));
  int32_t var5 = (131 * res_heat_val) + 65536;
  int32_t heatr_res_x100 = (int32_t)(((var4 / var5) - 250) * 34);
  int8_t heatr_res = (int8_t)((heatr_res_x100 + 50) / 100);

  return heatr_res;
}

int16_t BME680::ComputeTemperature(uint32_t t)
{
  int64_t var1 = ((int32_t)t >> 3) - ((int32_t)RT->par_t1() << 1);
  int64_t var2 = (var1 * (int32_t)RT->par_t2()) >> 11;
  int64_t var3 = ((var1 >> 1) * (var1 >> 1)) >> 12;
  var3 = ((var3) * ((int32_t)RT->par_t3() << 4)) >> 14;
  t_fine = (int32_t)(var2 + var3); // saved!
  return ((t_fine * 5) + 128) >> 8;
}

uint32_t BME680::ComputePressure(uint32_t p)
{
  /* This value is used to check precedence to multiplication or division
    * in the pressure compensation equation to achieve least loss of precision and
    * avoiding overflows.
    * i.e Comparing value, pres_ovf_check = (1 << 31) >> 1
    */
  const int32_t pres_ovf_check = INT32_C(0x40000000);

  int32_t var1 = (((int32_t)t_fine) >> 1) - 64000;
  int32_t var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)RT->par_p6()) >> 2;
  var2 = var2 + ((var1 * (int32_t)RT->par_p5()) << 1);
  var2 = (var2 >> 2) + ((int32_t)RT->par_p4() << 16);
  var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)RT->par_p3() << 5)) >> 3) + (((int32_t)RT->par_p2() * var1) >> 1);
  var1 = var1 >> 18;
  var1 = ((32768 + var1) * (int32_t)RT->par_p1()) >> 15;

  int32_t pressure_comp = 1048576 - p;
  pressure_comp = (int32_t)((pressure_comp - (var2 >> 12)) * ((uint32_t)3125));
  if (pressure_comp >= pres_ovf_check)
      pressure_comp = ((pressure_comp / var1) << 1);
  else
      pressure_comp = ((pressure_comp << 1) / var1);

  var1 = ((int32_t)RT->par_p9() * (int32_t)(((pressure_comp >> 3) * (pressure_comp >> 3)) >> 13)) >> 12;
  var2 = ((int32_t)(pressure_comp >> 2) * (int32_t)RT->par_p8()) >> 13;
  int32_t var3 = ((int32_t)(pressure_comp >> 8) * (int32_t)(pressure_comp >> 8) * (int32_t)(pressure_comp >> 8) * (int32_t)RT->par_p10()) >> 17;
  pressure_comp = (int32_t)(pressure_comp) + ((var1 + var2 + var3 + ((int32_t)RT->par_p7() << 7)) >> 4);

  return pressure_comp;
}

void BME680::Measure()
{
  int8_t res_heat_0 = ComputeHeaterTemp(300, 25);
  Write(RT->_rCtrl_hum, RT->_vosrs_h_2_0.Set(RT->Ctrl_hum(), 0x01));
  Write(RT->_rCtrl_meas, RT->_vosrs_t_2_0.Set(RT->Ctrl_meas(), 0x02));
  Write(RT->_rCtrl_meas, RT->_vosrs_p_2_0.Set(RT->Ctrl_meas(), 0x05));
  Write(RT->_rGas_wait_0, 0x59);
  Write(RT->_rRes_heat_0, res_heat_0);
  Write(RT->_rCtrl_gas_1, RT->_vnb_conv_3_0.Set(RT->Ctrl_gas_1(), 0x05));
  Write(RT->_rCtrl_gas_1, RT->_vrun_gas.Set(RT->Ctrl_gas_1(), 0x01));
  Write(RT->_rCtrl_meas, RT->_vmode_1_0.Set(RT->Ctrl_meas(), 0x01));
}

void BME680::UpdateTimed()
{
  RT->Refresh(false);
}

void BME680::Test(const std::vector<uint8_t>&)
{
  Measure();
  RT->Refresh(false);
}

void BME680::RegisterTable::Refresh(bool frequent)
{
  for (auto reg : registers)
    buffer[reg->Address()] = device.Read(*reg);
}

void BME680::RegisterTable::Write(std::ostream &os) {}

void BME680::RegisterTable::Read(std::istream &is) {}
