// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <array>
#include <stdexcept>

// #include <openssl/evp.h>

#include <json.hpp>
using json = nlohmann::json;

#include <integrity.h>

#include "enocean.h"

namespace EnOcean
{
  static size_t find_sof(const std::vector<uint8_t> &bytes, size_t from)
  {
    size_t end = bytes.size() * 8;
    for (size_t i = from; i < end; i++) {
      if (get_bit(bytes, i+0) == 0 &&
          get_bit(bytes, i+1) == 1 &&
          get_bit(bytes, i+2) == 1 &&
          get_bit(bytes, i+3) == 0)
      {
        return i;
      }
    }
    return end;
  }

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


  Decoder::Decoder() : ::Decoder()
  {
  }

  Decoder::~Decoder() {}

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

  const std::string& Decoder::Decode(std::vector<uint8_t> &bytes)
  {
    static std::string r;

    for (const auto& frame : get_frames(bytes))
      r += (r.empty() ? "" : " ") + frame.describe();

    return r;
  }

  std::vector<Frame> Decoder::get_frames(const std::vector<uint8_t> &bytes)
  {
    std::vector<Frame> frames;

    const size_t end = bytes.size() * 8;
    size_t sof = 0;
    while ((sof = find_sof(bytes, sof)) < end) {
      size_t pos = sof + 4;
      std::vector<uint8_t> fbytes;

      while (end - pos > 12) {
        uint8_t b[12];
        for (size_t i=0; i < 12; i++)
          b[i] = get_bit(bytes, pos + i);

        if (b[2] + b[3] != 1 || b[6] + b[7] != 1)
          break;

        uint8_t syn = (b[10] << 1) | b[11];
        if (syn != 0x01 && syn != 0x02)
          break;

        fbytes.push_back(b[0] << 7 | b[1] << 6 | b[2] << 5 | b[4] << 4 |
                         b[5] << 3 | b[6] << 2 | b[8] << 1 | b[9] << 0);
        fbytes.back() = ~fbytes.back();

        pos += 12;

        if (syn == 0x01)
          break;
      }

      try {
        frames.emplace_back(std::move(fbytes));
      }
      catch (...) {}

      sof = pos + 1;
    }

    return frames;
  }

  Encoder::Encoder() : ::Encoder()
  {
  }

  Encoder::~Encoder() {}

  std::vector<uint8_t> Encoder::Encode(const std::vector<uint8_t> &data)
  {
    std::vector<uint8_t> r;
    r.reserve(data.size() * 2);
    r.push_back(0x55); // preamble
    uint16_t rem = 0x06; // 4-bit sync sequence
    for (size_t i=0; i < data.size(); i++) {
      uint8_t d = ~data[i];
      uint16_t nb3 = (d & 0x20) != 0 ? 0x00 : 0x01;
      uint16_t nb6 = (d & 0x04) != 0 ? 0x00 : 0x01;
      uint16_t top3 = (d & 0xE0) >> 5;
      uint16_t mid3 = (d & 0x1C) >> 2;
      uint16_t last2 = d & 0x3;
      uint16_t next = i == data.size() - 1 ? 0x01 : 0x02;
      uint16_t e = (top3 << 9) | (nb3 << 8) | (mid3 << 5) | (nb6 << 4) | (last2 << 2) | next;
      if (i % 2 != 0) {
        r.push_back(e >> 4);
        rem = e & 0x000F;
      } else {
        r.push_back((rem << 4) | e >> 8);
        r.push_back(e & 0x00FF);
        rem = 0;
      }
    }
    r.push_back(rem << 4);
    return r;
  }

  std::vector<uint8_t> Encoder::Encode(const Frame &frame)
  {
    return Encode((const std::vector<uint8_t>&)frame);
  }
}