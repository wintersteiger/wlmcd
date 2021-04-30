// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ES9018K2M_H_
#define _ES9018K2M_H_

#include <mutex>

#include "device.h"
#include "i2c_device.h"

class ES9018K2M : public I2CDevice<uint8_t, uint8_t> {
public:
  class RegisterTableSet;
  RegisterTableSet &RTS;

  ES9018K2M(const std::string &bus = "/dev/i2c-1", uint8_t device_address = 0x48);
  virtual ~ES9018K2M();

  std::string Bus() const { return bus; }
  uint8_t DeviceAddress() const { return device_address; }

  virtual const char* Name() const { return "ES9018K2M"; }

  virtual void Reset();

  virtual void Write(std::ostream &os);
  virtual void Read(std::istream &is);

  using Device::Read;
  using Device::Write;
  using I2CDevice::Read;
  using I2CDevice::Write;

  uint8_t Read(const Register<uint8_t, uint8_t> &reg);
  virtual void Write(const Register<uint8_t, uint8_t> &reg, const uint8_t &value);

  virtual void UpdateTimed();
  virtual void UpdateFrequent();
  virtual void UpdateInfrequent();
};

#endif // _ES9018K2M_H_
