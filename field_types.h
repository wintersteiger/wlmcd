// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _FIELD_TYPES_H_
#define _FIELD_TYPES_H_

#include <cinttypes>
#include <vector>

typedef std::vector<uint8_t> bytes_t;
typedef std::pair<uint8_t, uint8_t> pair_uint8_t;

inline bytes_t mk_bytes(uint8_t x1, uint8_t x2) {
  return {x1, x2};
}

inline bytes_t mk_bytes(uint8_t x1, uint8_t x2, uint8_t x3) {
  return {x1, x2, x3};
}

typedef std::pair<double, bool> opt_double;
typedef std::pair<uint64_t, bool> opt_uint64_t;

#endif
