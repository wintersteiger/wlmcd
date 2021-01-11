// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <vector>

#include "basic.h"

static const uint8_t PKTLEN = 64;


Basic::State::State() {}

Basic::State::~State() {}

void Basic::State::Update(const std::vector<uint8_t> &msg)
{

}


Basic::Decoder::Decoder() {}

Basic::Decoder::~Decoder() {}

const std::string& Basic::Decoder::Decode(std::vector<uint8_t> &bytes)
{
  char *p = tmp;
  for (size_t i=0; i < PKTLEN; i++)
    p += snprintf(p, sizeof(tmp), "%02x", bytes[i]);
  message.str = std::string(tmp, p);
  return message.str;
}


Basic::Encoder::Encoder() {}

Basic::Encoder::~Encoder() {}

std::vector<uint8_t> Basic::Encoder::Encode(const std::vector<uint8_t> &data)
{
  std::vector<uint8_t> r;
  // r.push_back(data.size());
  for (size_t i=0; i < data.size() && i < PKTLEN; i++)
    r.push_back(data[i]);
  for (size_t i=data.size(); i < PKTLEN; i++)
    r.push_back(0);
  return r;
}
