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
#include "bme280.h"
#include "bme280_rt.h"

using json = nlohmann::json;

BME280::BME280(const std::string &bus, uint8_t device_address) :
  I2CDevice<uint8_t, uint8_t>(bus, device_address),
  RT(new RegisterTable(*this))
{
  Reset();
  if (RT->id() != 0x60)
    throw std::runtime_error("Invalid chip ID or unresponsive BME280");
}

BME280::BME280(const char *config_filename) :
  I2CDevice<uint8_t, uint8_t>(),
  RT(new RegisterTable(*this))
{
  std::ifstream is(config_filename);
  RT->Read(is);
  if (RT->id() != 0x60)
    throw std::runtime_error("Invalid chip ID, unresponsive BME280, or wrong config file");
  RT->Refresh(false);
}

BME280::~BME280()
{
  delete(RT);
}

void BME280::Reset()
{
  RT->Initialize();
  I2CDevice::Reset();
  Write(RT->_rreset, 0xB6);
  RT->Refresh(false);
}

void BME280::Write(std::ostream &os)
{
  RT->Write(os);
}

void BME280::Read(std::istream &is)
{
  RT->Read(is);
}

void BME280::Measure()
{
  Write(RT->_rctrl_hum, 0x01);
  Write(RT->_rctrl_meas, 0x25);
}

void BME280::UpdateTimed()
{
  RT->Refresh(false);
}

void BME280::Test(const std::vector<uint8_t>&)
{
  Measure();
  RT->Refresh(false);
}

float BME280::Temperature()
{
  return RT->Temperature();
}

float BME280::Pressure()
{
  return RT->Pressure();
}

float BME280::Humidity()
{
  return RT->Humidity();
}

void BME280::RegisterTable::Refresh(bool frequent)
{
  buffer.resize(0xFF, 0);

  // Data, Control
  auto tmp = device.Read(0xF2, 12);
  for (size_t i=0; i < 12; i++)
    buffer[0xF2+i] = tmp[i];

  // Reset, Chip ID
  for (auto i : {0xE0, 0xD0})
    buffer[i] = device.Read(i);

  if (!frequent) {
    // Calibration values
    tmp = device.Read(0x88, 25);
    for (size_t i=0; i < 25; i++)
      buffer[0x88+i] = tmp[i];
    tmp = device.Read(0xE1, 16);
    for (size_t i=0; i < 16; i++)
      buffer[0xE1+i] = tmp[i];
  }
}

void BME280::RegisterTable::Write(std::ostream &os) {
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

void BME280::RegisterTable::Read(std::istream &is)
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
