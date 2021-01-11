// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <cstdint>

#include <string>
#include <vector>

class Encoder {
public:
  Encoder() {}
  virtual ~Encoder() {}

  virtual std::vector<uint8_t> Encode(const std::vector<uint8_t> &data) = 0;
};

#endif
