// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _I2C_DEVICE_H_
#define _I2C_DEVICE_H_

#include <mutex>
#include <cstddef>
#include <vector>
#include <string>

#include "device.h"

template <typename AT, typename VT>
class I2CDevice : public Device<AT, VT>{
public:
  I2CDevice(const std::string &bus, uint8_t device_address);

  virtual ~I2CDevice();

  std::string Bus() const { return bus; }
  uint8_t DeviceAddress() const { return device_address; }

  void Reset();

  VT Read(const uint8_t &addr);

  std::vector<VT> Read(const uint8_t &addr, size_t length)
  {
    std::vector<uint8_t> r(length);
    for (size_t i=0; i < length; i++)
      r[i] = Read(addr + i);
    return r;
  }

  void Write(const uint8_t &addr, const VT &value);

  void Write(const uint8_t &addr, const std::vector<VT> &values)
  {
    for (size_t i=0; i < values.size(); i++)
      Write(addr + i, values[i]);
  }

protected:
  std::mutex mtx;
  int fd;
  std::string bus;
  uint8_t device_address;
};

#endif // _I2C_DEVICE_H_
