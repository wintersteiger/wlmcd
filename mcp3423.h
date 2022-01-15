// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _MCP3423_H_
#define _MCP3423_H_

#include <mutex>

#include "i2c_device.h"

class MCP3423 : public I2CDevice<uint8_t, uint8_t> {
public:
  class RegisterTable;
  RegisterTable &RT;

  MCP3423(const std::string &bus = "/dev/i2c-1", uint8_t device_address = 0x68);
  virtual ~MCP3423();

  std::string Bus() const { return bus; }
  uint8_t DeviceAddress() const { return device_address; }

  virtual const char* Name() const override { return "MCP3423"; }

  virtual void Write(std::ostream &os) const override;
  virtual void Read(std::istream &is) override;

  using Device::Read;
  using Device::Write;

  virtual void Reset() override;

  const std::vector<double>& Readings();
  const std::vector<int32_t>& RawReadings();
  double Reading() const { return last_reading; }
  int32_t RawReading() const { return last_raw_reading; }
  void Write(const uint8_t &config);

  virtual void UpdateInfrequent() override;

  virtual uint8_t Read(const uint8_t &addr) override;
  virtual std::vector<uint8_t> Read(const uint8_t &addr, size_t length) override;
  virtual void Write(const uint8_t &addr, const uint8_t &value) override;
  virtual void Write(const uint8_t &addr, const std::vector<uint8_t> &values) override;

  virtual void UpdateTimed() override;

protected:
  std::vector<double> last_readings;
  std::vector<int32_t> last_raw_readings;
  double last_reading;
  int32_t last_raw_reading;
};

#endif // _MCP3423_H_
