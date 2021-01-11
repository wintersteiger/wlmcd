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
#include "ui.h"

using json = nlohmann::json;

void throw_errno(const char* msg) {
  char exmsg[1024];
  snprintf(exmsg, sizeof(exmsg), "%s (%s (%d))", msg, strerror(errno), errno);
  throw std::runtime_error(exmsg);
}

INA219::INA219(double r_shunt, double max_expected_current, const std::string &bus, uint8_t device_address) :
  RT(*(new RegisterTable(*this))),
  fd(-1),
  bus(bus),
  device_address(device_address),
  r_shunt(r_shunt),
  max_expected_current(max_expected_current),
  current_lsb(0),
  power_lsb(0)
{
  Reset();

  RT.Refresh(false);
}

INA219::~INA219()
{
  if (fd >= 0)
    close(fd);
}

void INA219::Reset()
{
  {
    std::lock_guard<std::mutex> lock(mtx);

    if (fd >= 0)
      close(fd);

    if ((fd = open(bus.c_str(), O_RDWR)) < 0)
      throw_errno("failed to open the I2C bus");

    if (ioctl(fd, I2C_SLAVE, device_address) < 0)
      throw_errno("failed to acquire bus access and/or talk to slave");
  }

  Write(RT._rConfiguration, 0xFFFF);
  Write(RT._rConfiguration, 0x399F);

  if (!std::isnan(r_shunt) && !std::isnan(max_expected_current))
  {
    current_lsb = max_expected_current / 32768.0 /* 2**15 */ ;
    power_lsb = 20.0 * current_lsb;
    uint16_t cal = (uint16_t) trunc(0.04096 / (current_lsb * r_shunt));
    Write(RT._rCalibration.Address(), cal);
  }
  else
    Write(RT._rCalibration.Address(), 0);
}

uint16_t INA219::Read(const uint8_t &addr)
{
  uint8_t buf[2] = { addr };
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 1) != 1)
    throw_errno("failed to write to the I2C bus");
  if (read(fd, buf, 2) != 2)
    throw_errno("failed to read from the I2C bus");
  return buf[0] << 8 | buf[1];
}

uint16_t INA219::Read(const Register<uint8_t, uint16_t> &reg)
{
  return Read(reg.Address());
}

std::vector<uint16_t> INA219::Read(const uint8_t &addr, size_t length)
{
  std::vector<uint16_t> r(length);
  for (size_t i=0; i < length; i++)
    r[i] = Read(addr + i);
  return r;
}

void INA219::Write(const uint8_t &addr, const uint16_t &value)
{
  uint8_t buf[3];
  buf[0] = addr | 0x80;
  buf[1] = value >> 8;
  buf[2] = value & 0xFF;
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 3) != 3)
    throw_errno("failed to write to the I2C bus");
}

void INA219::Write(const Register<uint8_t, uint16_t> &reg, const uint16_t &value)
{
  Write(reg.Address(), value);
}

void INA219::Write(const uint8_t &addr, const std::vector<uint16_t> &values)
{
  for (size_t i=0; i < values.size(); i++)
    Write(addr + i, values[i]);
}

void INA219::WriteConfig(const std::string &filename)
{
  RT.WriteFile(filename);
}

void INA219::ReadConfig(const std::string &filename)
{
  RT.ReadFile(filename);
}

double INA219::BusVoltage(void) const
{
  uint16_t rv = RT._rBusVoltage(RT.Buffer());
  bool overflow = RT._vOVF(rv) != 0;
  uint16_t v_bus = RT._vBD_12_0(rv);
  double v_bus_d = (double)v_bus;
  return overflow ? nan : (v_bus_d * 0.004);
}

double INA219::ShuntVoltage(void) const
{
  int16_t v_shunt = RT._rShuntVoltage(RT.Buffer());
  uint16_t pg = RT._vPG(RT._rConfiguration(RT.Buffer()));
  double r = (v_shunt >> pg) * 0.000010;
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

void INA219::SetBusADCResolution(uint8_t value)
{
  uint16_t cfg_val = Read(RT._rConfiguration.Address()) ;
  cfg_val = (cfg_val & 0xF87F) | (value & 0x0F) << 7;
  Write(RT._rConfiguration, cfg_val);
}

void INA219::SetShuntADCResolution(uint8_t value)
{
  uint16_t cfg_val = Read(RT._rConfiguration.Address()) ;
  cfg_val = (cfg_val & 0xFF87) | (value & 0x0F) << 3;
  Write(RT._rConfiguration, cfg_val);
}

void INA219::UpdateTimed()
{
  RT.Refresh(false);
}

void INA219::UpdateFrequent()
{
}

void INA219::UpdateInfrequent()
{
}

void INA219::RegisterTable::Refresh(bool frequent)
{
  if (buffer.size() != 6)
    buffer.resize(6, 0);
  for (auto reg : registers)
    buffer[reg->Address()] = device.Read(reg->Address());
}

void INA219::RegisterTable::WriteFile(const std::string &filename)
{
  json j, dev, regs;
  char tmp[17];
  dev["Name"] = device.Name();
  dev["bus"] = device.bus;
  dev["address"] = device.device_address;
  dev["r_shunt"] = device.r_shunt;
  dev["max_expected_current"] = device.max_expected_current;
  j["Device"] = dev;
  std::vector<Register<uint8_t, uint16_t>*> regobjs = { &device.RT._rConfiguration, &device.RT._rCalibration };
  for (const auto reg : regobjs) {
    snprintf(tmp, sizeof(tmp), "%04x", (*reg)(buffer));
    regs[reg->Name()] = tmp;
  }
  j["Registers"] = regs;
  std::ofstream os(filename);
  os << std::setw(2) << j << std::endl;
}

void INA219::RegisterTable::ReadFile(const std::string &filename)
{
  json j = json::parse(std::ifstream(filename));
  json dev = j["Device"];

  if (dev["Name"] != device.Name())
    throw std::runtime_error("device mismatch");

  device.bus = dev["bus"];
  device.device_address = dev["address"];
  device.r_shunt = dev["r_shunt"];
  device.max_expected_current = dev["max_expected_current"];

  device.Reset();

  for (const auto &e : j["Registers"].items()) {
    if (!e.value().is_string())
      throw std::runtime_error(std::string("invalid value for '" + e.key() + "'"));
    std::string sval = e.value().get<std::string>();
    bool found = false;

    for (const auto reg : registers)
      if (reg->Name() == e.key()) {
        uint8_t val;
        sscanf(sval.c_str(), "%02hhx", &val);
        device.Write(reg->Address(), val);
        found = true;
        break;
      }
    if (!found)
      throw std::runtime_error(std::string("invalid register '") + e.key() + "'");
  }
}

void INA219::RegisterTable::Write(const Register<uint8_t, uint16_t> &reg, const uint16_t &value)
{
}
