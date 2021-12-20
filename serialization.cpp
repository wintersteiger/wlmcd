// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "serialization.h"

std::vector<uint8_t> hex_string_to_bytes(const char *s)
{
  size_t vec_sz = strlen(s);
  size_t num_bytes = vec_sz/2 + (vec_sz % 2);
  std::vector<uint8_t> bytes(num_bytes, 0);

  for (size_t i=0; i < bytes.size(); i++)
    bytes[i] = hex_to_byte(*(s+2*i), *(s + (2*i+1)));

  return bytes;
}

std::string to_hex(const std::vector<uint8_t> &bytes)
{
  std::string r;
  r.reserve(bytes.size() * 2 + 1);
  for (auto &b : bytes) {
    char tmp[3];
    sprintf(tmp, "%02x", b);
    r += tmp;
  }
  return r;
}

std::vector<uint8_t> from_hex(const std::string &data)
{
  std::vector<uint8_t> r;
  for (size_t i=0; i < data.size(); i += 2) {
    uint8_t t;
    if (sscanf(data.c_str() + i, "%02hhx", &t) != 1)
      return {};
    r.push_back(t);
  }
  return r;
}
