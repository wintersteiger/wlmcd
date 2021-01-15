// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <vector>

#include "spidev.h"

SPIDev::SPIDev(unsigned bus, unsigned channel, uint32_t speed) :
  fd(-1)
{
  snprintf(path, sizeof(path), "/dev/spidev%d.%d", bus, channel);

  if ((fd = open(path, O_RDWR)) == -1)
    throw std::runtime_error("Unable to open SPI device");

  uint32_t mode = SPI_MODE_0;
  if (ioctl(fd, SPI_IOC_WR_MODE32, &mode) == -1)
    throw std::runtime_error("Unable to set SPI mode");

  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1)
    throw std::runtime_error("Unable to set SPI speed");

  uint8_t bpw = 8;
  if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bpw) == -1)
    throw std::runtime_error("Unable to set SPI bits per word");
}

SPIDev::~SPIDev()
{
  if (fd != -1)
    close(fd);
}

void SPIDev::Transfer(std::vector<uint8_t> &data) const
{
  std::vector<uint8_t> rx;
  struct spi_ioc_transfer spi;

  rx.resize(data.size());

  memset(&spi, 0, sizeof (spi));
  spi.tx_buf        = (uint64_t)data.data();
  spi.rx_buf        = (uint64_t)rx.data();
  spi.len           = data.size();

  if (ioctl(fd, SPI_IOC_MESSAGE(1), &spi) == -1)
    throw std::runtime_error("SPI transfer failed");

  data.swap(rx);
}

void SPIDev::Transfer(uint8_t *buf, size_t sz) const
{
  struct spi_ioc_transfer spi;

  memset(&spi, 0, sizeof (spi));
  spi.tx_buf        = (uint64_t)buf;
  spi.rx_buf        = (uint64_t)buf;
  spi.len           = sz;

  if (ioctl(fd, SPI_IOC_MESSAGE(1), &spi) == -1)
    throw std::runtime_error("SPI transfer failed");
}