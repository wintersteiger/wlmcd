// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <cstddef>
#include <vector>
// #include <iostream>
#include <unordered_set>

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "errors.h"
#include "i2c_device.h"

static std::unordered_set<std::string> busses;

I2CDeviceBase::I2CDeviceBase(const std::string &bus, uint8_t device_address) :
  fd(-1),
  bus(bus),
  device_address(device_address)
{
}

void I2CDeviceBase::Reset()
{
  if (busses.find(bus) == busses.end()) {
    // The first time we see a particular bus, we could issue a general command 0x06 (reset)?
    busses.insert(bus);
    // GeneralCall(0x06);
  }

  const std::lock_guard<std::mutex> lock(mtx);

  if (fd >= 0)
    close(fd);

  if ((fd = open(bus.c_str(), O_RDWR)) < 0)
    throw_errno("failed to open the I2C bus");

  if (ioctl(fd, I2C_SLAVE, device_address) < 0)
    throw_errno("failed to acquire bus access and/or talk to slave");
}

void I2CDeviceBase::GeneralCall(uint8_t cmd)
{
  const std::lock_guard<std::mutex> lock(mtx);
  int fd;

  if ((fd = open(bus.c_str(), O_RDWR)) < 0)
    throw_errno("failed to open the I2C bus");

  // This reset all devices on the bus!
  if (ioctl(fd, I2C_SLAVE, 0x00) < 0)
    throw_errno("failed to acquire bus access and/or talk to slave");

  if (write(fd, &cmd, 1) != 1)
    throw_errno("failed to write to the I2C bus");

  close(fd);
}

template<>
uint8_t I2CDevice<uint8_t, uint8_t>::Read(const uint8_t &addr)
{
  uint8_t buf = addr;
  const std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, &buf, 1) != 1)
    throw_errno("failed to write to the I2C bus");
  if (read(fd, &buf, 1) != 1)
    throw_errno("failed to read from the I2C bus");
  // std::cout << " - " << std::hex << (unsigned)addr << " == " << std::hex << buf << " (" << fd << ")" << std::endl;
  return buf;
}

template<>
void I2CDevice<uint8_t, uint8_t>::Write(const uint8_t &addr, const uint8_t &value)
{
  // std::cout << " - " << std::hex << (unsigned)addr << " := " << value << " (" << fd << ")" << std::endl;
  uint8_t buf[2] = { addr, value };
  const std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 2) != 2)
    throw_errno("failed to write to the I2C bus");
}

template<>
uint16_t I2CDevice<uint8_t, uint16_t>::Read(const uint8_t &addr)
{
  uint8_t buf[2] = { addr };
  const std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 1) != 1)
    throw_errno("failed to write to the I2C bus");
  if (read(fd, buf, 2) != 2)
    throw_errno("failed to read from the I2C bus");
  uint16_t r = buf[0] << 8 | buf[1];
  // std::cout << " - " << std::hex << (unsigned)addr << " == " << std::hex << r << " (" << fd << ")" << std::endl;
  return r;
}

template <>
void I2CDevice<uint8_t, uint16_t>::Write(const uint8_t &addr, const uint16_t &value)
{
  // std::cout << " - " << std::hex << (unsigned)addr << " := " << value << " (" << fd << ")" << std::endl;
  uint8_t buf[3];
  buf[0] = addr;
  buf[1] = value >> 8;
  buf[2] = value & 0xFF;
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 3) != 3)
    throw_errno("failed to write to the I2C bus");
}

template<>
std::vector<uint8_t> I2CDevice<uint8_t, std::vector<uint8_t>>::Read(const uint8_t &addr)
{
  return {};
}

template <>
void I2CDevice<uint8_t, std::vector<uint8_t>>::Write(const uint8_t &addr, const std::vector<uint8_t> &value)
{
}
