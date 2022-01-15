// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <string.h>

#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>

#include "json.hpp"
#include "mcp9808.h"
#include "mcp9808_rt.h"

using json = nlohmann::json;

MCP9808::MCP9808(const std::string &bus, uint8_t device_address) :
  I2CDevice<uint8_t, uint16_t>(bus, device_address),
  RT(*(new RegisterTable(*this)))
{
  RT.Initialize();
  Reset();

  RT.Refresh(false);

  if (RT.ManufacturerID() != 0x0054)
    throw std::runtime_error("Unexpected MCP9808 manufacturer ID");
  if (RT.DeviceID() != 0x04)
    throw std::runtime_error("Unexpected MCP9808 device ID");
}

MCP9808::~MCP9808()
{
  delete(&RT);
}

void MCP9808::Write(const uint8_t &addr, const uint8_t &value)
{
  ((I2CDevice<uint8_t, uint8_t>*)this)->Write(addr, value);
}

void MCP9808::Write(std::ostream &os) const
{
  RT.Write(os);
}

void MCP9808::Read(std::istream &is)
{
  RT.Read(is);
}

void MCP9808::UpdateTimed()
{
  RT.Refresh(false);
}

double MCP9808::Temperature()
{
  int16_t raw = RT.Temperature() << 3;
  return raw / 128.0;
}

void MCP9808::RegisterTable::Refresh(bool frequent)
{
  for (auto reg : registers)
    buffer[reg->Address()] = device.Read(reg->Address());
}

void MCP9808::RegisterTable::Write(std::ostream &os) const
{
  json j, dev, regs;
  char tmp[17];
  dev["name"] = device.Name();
  dev["bus"] = device.bus;
  dev["address"] =  device.device_address;
  j["device"] = dev;
  for (const auto reg : registers) {
    if (reg->Writeable()) {
      if (reg->Name() == "Resolution_")
        snprintf(tmp, sizeof(tmp), "%02x", (*this)(*reg) >> 8);
      else
        snprintf(tmp, sizeof(tmp), "%04x", (*this)(*reg));
      regs[reg->Name()] = tmp;
    }
  }
  j["registers"] = regs;
  os << std::setw(2) << j << std::endl;
}

void MCP9808::RegisterTable::Read(std::istream &is)
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

    if (e.key() == "Resolution_") {
      uint8_t val;
      sscanf(sval.c_str(), "%02hhx", &val);
      device.Write(device.RT._rResolution_.Address(), val);
      found = true;
    }
    else {
      for (const auto reg : registers)
        if (reg->Name() == e.key()) {
          uint16_t val;
          sscanf(sval.c_str(), "%04hx", &val);
          device.Write(reg->Address(), val);
          found = true;
          break;
        }
    }
    if (!found)
      throw std::runtime_error(std::string("invalid register '") + e.key() + "'");
  }
}
