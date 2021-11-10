// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RADIO_H_
#define _RADIO_H_

#include <cstdint>
#include <vector>

#include "device.h"

class Radio
{
public:
  Radio() {}
  virtual ~Radio() {}

  enum class State { Idle = 0, RX = 1 };

  virtual void Goto(State state) = 0;
  virtual void Receive(std::vector<uint8_t> &packet) = 0;
  virtual void Transmit(const std::vector<uint8_t> &packet) = 0;

  virtual uint64_t IRQHandler() { return 0; }
};

#endif // _RADIO_H_
