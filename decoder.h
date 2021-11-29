// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _DECODER_H_
#define _DECODER_H_

#include <cstdint>

#include <string>
#include <vector>
#include <array>

inline uint8_t get_bit(const std::vector<uint8_t> &buf, size_t i)
{
  size_t byte_inx = i / 8;
  uint8_t byte = buf[byte_inx];
  size_t bit_inx = i % 8;
  return (byte & (0x01 << (8 - bit_inx - 1))) != 0 ? 0x01 : 0x00;
}

inline uint8_t get_byte(const std::vector<uint8_t> &buf, size_t i)
{
  uint8_t r = 0;
  for (size_t j=i; j < i+8; j++)
    r = (r << 1) | get_bit(buf, j);
  return r;
}

class Decoder {
public:
  Decoder() {}
  virtual ~Decoder() {}

  virtual const std::string& Decode(std::vector<uint8_t> &bytes) = 0;
};

#endif
