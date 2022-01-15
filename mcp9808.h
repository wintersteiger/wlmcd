// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _MCP9808_H_
#define _MCP9808_H_

#include <mutex>

#include "i2c_device.h"

class MCP9808 : public I2CDevice<uint8_t, uint16_t> {
public:
  class RegisterTable;
  RegisterTable &RT;

  MCP9808(const std::string &bus = "/dev/i2c-1", uint8_t device_address = 0x18);
  virtual ~MCP9808();

  std::string Bus() const { return bus; }
  uint8_t DeviceAddress() const { return device_address; }

  virtual const char* Name() const override { return "MCP9808"; }

  virtual void Write(std::ostream &os) const override;
  virtual void Read(std::istream &is) override;

  using I2CDevice::Reset;
  using I2CDevice::Read;
  using I2CDevice::Write;

  void Write(const uint8_t &addr, const uint8_t &value);

  virtual void UpdateTimed() override;

  double Temperature();
};

#endif // _MCP9808_H_
