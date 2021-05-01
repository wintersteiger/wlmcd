// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _I2C_DEVICE_H_
#define _I2C_DEVICE_H_

#include <unistd.h>

#include <mutex>
#include <cstddef>
#include <vector>
#include <string>

#include "device.h"

class I2CDeviceBase {
public:
  I2CDeviceBase(const std::string &bus, uint8_t device_address) :
    bus(bus),
    device_address(device_address)
  {}

  virtual ~I2CDeviceBase() {
    if (fd >= 0)
      close(fd);
  }

  virtual void Reset();

protected:
  std::mutex mtx;
  int fd;
  std::string bus;
  uint8_t device_address;
};

template <typename AT, typename VT>
class I2CDevice : public Device<AT, VT>, I2CDeviceBase {
public:
  I2CDevice(const std::string &bus, uint8_t device_address) :
    I2CDeviceBase(bus, device_address)
  {}

  virtual ~I2CDevice() = default;

  std::string Bus() const { return bus; }
  uint8_t DeviceAddress() const { return device_address; }

  using I2CDeviceBase::Reset;

  VT Read(const uint8_t &addr);

  virtual std::vector<VT> Read(const uint8_t &addr, size_t length) override
  {
    std::vector<VT> r(length);
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
  using I2CDeviceBase::mtx;
  using I2CDeviceBase::fd;
  using I2CDeviceBase::bus;
  using I2CDeviceBase::device_address;
};

#endif // _I2C_DEVICE_H_
