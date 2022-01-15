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

#include "errors.h"
#include "ccs811.h"
#include "ccs811_rt.h"

using json = nlohmann::json;

CCS811::CCS811(const std::string &bus, uint8_t device_address) :
  I2CDevice<uint8_t, std::vector<uint8_t>>(bus, device_address),
  RT(new RegisterTable(*this))
{
  Reset();
  // if (RT->id() != 0x60)
  //   throw std::runtime_error("Invalid chip ID or unresponsive CCS811");
}

CCS811::CCS811(const char *config_filename) :
  I2CDevice<uint8_t, std::vector<uint8_t>>(),
  RT(new RegisterTable(*this))
{
  std::ifstream is(config_filename);
  RT->Read(is);
  // if (RT->id() != 0x60)
    // throw std::runtime_error("Invalid chip ID, unresponsive CCS811, or wrong config file");
  RT->Refresh(false);
}

CCS811::~CCS811()
{
  delete(RT);
}

void CCS811::Reset()
{
  RT->Initialize();
  I2CDevice::Reset();
  // Write(RT->_rreset, 0xB6);
  RT->Refresh(false);
}

void CCS811::Write(std::ostream &os) const
{
  RT->Write(os);
}

void CCS811::Read(std::istream &is)
{
  RT->Read(is);
}

void CCS811::UpdateTimed()
{
  RT->Refresh(false);
}

void CCS811::RegisterTable::Refresh(bool frequent)
{
  for (auto& r : registers) {
    if (r->Readable())
      buffer[r->Address()] = device.Read(r->Address());
  }
}

void CCS811::RegisterTable::Write(std::ostream &os) const
{
  json j, dev, regs;
  char tmp[17];
  dev["name"] = device.Name();
  dev["bus"] = device.bus;
  dev["address"] = device.device_address;
  j["device"] = dev;
  // for (const auto& reg : this->registers) {
  //   if (reg->Writeable()) {
  //     snprintf(tmp, sizeof(tmp), "%02x", (*this)(*reg));
  //     regs[reg->Name()] = tmp;
  //   }
  // }
  // j["registers"] = regs;
  os << std::setw(2) << j << std::endl;
}

void CCS811::RegisterTable::Read(std::istream &is)
{
  json j = json::parse(is);
  json dev = j["device"];

  if (dev["name"] != device.Name())
    throw std::runtime_error("device mismatch");

  device.bus = dev["bus"];
  device.device_address = dev["address"];
  device.Reset();

  // for (const auto &e : j["registers"].items()) {
  //   if (!e.value().is_string())
  //     throw std::runtime_error(std::string("invalid value for '" + e.key() + "'"));
  //   std::string sval = e.value().get<std::string>();

  //   bool found = false;
  //   for (const auto reg : registers) {
  //     if (reg->Name() == e.key()) {
  //       found = true;
  //       if (!reg->Writeable())
  //         continue;
  //       uint8_t val;
  //       sscanf(sval.c_str(), "%02hhx", &val);
  //       device.Write(*reg, val);
  //       break;
  //     }
  //   }
  //   if (!found)
  //     throw std::runtime_error(std::string("invalid register '") + e.key() + "'");
  // }

  Refresh(false);
}

std::vector<uint8_t> CCS811::Read(const uint8_t &addr)
{
  size_t value_size = RT->Find(addr)->value_size();
  uint8_t buf[1] = {addr};
  const std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 1) != 1)
    throw_errno("failed to write to the I2C bus");
  std::vector<uint8_t> r(value_size);
  if ((size_t)read(fd, r.data(), value_size) != value_size)
    throw_errno("failed to read from the I2C bus");
  return r;
}

void CCS811::Write(const uint8_t &addr, const std::vector<uint8_t> &value)
{
  std::vector<uint8_t> buf = value;
  buf.insert(buf.begin(), addr);
  std::lock_guard<std::mutex> lock(mtx);
  if ((size_t)write(fd, buf.data(), buf.size()) != buf.size())
    throw_errno("failed to write to the I2C bus");
}