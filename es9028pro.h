// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ES9028PRO_H_
#define _ES9028PRO_H_

#include "i2c_device.h"

class ES9028PRO : public I2CDevice<uint8_t, uint8_t> {
public:
  class RegisterTableSet;
  RegisterTableSet *RTS;

  ES9028PRO(const std::string &bus = "/dev/i2c-1", uint8_t device_address = 0x48, double F_MCLK = 80*1e6);
  virtual ~ES9028PRO();

  std::string Bus() const { return bus; }
  uint8_t DeviceAddress() const { return device_address; }

  virtual const char* Name() const override { return "ES9028PRO"; }

  virtual void Reset() override;

  virtual void Write(std::ostream &os) const override;
  virtual void Read(std::istream &is) override;

  using Device::Read;
  using Device::Write;
  using I2CDevice::Read;
  using I2CDevice::Write;

  virtual void UpdateInfrequent() override;
  virtual void UpdateTimed() override;

  double FSR() const;
  double CLK_FREQ() const { return F_MCLK; }
  double VolumeRampRate() const;
  double SoftStartTime() const;

protected:
  double F_MCLK = 80*1e6;
};

#endif // _ES9028PRO_H_
