// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "decoder.h"

std::vector<uint8_t> hex_string_to_bytes(const char *s)
{
  size_t vec_sz = strlen(s);
  size_t num_bytes = vec_sz/2 + (vec_sz % 2);
  std::vector<uint8_t> bytes(num_bytes, 0);

  for (size_t i=0; i < bytes.size(); i++)
    bytes[i] = hex_to_byte(*(s+2*i), *(s + (2*i+1)));

  return bytes;
}