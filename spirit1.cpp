// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "json.hpp"
using json = nlohmann::json;

#include "spirit1.h"
#include "spirit1_rt.h"

SPIRIT1::SPIRIT1(unsigned spi_bus, unsigned spi_channel, const std::string &config_file, double f_xo) :
  SPIDev(spi_bus, spi_channel, 10000000),
  RT(new RegisterTable(*this)),
  f_xo(f_xo)
{
  FindAddressBlocks();
  Reset();
  RT->Refresh(false);
  if (RT->PARTNUM_7_0() != 0x01 || RT->VERSION_7_0() != 0x30)
    throw std::runtime_error("unknown part no/version");

  if (!config_file.empty()) {
    auto is = std::ifstream(config_file);
    Read(is);
  }
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

  if (F_xo() > 26e6) {
    uint8_t rv = Read(RT->_rSYNTH_CONFIG_1.Address());
    rv = RT->_vREFDIV.Set(rv, 1);
    Write(RT->_rSYNTH_CONFIG_1.Address(), rv);
  }
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

void SPIRIT1::Execute(SPIRIT1::Command cmd)
{
  const std::lock_guard<std::mutex> lock(mtx);
  std::vector<uint8_t> b = { 0x80, static_cast<uint8_t>(cmd) };
  SPIDev::Transfer(b);
}

void SPIRIT1::Receive(std::vector<uint8_t> &pkt) {}
void SPIRIT1::Transmit(const std::vector<uint8_t> &pkt) {}

void SPIRIT1::UpdateFrequent() { RT->Refresh(true); }
void SPIRIT1::UpdateInfrequent() { RT->Refresh(false); }

void SPIRIT1::Write(std::ostream &os) { RT->Write(os); }
void SPIRIT1::Read(std::istream &is) { RT->Read(is); }

double SPIRIT1::rFrequency() const
{
  uint32_t SYNT = (RT->SYNT_25_21() << 21) | (RT->SYNT_20_13() << 13) | (RT->SYNT_12_5() << 5) | RT->SYNT_4_0();
  double B = 1.0;
  switch (RT->BS()) {
    case 1: B = 6.0; break;
    case 3: B = 12.0; break;
    case 4: B = 16.0; break;
    case 5: B = 32.0; break;
  }
  double D = RT->REFDIV() == 0 ? 1.0 :  2.0;
  double Q = F_xo() / ((B*D)/2.0);
  return Q * (SYNT/pow(2, 18));
}

void SPIRIT1::setFrequency(double f)
{
  double B = 1.0;
  switch (RT->BS()) {
    case 1: B = 6.0; break;
    case 3: B = 12.0; break;
    case 4: B = 16.0; break;
    case 5: B = 32.0; break;
  }
  double D = RT->REFDIV() == 0x00 ? 1.0 :  2.0;
  double Q = F_xo() / ((B*D)/2.0);
  uint32_t SYNT = (f * pow(2, 18)) / Q;
  RT->Write(RT->_rSYNT0, RT->_vSYNT_4_0, SYNT & 0x1F);
  RT->Write(RT->_rSYNT1, RT->_vSYNT_12_5, (SYNT >> 5) & 0xFF);
  RT->Write(RT->_rSYNT2, RT->_vSYNT_20_13, (SYNT >> 13) & 0xFF);
  RT->Write(RT->_rSYNT3, RT->_vSYNT_25_21, (SYNT >> 21) & 0x1F);
}

double SPIRIT1::rDeviation() const
{
  return (F_xo() * ((8 + RT->FDEV_M()) << (RT->FDEV_E() - 1))) / pow(2, 18);
}

double SPIRIT1::rDatarate() const
{
  return (F_clk() * ((256 + RT->DATARATE_M()) << RT->DATARATE_E())) / pow(2, 28);
}

static float filter_bandwidths_24[9][10] = {
  { 738.6, 416.2, 207.4, 103.7, 51.8, 25.8, 12.9, 6.5, 3.2, 1.7 },
  { 733.9, 393.1, 196.1,  98.0, 48.9, 24.5, 12.3, 6.1, 3.0, 1.6 },
  { 709.3, 372.2, 185.6,  92.8, 46.3, 23.2, 11.6, 5.8, 2.9, 1.5 },
  { 680.1, 351.5, 175.4,  87.7, 43.8, 21.9, 11.0, 5.4, 2.8, 1.4 },
  { 650.9, 334.2, 166.8,  83.4, 41.6, 20.9, 10.4, 5.2, 2.6, 1.3 },
  { 619.3, 315.4, 157.5,  78.7, 39.3, 19.7,  9.8, 4.9, 2.5, 1.2 },
  { 592.9, 300.4, 149.9,  75.0, 37.5, 18.7,  9.3, 4.7, 2.3, 1.2 },
  { 541.6, 271.8, 135.8,  67.8, 33.9, 17.0,  8.5, 4.2, 2.1, 1.1 },
  { 499.8, 249.5, 124.6,  62.3, 31.1, 15.6,  7.8, 3.9, 1.9, 1.0 }
};

double SPIRIT1::rFilterBandwidth() const
{
  return filter_bandwidths_24[RT->CHFLT_M_3_0()][RT->CHFLT_E_3_0()] * (F_clk()/24e6);
}

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
    device.f_clk = PD_CLKDIV() ? device.f_xo : device.f_xo / 2.0;
  }
}

void SPIRIT1::RegisterTable::Write(std::ostream &os)
{
  json j, dev, regs;
  char tmp[32];
  dev["name"] = device.Name();
  j["device"] = dev;
  for (const auto reg : registers) {
    if (reg->Writeable()) {
      snprintf(tmp, sizeof(tmp), "%02x", (*this)(*reg));
      regs[reg->Name()] = tmp;
    }
  }
  j["registers"] = regs;
  os << std::setw(2) << j << std::endl;
}

void SPIRIT1::RegisterTable::Read(std::istream &is)
{
  json j = json::parse(is);

  if (j["device"]["name"] != device.Name())
    throw std::runtime_error("device mismatch");

  for (const auto &e : j["registers"].items())
  {
    if (!e.value().is_string())
      throw std::runtime_error(std::string("invalid value for '" + e.key() + "'"));
    std::string sval = e.value().get<std::string>();
    if (sval.size() != 2)
      throw std::runtime_error(std::string("invalid value length for '" + e.key() + "'"));
    bool found = false;
    for (const auto reg : registers)
    {
      if (reg->Name() == e.key() && reg->Writeable()) {
        uint8_t val;
        sscanf(sval.c_str(), "%02hhx", &val);
        device.Write(*reg, val);
        found = true;
        break;
      }
    }
    if (!found)
      throw std::runtime_error(std::string("invalid register '") + e.key() + "'");
  }
}
