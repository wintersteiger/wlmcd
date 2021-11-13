// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <string.h>

#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <stdexcept>

#include "json.hpp"
#include "errors.h"
#include "mcp3423.h"
#include "mcp3423_rt.h"

using json = nlohmann::json;

MCP3423::MCP3423(const std::string &bus, uint8_t device_address) :
  I2CDevice<uint8_t, uint8_t>(bus, device_address),
  RT(*(new RegisterTable(*this)))
{
  last_readings.resize(2, 0.0);
  last_raw_readings.resize(2, 0);
  RT.Initialize();
  Reset();

  RT.Refresh(false);
}

MCP3423::~MCP3423()
{
  delete(&RT);
}

void MCP3423::Reset()
{
  I2CDevice::Reset();
}

const std::vector<double>& MCP3423::Readings()
{
  return last_readings;
}

void MCP3423::Write(const uint8_t &config)
{
  const std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, &config, 1) != 1)
    throw_errno("failed to write to the I2C bus");
}

static double lsb_voltage(uint8_t config)
{
  uint8_t pga = config & 0x03;
  uint8_t resolution = (config >> 2) & 0x03;
  double d = 1 << pga;
  switch (resolution) {
    case 0: return (1e-3)/d;
    case 1: return (250e-6)/d;
    case 2: return (62.5e-6)/d;
    case 3: return (15.625e-6)/d;
    default: throw std::logic_error("invalid resolution setting");
  }
}

uint8_t MCP3423::Read(const uint8_t &addr)
{
  if (addr != 0)
    throw std::logic_error("device registers not addressable");
  uint8_t buf[4];
  const std::lock_guard<std::mutex> lock(mtx);
  if (read(fd, &buf[0], 4) != 4)
    throw_errno("failed to read from the I2C bus");
  int32_t val;
  uint8_t config = buf[3];
  uint8_t num_data_bytes = ((config >> 2) & 0x03) == 0x03 ? 3 : 2;
  uint8_t channel = (config & 0x20) == 0 ? 0 : 1;
  if (num_data_bytes == 2) {
    val = buf[0] << 8 | buf[1];
    if (val & 0x00008000)
      val |= 0xFFFF0000;
    if ((buf[2] & 0x7F) != (buf[3] & 0x7F))
      throw std::runtime_error("inconsistent configurations reported");
  }
  else if (num_data_bytes == 3) {
    val = buf[0] << 16 | buf[1] << 8 | buf[2];
    if (val & 0x00800000)
      val |= 0xFF000000;
  }
  last_raw_reading = val;
  last_raw_readings[channel] = val;
  last_reading = lsb_voltage(config) * val;
  last_readings[channel] = last_reading;
  return config;
}

std::vector<uint8_t> MCP3423::Read(const uint8_t &addr, size_t length)
{
  throw std::logic_error("device registers not addressable");
}

void MCP3423::Write(const uint8_t &addr, const uint8_t &value)
{
  if (addr != 0)
    throw std::logic_error("device registers not addressable");
  else
    Write(value);
}

void MCP3423::Write(const uint8_t &addr, const std::vector<uint8_t> &values)
{
  throw std::logic_error("device registers not addressable");
}

void MCP3423::Write(std::ostream &os)
{
  RT.Write(os);
}

void MCP3423::Read(std::istream &is)
{
  RT.Read(is);
}

void MCP3423::UpdateInfrequent()
{
  RT.Refresh(false);
}

void MCP3423::UpdateTimed()
{
  RT.Refresh(false);
}

void MCP3423::RegisterTable::Refresh(bool frequent)
{
  for (auto reg : registers)
    buffer[reg->Address()] = device.Read(reg->Address());
}

void MCP3423::RegisterTable::Write(std::ostream &os)
{
  json j, dev, regs;
  char tmp[17];
  dev["name"] = device.Name();
  dev["bus"] = device.bus;
  dev["address"] =  device.device_address;
  j["device"] = dev;
  for (const auto reg : registers) {
    if (reg->Writeable()) {
      snprintf(tmp, sizeof(tmp), "%02x", (*this)(*reg));
      regs[reg->Name()] = tmp;
    }
  }
  j["registers"] = regs;
  os << std::setw(2) << j << std::endl;
}

void MCP3423::RegisterTable::Read(std::istream &is)
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
