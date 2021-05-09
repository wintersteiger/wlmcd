// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>

#include "json.hpp"
#include "ina219.h"
#include "ina219_rt.h"

using json = nlohmann::json;

INA219::INA219(double r_shunt, double max_expected_current, const std::string &bus, uint8_t device_address) :
  I2CDevice<uint8_t, uint16_t>(bus, device_address),
  RT(*(new RegisterTable(*this))),
  r_shunt(r_shunt),
  max_expected_current(max_expected_current),
  current_lsb(0),
  power_lsb(0)
{
  RT.Initialize();
  Reset();
  RT.Refresh(false);
}

INA219::~INA219()
{
  delete(&RT);
}

void INA219::Reset()
{
  I2CDevice<uint8_t, uint16_t>::Reset();

  RT.Write(RT._rConfiguration, 0xFFFF);
  RT.Write(RT._rConfiguration, 0x399F);

  if (!std::isnan(r_shunt) && !std::isnan(max_expected_current))
  {
    current_lsb = max_expected_current / 32768.0 /* 2**15 */ ;
    power_lsb = 20.0 * current_lsb;
    uint16_t cal = (uint16_t) trunc(0.04096 / (current_lsb * r_shunt));
    Write(RT._rCalibration, cal);
  }
  else
    Write(RT._rCalibration, 0);
}

void INA219::Write(std::ostream &os)
{
  RT.Write(os);
}

void INA219::Read(std::istream &is)
{
  RT.Read(is);
}

double INA219::BusVoltage(void) const
{
  bool overflow = RT.OVF() != 0;
  uint16_t v_bus = RT.BD_12_0();
  double v_bus_d = (double)v_bus;
  return overflow ? nan : (v_bus_d * 0.004);
}

double INA219::ShuntVoltage(void) const
{
  int16_t v_shunt = RT.ShuntVoltage();
  uint16_t pg = RT.PG();
  double r = v_shunt * 0.000010;
  return r;
}

// Current and Power remain at 0.0 until the calibration register is programmed.
double INA219::Current(void) const
{
  // EXPECTED Current = (Shunt Voltage Register * Calibration Register) / 4096
  int16_t current_reg = RT._rCurrent(RT.Buffer());
  return current_reg * current_lsb;
}

double INA219::Power(void) const
{
  // EXPECTED Power = (Current Register * Bus Voltage Register) / 5000
  int16_t power_reg = RT._rPower(RT.Buffer());
  return power_reg * power_lsb;
}

void INA219::SetBusVoltageRange(BusVoltageRange range)
{
  uint16_t cfg_val = Read(RT._rConfiguration) & 0x1FFF;
  if (range == _32V)
    cfg_val |= 0x2000;
  RT.Write(RT._rConfiguration, cfg_val);
}

void INA219::SetPGARange(PGARange range)
{
  uint16_t cfg_val = Read(RT._rConfiguration) & 0x07FF;
  switch (range) {
    case _40mV: break;
    case _80mV: cfg_val |= 0x0800; break;
    case _160mV: cfg_val |= 0x1000; break;
    case _320mV: cfg_val |= 0x1800; break;
    default: throw std::runtime_error("invalid PGA range");
  }
  RT.Write(RT._rConfiguration, cfg_val);
}

void INA219::SetBusADCResolution(uint8_t value)
{
  uint16_t cfg_val = Read(RT._rConfiguration);
  cfg_val = (cfg_val & 0x387F) | (value & 0x000F) << 7;
  RT.Write(RT._rConfiguration, cfg_val);
}

void INA219::SetShuntADCResolution(uint8_t value)
{
  uint16_t cfg_val = Read(RT._rConfiguration);
  cfg_val = (cfg_val & 0x3F87) | (value & 0x000F) << 3;
  RT.Write(RT._rConfiguration, cfg_val);
}

void INA219::UpdateTimed()
{
  RT.Refresh(false);
}

void INA219::RegisterTable::Refresh(bool frequent)
{
  if (buffer.size() != registers.size())
    buffer.resize(registers.size(), 0);
  for (auto reg : registers)
    buffer[reg->Address()] = device.Read(*reg);
}

void INA219::RegisterTable::Write(std::ostream &os)
{
  json j, dev, regs;
  char tmp[17];
  dev["name"] = device.Name();
  dev["bus"] = device.bus;
  dev["address"] = device.device_address;
  dev["r_shunt"] = device.r_shunt;
  dev["max_expected_current"] = device.max_expected_current;
  j["device"] = dev;
  std::vector<Register<uint8_t, uint16_t>*> regobjs = { &device.RT._rConfiguration, &device.RT._rCalibration };
  for (const auto reg : regobjs) {
    snprintf(tmp, sizeof(tmp), "%04x", (*reg)(buffer));
    regs[reg->Name()] = tmp;
  }
  j["registers"] = regs;
  os << std::setw(2) << j << std::endl;
}

void INA219::RegisterTable::Read(std::istream &is)
{
  json j = json::parse(is);
  json dev = j["device"];

  if (dev["name"] != device.Name())
    throw std::runtime_error("device mismatch");

  device.bus = dev["bus"];
  device.device_address = dev["address"];
  device.r_shunt = dev["r_shunt"];
  device.max_expected_current = dev["max_expected_current"];

  device.Reset();

  for (const auto &e : j["registers"].items()) {
    if (!e.value().is_string())
      throw std::runtime_error(std::string("invalid value for '" + e.key() + "'"));
    std::string sval = e.value().get<std::string>();
    bool found = false;

    for (const auto reg : registers)
      if (reg->Name() == e.key()) {
        uint8_t val;
        sscanf(sval.c_str(), "%02hhx", &val);
        device.Write(*reg, val);
        found = true;
        break;
      }
    if (!found)
      throw std::runtime_error(std::string("invalid register '") + e.key() + "'");
  }
}
