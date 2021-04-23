// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <cmath>
#include <unistd.h>

#include <vector>
#include <fstream>
#include <iomanip>

#include <gpiod.h>

#include "json.hpp"
#include "sleep.h"
#include "rfm69.h"
#include "rfm69_rt.h"

using json = nlohmann::json;

RFM69::RFM69(
  unsigned spi_bus, unsigned spi_channel,
  const std::string &config_file,
  double f_xosc) :
  Device(),
  SPIDev(spi_bus, spi_channel, 10000000),
  RT(new RegisterTable(*this)),
  spi_channel(spi_channel),
  f_xosc(f_xosc),
  f_step(f_xosc / pow(2, 19))
  // ,
  // recv_buf(new uint8_t[1024]),
  // recv_buf_sz(1024), recv_buf_begin(0), recv_buf_pos(0)
{
  Reset();

  SetMode(Mode::STDBY);

  if (!config_file.empty()) {
    auto is = std::ifstream(config_file);
    Read(is);
  }

  SetMode(Mode::RX);
  RT->Refresh(false);
}

RFM69::~RFM69() {
  Reset();
  SetMode(Mode::SLEEP);
  // delete[](recv_buf);
  delete(RT);
}

void RFM69::Reset()
{
  ClearFlags();
  SetMode(Mode::STDBY);
}

uint8_t RFM69::Read(const uint8_t &addr)
{
  mtx.lock();
  std::vector<uint8_t> res(2);
  res[0] = addr & 0x7F;
  SPIDev::Transfer(res);
  mtx.unlock();
  return res[1];
}

std::vector<uint8_t> RFM69::Read(const uint8_t &addr, size_t length)
{
  mtx.lock();
  std::vector<uint8_t> res;
  res.resize(length + 1);
  res[0] = addr & 0x7F;
  SPIDev::Transfer(res);
  res.erase(res.begin());
  mtx.unlock();
  return res;
}

void RFM69::Write(const uint8_t &addr, const uint8_t &value)
{
  mtx.lock();
  std::vector<uint8_t> buf(2);
  buf[0] = 0x80 | (addr & 0x7F);
  buf[1] = value;
  SPIDev::Transfer(buf);
  mtx.unlock();
}

void RFM69::Write(const uint8_t &addr, const std::vector<uint8_t> &values)
{
  mtx.lock();
  size_t n = values.size();
  std::vector<uint8_t> buf(n+1);
  buf[0] = 0x80 | (addr & 0x7F);;
  memcpy(&buf[1], values.data(), n);
  SPIDev::Transfer(buf);
  mtx.unlock();
}

RFM69::Mode RFM69::GetMode()
{
  return (Mode)RT->_vMode(Read(RT->_rOpMode));
}

void RFM69::SetMode(Mode m)
{
  uint8_t nv = RT->_vMode.Set(Read(RT->_rOpMode), m);
  Write(RT->_rOpMode, nv);

  size_t limit = 50;
  do {
    if (GetMode() == m)
      return;
    sleep_us(10);
  } while(--limit);

  // Device did not react after `limit` tries.
  responsive = false;
}

void RFM69::UpdateFrequent()
{
  RT->Refresh(true);
}

void RFM69::UpdateInfrequent()
{
  RT->Refresh(false);
}

void RFM69::Write(std::ostream &os)
{
  RT->Write(os);
}

void RFM69::Read(std::istream &is)
{
  RT->Read(is);
}

void RFM69::ClearFlags()
{
  uint8_t irq1 = Read(RT->_rIrqFlags1);
  uint8_t irq2 = Read(RT->_rIrqFlags2);
  Write(RT->_rIrqFlags1, irq1 | 0x09); // RSSI, SyncAddressMatch
  Write(RT->_rIrqFlags2, irq2 | 0x10); // FifoOverrun
}

void RFM69::Receive(std::vector<uint8_t> &packet)
{
  uint8_t length = Read(RT->_rPayloadLength);
  // packet = Read(0x00, length);
  packet.reserve(length);
  packet.resize(0);
  for (; length > 0; length--)
    packet.push_back(Read(RT->_rFifo));


  // uint8_t crc;

  // size_t rxbytes_last = 0, rxbytes = 1;
  // while (true)
  // {
  //   do {
  //     rxbytes_last = rxbytes;
  //     rxbytes = 128; // number of bytes in fifo not available?
  //   } while (rxbytes != rxbytes_last);

  //   bool overflow = (rxbytes & 0x80) != 0;
  //   size_t n = rxbytes & 0x7F;

  //   size_t m = n <= 1 ? n : n-1;

  //   if (n == 0)
  //     break;
  //   else if (overflow) {
  //     break;
  //   }
  //   else
  //   {
  //     std::vector<uint8_t> buf = Read(RT->_rFifo.Address(), m);
  //     for (uint8_t &bi : buf) {
  //       recv_buf[recv_buf_pos++] = bi;
  //       recv_buf_pos %= recv_buf_sz;
  //     }
  //   }

  //   sleep_us(1000); // give the FIFO a chance to catch up
  // }

  // size_t pkt_sz = recv_buf_held(), i=0;
  // packet.resize(pkt_sz);
  // while (recv_buf_begin != recv_buf_pos) {
  //   packet[i++] = recv_buf[recv_buf_begin++];
  //   recv_buf_begin %= recv_buf_sz;
  // }

  // recv_buf_begin = recv_buf_pos;
}

void RFM69::Transmit(const std::vector<uint8_t> &pkt)
{
}

void RFM69::Test(const std::vector<uint8_t> &data)
{
  Transmit(data);
}

double RFM69::rRSSI() const {
  return - (RT->RssiValue() / 2.0);
}

uint64_t RFM69::rSyncWord() const {
  uint64_t r = 0;
  r = (r << 8) | RT->SyncValue1();
  r = (r << 8) | RT->SyncValue2();
  r = (r << 8) | RT->SyncValue3();
  r = (r << 8) | RT->SyncValue4();
  r = (r << 8) | RT->SyncValue5();
  r = (r << 8) | RT->SyncValue6();
  r = (r << 8) | RT->SyncValue7();
  r = (r << 8) | RT->SyncValue8();
  return r;
}

void RFM69::RegisterTable::Refresh(bool frequent)
{
  RegisterTable &rt = *device.RT;
  uint8_t om = device.Read(rt._rOpMode);
  uint8_t irq1 = device.Read(rt._rIrqFlags1);
  uint8_t irq2 = device.Read(rt._rIrqFlags2);
  uint8_t rssi = device.Read(rt._rRssiValue);
  uint8_t feim = device.Read(rt._rFeiMsb);
  uint8_t feil = device.Read(rt._rFeiLsb);
  device.mtx.lock();
  if (buffer.size() != 0x72)
    buffer.resize(0x72, 0);
  buffer[rt._rOpMode.Address()] = om;
  buffer[rt._rIrqFlags1.Address()] = irq1;
  buffer[rt._rIrqFlags2.Address()] = irq2;
  buffer[rt._rRssiValue.Address()] = rssi;
  buffer[rt._rFeiMsb.Address()] = feim;
  buffer[rt._rFeiLsb.Address()] = feil;
  device.mtx.unlock();
  if (!frequent)
    for (size_t i=0x01; i < 0x71; i++)
      buffer[i] = device.Read(i);
}

void RFM69::RegisterTable::Write(std::ostream &os)
{
  json j, dev, regs;
  dev["name"] = device.Name();
  j["Device"] = dev;
  for (const auto reg : registers)
    if (reg->Address() != 0x00) {
      char tmp[3];
      snprintf(tmp, 3, "%02x", (*reg)(buffer));
      regs[reg->Name()] = tmp;
    }
  j["registers"] = regs;
  os << std::setw(2) << j << std::endl;
}

void RFM69::RegisterTable::Set(const std::string &key, const std::string &value)
{
  if (value.size() != 2)
    throw std::runtime_error(std::string("invalid value length for '" + key + "'"));
  bool found = false;
  for (const auto &r : registers)
    if (r->Name() == key && r->Address() != 0) {
      uint8_t val;
      sscanf(value.c_str(), "%02hhx", &val);
      device.Write(*r, val);
      found = true;
      break;
    }
  if (!found)
    throw std::runtime_error(std::string("invalid register '") + key + "'");
}

void RFM69::RegisterTable::Read(std::istream &is)
{
  device.SetMode(Mode::STDBY);
  device.ClearFlags();

  json j = json::parse(is);
  if (j["Device"]["Name"] != device.Name())
    throw std::runtime_error("device mismatch");
  for (const auto &e : j["registers"].items()) {
    const std::string &name = e.key();
    if (name == "OpMode")
      continue;
    if (!e.value().is_string())
      throw std::runtime_error(std::string("invalid value for '" + e.key() + "'"));
    Set(name, e.value().get<std::string>());
  }
  if (j["registers"].contains("OpMode"))
    Set("OpMode", j["registers"]["OpMode"]);
}

void RFM69::RegisterTable::Write(const Register<uint8_t, uint8_t> &reg, const uint8_t &value)
{
  device.Write(reg.Address(), value);
}
