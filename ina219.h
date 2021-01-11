// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _INA219_H_
#define _INA219_H_

#include <mutex>

#include "device.h"

class INA219 : public Device<uint8_t, uint16_t> {
public:
  class RegisterTable;
  RegisterTable &RT;
  constexpr static double nan = std::numeric_limits<double>::quiet_NaN();

  INA219(double r_shunt = nan,
         double max_current = nan,
         const std::string &bus = "/dev/i2c-1",
         uint8_t device_address = 0x41);
  virtual ~INA219();

  std::string Bus() const { return bus; }
  uint8_t DeviceAddress() const { return device_address; }

  virtual const char* Name() const { return "INA219"; }

  virtual void Reset();

  virtual void WriteConfig(const std::string &filename);
  virtual void ReadConfig(const std::string &filename);

  virtual uint16_t Read(const uint8_t &addr);
  uint16_t Read(const Register<uint8_t, uint16_t> &reg);
  virtual std::vector<uint16_t> Read(const uint8_t &addr, size_t length);

  virtual void Write(const uint8_t &addr, const uint16_t &value);
  virtual void Write(const Register<uint8_t, uint16_t> &reg, const uint16_t &value);
  virtual void Write(const uint8_t &addr, const std::vector<uint16_t> &values);

  virtual void UpdateTimed();
  virtual void UpdateFrequent();
  virtual void UpdateInfrequent();

  double BusVoltage(void) const;
  double ShuntVoltage(void) const;
  double Current(void) const;
  double Power(void) const;

  void SetBusADCResolution(uint8_t value);
  void SetShuntADCResolution(uint8_t value);

protected:
  std::mutex mtx;
  int fd;
  std::string bus;
  uint8_t device_address;
  double r_shunt, max_expected_current, current_lsb, power_lsb;

  friend void throw_errno(INA219 *ina219, const char* msg);
};

#endif // _INA219_H_
