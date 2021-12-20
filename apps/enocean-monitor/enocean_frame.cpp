// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <array>
#include <stdexcept>

#include <integrity.h>

#include "enocean_frame.h"

namespace EnOcean
{
  Frame::Frame(std::vector<uint8_t>&& fbytes) :
    buffer(std::move(fbytes))
  {
    if (buffer.size() < 7 || buffer.size() > 21)
      throw std::runtime_error("frame size out of range");
  }

  Frame::Frame(uint8_t rorg, const std::vector<uint8_t> &payload, TXID source, uint8_t status) {
    buffer.clear();
    buffer.push_back(rorg);
    for (auto b : payload)
      buffer.push_back(b);
    buffer.push_back((source >> 24) & 0xFF);
    buffer.push_back((source >> 16) & 0xFF);
    buffer.push_back((source >> 8) & 0xFF);
    buffer.push_back(source & 0xFF);
    buffer.push_back(status | 0x80);
    buffer.push_back(crc8(buffer, 0x07));
  }

  Frame::~Frame() {}

  uint8_t Frame::rorg() const { return buffer[0]; }

  const uint8_t* Frame::data() const { return &buffer[1]; }

  TXID Frame::txid() const {
    return (buffer[buffer.size()-6] << 24) |
           (buffer[buffer.size()-5] << 16) |
           (buffer[buffer.size()-4] << 8) |
            buffer[buffer.size()-3];
  }

  uint8_t Frame::status() const { return buffer[buffer.size()-2]; }

  uint8_t Frame::hash() const { return buffer[buffer.size()-1]; }

  size_t Frame::num_repeater_hops() const { return status() & 0x0F; }

  Frame::IntegrityMechanism Frame::integrity_mechanism(bool skip_last) const {
    size_t status_inx = skip_last ? buffer.size() - 2 : buffer.size() - 1;
    if (status_inx >= buffer.size()) throw std::logic_error("invalid frame");
    return (buffer[status_inx] & 0x80) != 0 ? IntegrityMechanism::CRC8 : IntegrityMechanism::Checksum;
  }

  bool Frame::crc_ok() const
  {
    switch (integrity_mechanism(true))
    {
      case IntegrityMechanism::CRC8:
        return crc8(buffer, 0x07, true) == buffer.back();
      case IntegrityMechanism::Checksum:
        return checksum(buffer, true) == buffer.back();
      default:
        throw std::logic_error("invalid integrity mechanism");
    }
  }

  std::string Frame::describe() const {
    static char tmp[2048];

    char* p = tmp;
    *p = 0;

    for (size_t i=0; i < size(); i++)
      p += snprintf(p, sizeof(tmp) - (p-tmp), "%02x", buffer[i] & 0xFF);
    if (!crc_ok())
      p += snprintf(p, sizeof(tmp) - (p-tmp), "!");

    return std::string(tmp, p - tmp);
  }
}