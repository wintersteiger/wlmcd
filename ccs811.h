// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CCS811_H_
#define _CCS811_H_

#include "i2c_device.h"

class CCS811 : public I2CDevice<uint8_t, std::vector<uint8_t>> {
public:
  class RegisterTable;
  RegisterTable *RT;

  CCS811(const std::string &bus = "/dev/i2c-1", uint8_t device_address = 0x5A);
  CCS811(const char *config_filename);
  virtual ~CCS811();

  virtual const char* Name() const override { return "CCS811"; }

  using Device::Read;
  using Device::Write;

  virtual void Reset() override;

  virtual void UpdateTimed() override;

  virtual void Write(std::ostream &os) override;
  virtual void Read(std::istream &is) override;

  using I2CDevice<uint8_t, std::vector<uint8_t>>::Bus;
  using I2CDevice<uint8_t, std::vector<uint8_t>>::DeviceAddress;
  using I2CDevice<uint8_t, std::vector<uint8_t>>::Read;
  using I2CDevice<uint8_t, std::vector<uint8_t>>::Write;
  virtual std::vector<uint8_t> Read(const uint8_t &addr) override;
  virtual void Write(const uint8_t &addr, const std::vector<uint8_t> &value) override;
};

#endif // _CCS811_H_
