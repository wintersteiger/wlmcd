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
#include "mcp9808.h"
#include "mcp9808_rt.h"

using json = nlohmann::json;

static void throw_errno(const char* msg) {
  char exmsg[1024];
  snprintf(exmsg, sizeof(exmsg), "%s (%s (%d))", msg, strerror(errno), errno);
  throw std::runtime_error(exmsg);
}

MCP9808::MCP9808(const std::string &bus, uint8_t device_address) :
  RT(*(new RegisterTable(*this))),
  fd(-1),
  bus(bus),
  device_address(device_address)
{
  Reset();

  RT.Refresh(false);

  if (RT.ManufacturerID() != 0x0054)
    throw std::runtime_error("Unexpected MCP9808 manufacturer ID");
  if (RT.DeviceID() != 0x04)
    throw std::runtime_error("Unexpected MCP9808 device ID");
}

MCP9808::~MCP9808()
{
  if (fd >= 0)
    close(fd);
  delete(&RT);
}

void MCP9808::Reset()
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
}

uint16_t MCP9808::Read(const uint8_t &addr)
{
  uint8_t buf[2] = { addr, 0 };
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 1) != 1)
    throw_errno("failed to write to the I2C bus");
  if (read(fd, buf, 2) != 2)
    throw_errno("failed to read from the I2C bus");
  return buf[0] << 8 | buf[1];
}

uint16_t MCP9808::Read(const Register<uint8_t, uint16_t> &reg)
{
  return Read(reg.Address());
}

std::vector<uint16_t> MCP9808::Read(const uint8_t &addr, size_t length)
{
  std::vector<uint16_t> r(length);
  for (size_t i=0; i < length; i++)
    r[i] = Read(addr + i);
  return r;
}

void MCP9808::Write(const uint8_t &addr, const uint16_t &value)
{
  uint8_t buf[3];
  buf[0] = addr;
  buf[1] = value >> 8;
  buf[2] = value & 0xFF;
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 3) != 3)
    throw_errno("failed to write to the I2C bus");
}

void MCP9808::Write(const uint8_t &addr, const uint8_t &value)
{
  uint8_t buf[2];
  buf[0] = addr;
  buf[1] = value;
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 2) != 2)
    throw_errno("failed to write to the I2C bus");
}

void MCP9808::Write(const Register<uint8_t, uint16_t> &reg, const uint16_t &value)
{
  Write(reg.Address(), value);
}

void MCP9808::Write(const uint8_t &addr, const std::vector<uint16_t> &values)
{
  for (size_t i=0; i < values.size(); i++)
    Write(addr + i, values[i]);
}

void MCP9808::Write(std::ostream &os)
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

void MCP9808::UpdateFrequent()
{
}

void MCP9808::UpdateInfrequent()
{
}

double MCP9808::Temperature()
{
  int16_t raw = RT.Temperature() << 3;
  return raw / 128.0;
}

void MCP9808::RegisterTable::Refresh(bool frequent)
{
  if (buffer.size() != registers.size())
    buffer.resize(registers.size(), 0);
  for (auto reg : registers)
    buffer[reg->Address()] = device.Read(reg->Address());
}

void MCP9808::RegisterTable::Write(std::ostream &os)
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
        snprintf(tmp, sizeof(tmp), "%02x", (*reg)(buffer) >> 8);
      else
        snprintf(tmp, sizeof(tmp), "%04x", (*reg)(buffer));
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
      device.Write(device.RT._rResolution_, val);
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

void MCP9808::RegisterTable::Write(const Register<uint8_t, uint16_t> &reg, const uint16_t &value)
{
  device.Write(reg, value);
}
