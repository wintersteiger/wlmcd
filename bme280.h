// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _BME280_H_
#define _BME280_H_

#include "i2c_device.h"

class BME280 : public I2CDevice<uint8_t, uint8_t> {
public:
  class RegisterTable;
  RegisterTable *RT;

  BME280(const std::string &bus = "/dev/i2c-1", uint8_t device_address = 0x76);
  BME280(const char *config_filename);
  virtual ~BME280();

  using I2CDevice::Bus;
  using I2CDevice::DeviceAddress;

  virtual const char* Name() const override { return "BME280"; }

  using Device::Read;
  using Device::Write;
  using I2CDevice::Read;
  using I2CDevice::Write;

  virtual void Reset() override;

  virtual void UpdateTimed() override;

  virtual void Write(std::ostream &os) override;
  virtual void Read(std::istream &is) override;

  virtual void Test(const std::vector<uint8_t> &data) override;

protected:
  std::vector<uint8_t> GetCalibData();
  int8_t ComputeHeaterTemp(uint16_t target_temp, int32_t ambient_temp);
  void Measure();
};

#endif // _BME280_H_
