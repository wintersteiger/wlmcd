// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <cstddef>
#include <vector>

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "i2c_device.h"

static void throw_errno(const char* msg)
{
  char exmsg[1024];
  snprintf(exmsg, sizeof(exmsg), "%s (%s (%d))", msg, strerror(errno), errno);
  throw std::runtime_error(exmsg);
}

void I2CDeviceBase::Reset()
{
  std::lock_guard<std::mutex> lock(mtx);

  if (fd >= 0)
    close(fd);

  if ((fd = open(bus.c_str(), O_RDWR)) < 0)
    throw_errno("failed to open the I2C bus");

  if (ioctl(fd, I2C_SLAVE, device_address) < 0)
    throw_errno("failed to acquire bus access and/or talk to slave");
}

template<>
uint8_t I2CDevice<uint8_t, uint8_t>::Read(const uint8_t &addr)
{
  uint8_t buf = addr;
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, &buf, 1) != 1)
    throw_errno("failed to write to the I2C bus");
  if (read(fd, &buf, 1) != 1)
    throw_errno("failed to read from the I2C bus");
  return buf;
}

template<>
void I2CDevice<uint8_t, uint8_t>::Write(const uint8_t &addr, const uint8_t &value)
{
  uint8_t buf[2] = { addr, value };
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 2) != 2)
    throw_errno("failed to write to the I2C bus");
}

template<>
uint16_t I2CDevice<uint8_t, uint16_t>::Read(const uint8_t &addr)
{
  uint8_t buf[2] = { addr };
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 1) != 1)
    throw_errno("failed to write to the I2C bus");
  if (read(fd, buf, 2) != 2)
    throw_errno("failed to read from the I2C bus");
  return buf[0] << 8 | buf[1];
}

template <>
void I2CDevice<uint8_t, uint16_t>::Write(const uint8_t &addr, const uint16_t &value)
{
  uint8_t buf[3];
  buf[0] = addr;
  buf[1] = value >> 8;
  buf[2] = value & 0xFF;
  std::lock_guard<std::mutex> lock(mtx);
  if (write(fd, buf, 3) != 3)
    throw_errno("failed to write to the I2C bus");
}
