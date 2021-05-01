// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _INA219_H_
#define _INA219_H_

#include <mutex>

#include "i2c_device.h"

class INA219 : public I2CDevice<uint8_t, uint16_t> {
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

  virtual void Reset() override;

  using I2CDevice::Read;
  using I2CDevice::Write;

  virtual void Write(std::ostream &os);
  virtual void Read(std::istream &is);

  virtual void UpdateTimed() override;

  double BusVoltage(void) const;
  double ShuntVoltage(void) const;
  double Current(void) const;
  double Power(void) const;

  typedef enum { _16V, _32V } BusVoltageRange;
  void SetBusVoltageRange(BusVoltageRange range = _32V);
  typedef enum { _40mV, _80mV, _160mV, _320mV } PGARange;
  void SetPGARange(PGARange range = _320mV);
  void SetBusADCResolution(uint8_t value);
  void SetShuntADCResolution(uint8_t value);

protected:
  double r_shunt, max_expected_current, current_lsb, power_lsb;
};

#endif // _INA219_H_
