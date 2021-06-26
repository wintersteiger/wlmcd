// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <iostream>
#include <iomanip>

#include "json.hpp"

#include "es9028pro.h"
#include "es9028pro_rt.h"

using json = nlohmann::json;

ES9028PRO::RegisterTableSet::RegisterTableSet(ES9028PRO &device) :
  device(device),
  Main(device),
  Consumer(device),
  Professional(device)
{
  Main.Initialize();
  Consumer.Initialize();
  Professional.Initialize();
}

ES9028PRO::RegisterTableSet::~RegisterTableSet()
{
}

void ES9028PRO::RegisterTableSet::MainRT::Refresh(bool frequent)
{
  for (auto reg : registers)
    buffer[reg->Address()] = device.I2CDevice::Read(reg->Address());
}

void ES9028PRO::RegisterTableSet::MainRT::Write(std::ostream &os)
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

void ES9028PRO::RegisterTableSet::MainRT::Read(std::istream &is)
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

void ES9028PRO::RegisterTableSet::ConsumerRT::Refresh(bool frequent)
{
  for (auto reg : registers)
    buffer[reg->Address()] = device.I2CDevice::Read(reg->Address());
}

void ES9028PRO::RegisterTableSet::ProfessionalRT::Refresh(bool frequent)
{
  for (auto reg : registers)
    buffer[reg->Address()] = device.I2CDevice::Read(reg->Address());
}

void ES9028PRO::RegisterTableSet::Refresh(bool frequent)
{
  device.RTS->Main.Refresh(frequent);
  device.RTS->Consumer.Refresh(frequent);
  device.RTS->Professional.Refresh(frequent);
}

void ES9028PRO::RegisterTableSet::Read(std::istream &is)
{
  device.RTS->Main.Read(is);
}

void ES9028PRO::RegisterTableSet::Write(std::ostream &os)
{
  device.RTS->Main.Write(os);
}