// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <chrono>

#include "json.hpp"
#include "bme680.h"
#include "bme680_rt.h"

using json = nlohmann::json;

BME680::BME680(const std::string &bus, uint8_t device_address) :
  I2CDevice<uint8_t, uint8_t>(bus, device_address),
  RT(new RegisterTable(*this))
{
  Reset();
  if (RT->Id() != 0x61)
    throw std::runtime_error("Invalid chip ID or unresponsive BME680");
}

BME680::BME680(const char *config_filename) :
  I2CDevice<uint8_t, uint8_t>(),
  RT(new RegisterTable(*this))
{
  std::ifstream is(config_filename);
  RT->Read(is);
  if (RT->Id() != 0x61)
    throw std::runtime_error("Invalid chip ID, unresponsive BME680, or wrong config file");
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

void BME680::Measure()
{
  Write(RT->_rCtrl_hum, 0x01);

  int8_t res_heat_0 = RT->HeaterSetPoint(300, 25);
  Write(RT->_rGas_wait_0, 0x59);
  Write(RT->_rRes_heat_0, res_heat_0);
  Write(RT->_rCtrl_gas_1, 0x10); // nb_conv + run_gas

  Write(RT->_rCtrl_meas, 0x25); // temp + press + mode
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

void BME680::RegisterTable::Write(std::ostream &os) {
  json j, dev, regs;
  char tmp[17];
  dev["name"] = device.Name();
  dev["bus"] = device.bus;
  dev["address"] = device.device_address;
  j["device"] = dev;
  for (const auto& reg : this->registers) {
    if (reg->Writeable()) {
      snprintf(tmp, sizeof(tmp), "%02x", (*this)(*reg));
      regs[reg->Name()] = tmp;
    }
  }
  j["registers"] = regs;
  os << std::setw(2) << j << std::endl;
}

void BME680::RegisterTable::Read(std::istream &is)
{
  json j = json::parse(is);
  json dev = j["device"];

  if (dev["name"] != device.Name())
    throw std::runtime_error("device mismatch");

  device.bus = dev["bus"];
  device.device_address = dev["address"];
  device.Reset();

  for (const auto &e : j["registers"].items()) {
    if (!e.value().is_string())
      throw std::runtime_error(std::string("invalid value for '" + e.key() + "'"));
    std::string sval = e.value().get<std::string>();

    bool found = false;
    for (const auto reg : registers) {
      if (reg->Name() == e.key()) {
        found = true;
        if (!reg->Writeable())
          continue;
        uint8_t val;
        sscanf(sval.c_str(), "%02hhx", &val);
        device.Write(*reg, val);
        break;
      }
    }
    if (!found)
      throw std::runtime_error(std::string("invalid register '") + e.key() + "'");
  }

  Refresh(false);
}
