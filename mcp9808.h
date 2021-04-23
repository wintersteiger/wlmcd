// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _MCP9808_H_
#define _MCP9808_H_

#include <mutex>

#include "device.h"

class MCP9808 : public Device<uint8_t, uint16_t> {
public:
  class RegisterTable;
  RegisterTable &RT;

  MCP9808(const std::string &bus = "/dev/i2c-1", uint8_t device_address = 0x18);
  virtual ~MCP9808();

  std::string Bus() const { return bus; }
  uint8_t DeviceAddress() const { return device_address; }

  virtual const char* Name() const { return "MCP9808"; }

  virtual void Reset();

  virtual void Write(std::ostream &os);
  virtual void Read(std::istream &is);

  using Device::Read;
  using Device::Write;

  virtual uint16_t Read(const uint8_t &addr);
  uint16_t Read(const Register<uint8_t, uint16_t> &reg);
  virtual std::vector<uint16_t> Read(const uint8_t &addr, size_t length);

  virtual void Write(const uint8_t &addr, const uint16_t &value);
  virtual void Write(const Register<uint8_t, uint16_t> &reg, const uint16_t &value);
  virtual void Write(const uint8_t &addr, const std::vector<uint16_t> &values);

  void Write(const uint8_t &addr, const uint8_t &value);

  virtual void UpdateTimed();
  virtual void UpdateFrequent();
  virtual void UpdateInfrequent();

  double Temperature();

protected:
  std::mutex mtx;
  int fd;
  std::string bus;
  uint8_t device_address;
};

#endif // _MCP9808_H_
