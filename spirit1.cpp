// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "spirit1.h"
#include "spirit1_rt.h"

SPIRIT1::SPIRIT1(unsigned spi_bus, unsigned spi_channel, double f_xo) :
  SPIDev(spi_bus, spi_channel, 1000000),
  RT(new RegisterTable(*this)),
  f_xo(f_xo)
{
  FindAddressBlocks();

  Reset();
  RT->Refresh(false);
  if (RT->PARTNUM_7_0() != 0x01 || RT->VERSION_7_0() != 0x30)
    throw std::runtime_error("unknown part no/version");
}

SPIRIT1::~SPIRIT1() {}

void SPIRIT1::FindAddressBlocks()
{
  if (address_blocks.empty())
  {
    bool used_addrs[256] = {false};
    for (const auto &r : *RT)
      used_addrs[r->Address()] = true;
    size_t start = SIZE_MAX;
    for (size_t i=0; i < 256; i++)
    {
      if (used_addrs[i]) {
        if (start == SIZE_MAX)
          start = i;
      }
      else if (start != SIZE_MAX) {
        address_blocks.push_back(std::make_pair(start, i-1));
        start = SIZE_MAX;
      }
    }
  }
}

void SPIRIT1::Reset()
{
  std::vector<uint8_t> cmd(2, 0);
  cmd[0] = 0x80;
  cmd[1] = 0x70;
  SPIDev::Transfer(cmd);
}

uint8_t SPIRIT1::Read(const uint8_t &addr)
{
  const std::lock_guard<std::mutex> lock(mtx);
  std::vector<uint8_t> res(3, 0);
  res[0] = 0x01;
  res[1] = addr;
  SPIDev::Transfer(res);
  status_bytes[0] = res[0];
  status_bytes[1] = res[1];
  return res[2];
}

std::vector<uint8_t> SPIRIT1::Read(const uint8_t &addr, size_t length)
{
  const std::lock_guard<std::mutex> lock(mtx);
  std::vector<uint8_t> res(length + 2, 0);
  res[0] = 0x01;
  res[1] = addr;
  SPIDev::Transfer(res);
  status_bytes[0] = res[0];
  status_bytes[1] = res[1];
  res.erase(res.begin());
  res.erase(res.begin());
  return res;
}

void SPIRIT1::Write(const uint8_t &addr, const uint8_t &value)
{
  const std::lock_guard<std::mutex> lock(mtx);
  std::vector<uint8_t> b(3);
  b[0] = 0x00;
  b[1] = addr;
  b[2] = value;
  SPIDev::Transfer(b);
}

void SPIRIT1::Write(const uint8_t &addr, const std::vector<uint8_t> &values)
{
  const std::lock_guard<std::mutex> lock(mtx);
  size_t n = values.size();
  std::vector<uint8_t> b(n+2);
  b[0] = 0x00;
  b[1] = addr;
  memcpy(&b[2], values.data(), n);
  SPIDev::Transfer(b);
}

void SPIRIT1::Receive(std::vector<uint8_t> &pkt) {}
void SPIRIT1::Transmit(const std::vector<uint8_t> &pkt) {}

void SPIRIT1::UpdateFrequent() { RT->Refresh(true); }
void SPIRIT1::UpdateInfrequent() { RT->Refresh(false); }

void SPIRIT1::Write(std::ostream &os) {}
void SPIRIT1::Read(std::istream &is) {}

void SPIRIT1::RegisterTable::Refresh(bool frequent)
{
  const size_t buffer_size = 256;
  if (buffer.size() != buffer_size) {
    device.mtx.lock();
    buffer.resize(buffer_size);
    device.mtx.unlock();
  }
  if (!frequent) {
    for (const auto &b : device.address_blocks) {
      size_t sz = b.second-b.first+1;
      auto tmp = device.Read(b.first, sz);
      memcpy(buffer.data() + b.first, tmp.data(), sz);
    }
  }
}

void SPIRIT1::RegisterTable::Write(std::ostream &os) {}
void SPIRIT1::RegisterTable::Read(std::istream &is) {}