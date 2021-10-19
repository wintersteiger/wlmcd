// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include <openssl/evp.h>

#include <json.hpp>
using json = nlohmann::json;

#include "enocean.h"

EnOcean::State::State() :
  ::State()
{}

void EnOcean::State::Update(const std::vector<uint8_t> &msg)
{
}

EnOcean::Decoder::Decoder() : ::Decoder()
{
}

EnOcean::Decoder::~Decoder() {}

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

static uint8_t crc8(const std::vector<uint8_t> &data, uint8_t polynomial, bool skip_last = false)
{
  uint8_t rem = 0;
  size_t sz = data.size();

  if (skip_last)
    sz--;

  for (size_t i = 0; i < sz; i++)
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

const std::string& EnOcean::Decoder::Decode(std::vector<uint8_t> &bytes)
{
  char* p = tmp;
  *p = 0;

  std::vector<uint8_t> frame;

  const size_t end = bytes.size() * 8;
  size_t sof = 0;
  while ((sof = find_sof(bytes, sof)) < end)
  {
    size_t pos = sof + 4;
    frame.clear();

    while (end - pos > 12)
    {
      uint8_t b[12];
      for (size_t i=0; i < 12; i++)
        b[i] = get_bit(bytes, pos + i);

      if (b[2] + b[3] != 1 || b[6] + b[7] != 1)
        {
        frame.clear();
        break;
      }

      uint8_t syn = (b[10] << 1) | b[11];
      if (syn != 0x01 && syn != 0x02)
        {
        frame.clear();
        break;
      }

      frame.push_back(b[0] << 7 | b[1] << 6 | b[2] << 5 | b[4] << 4 |
                      b[5] << 3 | b[6] << 2 | b[8] << 1 | b[9] << 0);
      frame.back() = ~frame.back();

      pos += 12;

      if (syn == 0x01)
        break;
    }

    bool crc_ok = true;

    if (frame.size() <= 2)
      frame.clear();
    else
      crc_ok = crc8(frame, 0x07, true) == frame.back();

    if (!frame.empty()) {
      if (p != tmp)
        p += snprintf(p, sizeof(tmp) - (p-tmp), " ");
      for (uint8_t b : frame)
        p += snprintf(p, sizeof(tmp) - (p-tmp), "%02x", b & 0xFF);
      if (!crc_ok)
        p += snprintf(p, sizeof(tmp) - (p-tmp), "!");
    }

    sof = pos + 1;
  }

  tmp_str = std::string(tmp, p - tmp);
  return tmp_str;
}

EnOcean::Encoder::Encoder() : ::Encoder()
{
}

EnOcean::Encoder::~Encoder() {}

std::vector<uint8_t> EnOcean::Encoder::Encode(const std::vector<uint8_t> &data)
{
  return {};
}
