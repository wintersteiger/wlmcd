// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SPIDEV_H_
#define _SPIDEV_H_

#include <cstdint>
#include <vector>

class SPIDev {
public:
  SPIDev(unsigned bus, unsigned channel, uint32_t speed = 10000000);
  virtual ~SPIDev();

  void Transfer(std::vector<uint8_t> &data) const;
  void Transfer(uint8_t *buf, size_t sz) const;

protected:
  char path[256];
  int fd;
};

#endif // _SPIDEV_H_