// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _DECODER_H_
#define _DECODER_H_

#include <cstdint>

#include <string>
#include <vector>

inline uint8_t hex_char_to_byte(char c)
{
  if ('0' <= c && c <= '9')
    return c - '0';
  else if ('a' <= c && c <= 'f')
    return 10 + (c - 'a');
  else if ('A' <= c && c <= 'F')
    return 10 + (c - 'A');
  return 0x00;
}

inline uint8_t hex_to_byte(char c1, char c2)
{
  return hex_char_to_byte(c1) << 4 | hex_char_to_byte(c2);
}

extern std::vector<uint8_t> hex_string_to_bytes(const char *s);


inline uint8_t get_bit(const std::vector<uint8_t> &buf, size_t i)
{
  size_t byte_inx = i / 8;
  uint8_t byte = buf[byte_inx];
  size_t bit_inx = i % 8;
  return (byte & (0x01 << (8 - bit_inx - 1))) != 0;
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
