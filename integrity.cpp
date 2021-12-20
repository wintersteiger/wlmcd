// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstdint>

#include "integrity.h"

uint8_t crc8(const std::vector<uint8_t> &data, uint8_t polynomial, bool skip_last)
{
  return crc8(data.data(), data.size(), polynomial, skip_last);
}

uint8_t crc8(const uint8_t *data, size_t size, uint8_t polynomial, bool skip_last)
{
  uint8_t rem = 0;

  if (skip_last)
    size--;

  for (size_t i = 0; i < size; i++)
  {
    rem = rem ^ data[i];
    for (size_t j = 0; j < 8; j++)
    {
      if (rem & 0x80)
        rem = (rem << 1) ^ polynomial;
      else
        rem = (rem << 1);
    }
  }

  return rem;
}

uint8_t checksum(const std::vector<uint8_t> &data, bool skip_last)
{
  uint8_t r = 0;
  for (auto b : data)
    r += b;
  if (skip_last)
    r -= data.back();
  return r;
}

uint8_t checkxor(const std::vector<uint8_t> &data, bool skip_last)
{
  uint8_t r = 0;
  for (auto it = data.begin(); it != (data.end() - (skip_last ? 1 : 0)); it++)
    r ^= *it;
  return r;
}