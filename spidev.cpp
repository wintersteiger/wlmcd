// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include <cstring>
#include <stdexcept>
#include <vector>

void Read(std::vector<uint8_t> &data, size_t len=0x70)
{
  data.resize(0x71, 0);

  int fd ;
  char spiDev [32];
  snprintf (spiDev, 31, "/dev/spidev0.%d", 0);
  if ((fd = open (spiDev, O_RDWR)) < 0)
    throw std::runtime_error("Unable to open SPI device");

  data[0] = 0x01;
  struct spi_ioc_transfer spi;
  memset (&spi, 0, sizeof (spi)) ;
  spi.tx_buf        = (unsigned long)data.data() ;
  spi.rx_buf        = (unsigned long)(data.data() + 1);
  spi.len           = len;
  spi.delay_usecs   = 0;
  spi.speed_hz      = 10000000;
  spi.bits_per_word = 8;
  ioctl (fd, SPI_IOC_MESSAGE(1), &spi);
}
