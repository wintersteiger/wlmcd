// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _INTEGRITY_H_
#define _INTEGRITY_H_

#include <cstdint>
#include <vector>

uint8_t crc8(const std::vector<uint8_t> &data, uint8_t polynomial = 0x07, bool skip_last = false);
uint8_t crc8(const uint8_t *data, size_t size, uint8_t polynomial = 0x07, bool skip_last = false);

uint16_t crc16(const std::vector<uint8_t> &data, uint16_t polynomial, uint16_t init = 0x0000, uint16_t xorout = 0x0000, bool skip_last = false);
uint16_t crc16(const uint8_t *data, size_t size, uint16_t polynomial, uint16_t init = 0x0000, uint16_t xorout = 0x0000, bool skip_last = false);

uint8_t checksum(const std::vector<uint8_t> &data, bool skip_last = false);

uint8_t checkxor(const std::vector<uint8_t> &data, bool skip_last = false);

#endif //  _INTEGRITY_H_
