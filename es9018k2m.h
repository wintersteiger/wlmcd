// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ES9018K2M_H_
#define _ES9018K2M_H_

#include "i2c_device.h"

class ES9018K2M : public I2CDevice<uint8_t, uint8_t> {
public:
  class RegisterTableSet;
  RegisterTableSet *RTS;

  ES9018K2M(const std::string &bus = "/dev/i2c-1", uint8_t device_address = 0x48);
  virtual ~ES9018K2M();

  std::string Bus() const { return bus; }
  uint8_t DeviceAddress() const { return device_address; }

  virtual const char* Name() const override { return "ES9018K2M"; }

  virtual void Reset() override;

  virtual void Write(std::ostream &os) const override;
  virtual void Read(std::istream &is) override;

  using Device::Read;
  using Device::Write;
  using I2CDevice::Read;
  using I2CDevice::Write;

  virtual void UpdateTimed() override;

  double SampleRate() const;

protected:
  static constexpr double F_MCLK = 80*1e6;
};

#endif // _ES9018K2M_H_
