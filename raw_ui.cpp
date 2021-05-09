// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "raw_ui.h"

template <>
bool ValueParser<uint8_t>::Parse(const char *v_str, uint8_t &value) {
  return sscanf(v_str, "%02hhx", &value) == 1;
}

template <>
bool ValueParser<uint16_t>::Parse(const char *v_str, uint16_t &value) {
  return sscanf(v_str, "%04hx", &value) == 1;
}

template <>
bool ValueParser<uint32_t>::Parse(const char *v_str, uint32_t &value) {
  return sscanf(v_str, "%08x", &value) == 1;
}
