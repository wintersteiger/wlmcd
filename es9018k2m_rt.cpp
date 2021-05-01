// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <iostream>
#include <iomanip>

#include "json.hpp"

#include "es9018k2m.h"
#include "es9018k2m_rt.h"

using json = nlohmann::json;

ES9018K2M::RegisterTableSet::RegisterTableSet(ES9018K2M &device) :
  device(device),
  Main(device),
  Consumer(device),
  Professional(device)
{
}

ES9018K2M::RegisterTableSet::~RegisterTableSet()
{
}

void ES9018K2M::RegisterTableSet::MainRT::Write(const Register<uint8_t, uint8_t> &reg, const uint8_t &value)
{
  device.I2CDevice::Write(reg.Address(), value);
}

void ES9018K2M::RegisterTableSet::MainRT::Refresh(bool frequent)
{
  buffer.resize(registers.size(), 0);
  for (auto reg : registers)
    buffer[reg->Address()] = device.I2CDevice::Read(reg->Address());
}

void ES9018K2M::RegisterTableSet::MainRT::Write(std::ostream &os)
{
  json j, dev, regs;
  char tmp[17];
  dev["name"] = device.Name();
  dev["bus"] = device.bus;
  dev["address"] =  device.device_address;
  j["device"] = dev;
  for (const auto reg : registers) {
    if (reg->Writeable()) {
      snprintf(tmp, sizeof(tmp), "%02x", (*reg)(buffer));
      regs[reg->Name()] = tmp;
    }
  }
  j["registers"] = regs;
  os << std::setw(2) << j << std::endl;
}

void ES9018K2M::RegisterTableSet::MainRT::Read(std::istream &is)
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
        device.I2CDevice::Write(reg->Address(), val);
        found = true;
        break;
      }

    if (!found)
      throw std::runtime_error(std::string("invalid register '") + e.key() + "'");
  }
}

void ES9018K2M::RegisterTableSet::ConsumerRT::Refresh(bool frequent)
{
  buffer.resize(0x5D+1, 0);
  for (auto reg : registers)
    buffer[reg->Address()] = device.I2CDevice::Read(reg->Address());
}

void ES9018K2M::RegisterTableSet::ProfessionalRT::Refresh(bool frequent)
{
  buffer.resize(0x5D+1, 0);
  for (auto reg : registers)
    buffer[reg->Address()] = device.I2CDevice::Read(reg->Address());
}

void ES9018K2M::RegisterTableSet::Refresh(bool frequent)
{
  device.RTS.Main.Refresh(frequent);
  device.RTS.Professional.Refresh(frequent); // Consumer RT overlaps completely.
}

void ES9018K2M::RegisterTableSet::Read(std::istream &is)
{
  device.RTS.Main.Read(is);
}

void ES9018K2M::RegisterTableSet::Write(std::ostream &os)
{
  device.RTS.Main.Write(os);
}