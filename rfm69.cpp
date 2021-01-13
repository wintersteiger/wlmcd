// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <cmath>

#include <vector>
#include <fstream>
#include <iomanip>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "json.hpp"
#include "rfm69.h"
#include "rfm69_rt.h"

using json = nlohmann::json;

static std::map<int, RFM69*> gpio_to_device;

RFM69::RFM69(int spi_channel, int NSS, int INT, int RST, std::vector<Decoder*> decoders, double f_xosc) :
  Device(decoders),
  RT(*new RegisterTable(*this)),
  f_xosc(f_xosc),
  f_step(f_xosc / pow(2, 19)),
  spi_channel(spi_channel),
  NSS(NSS),
  INT(INT),
  RST(RST)
{
  Reset();
  return;

  SetMode(Mode::SLEEP);
  SetMode(Mode::STDBY);
  ClearFlags();
  SetMode(Mode::RX);

  RT.Refresh(false);

  gpio_to_device[NSS] = this;

  // if (INT != -1)
  //   wiringPiISR(GDO[2], INT_EDGE_RISING, RFM69_RX);
}

RFM69::~RFM69() {
  delete &RT;
}

void RFM69::Reset()
{
  if (RST != -1) {
    digitalWrite(RST, HIGH);
    delayMicroseconds(110);
    digitalWrite(RST, LOW);
    delayMicroseconds(5500);
  }

  digitalWrite(NSS, LOW);
  delayMicroseconds(1);
  digitalWrite(NSS, HIGH);
  delayMicroseconds(1);

  ClearFlags();
}

uint8_t RFM69::Read(const uint8_t &addr)
{
  mtx.lock();
  digitalWrite(NSS, LOW); delayMicroseconds(1);
  uint8_t res[2];
  res[0] = addr & 0x7F;
  wiringPiSPIDataRW(spi_channel, res, 2);
  digitalWrite(NSS, HIGH); delayMicroseconds(1);
  mtx.unlock();
  return res[1];
}

std::vector<uint8_t> RFM69::Read(const uint8_t &addr, size_t length)
{
  mtx.lock();
  digitalWrite(NSS, LOW); delayMicroseconds(1);
  std::vector<uint8_t> res;
  res.resize(length + 1);
  res[0] = addr & 0x7F;
  wiringPiSPIDataRW(spi_channel, res.data(), res.size());
  res.erase(res.begin());
  digitalWrite(NSS, HIGH); delayMicroseconds(1);
  mtx.unlock();
  return res;
}

void RFM69::Write(const uint8_t &addr, const uint8_t &value)
{
  mtx.lock();
  digitalWrite(NSS, LOW); delayMicroseconds(1);
  uint8_t res[2];
  res[0] = 0x80 | (addr & 0x7F);;
  res[1] = value;
  wiringPiSPIDataRW(spi_channel, res, 2);
  digitalWrite(NSS, HIGH); delayMicroseconds(1);
  mtx.unlock();
}

void RFM69::Write(const uint8_t &addr, const std::vector<uint8_t> &values)
{
  mtx.lock();
  digitalWrite(NSS, LOW); delayMicroseconds(1);
  size_t n = values.size();
  uint8_t b[n+1];
  b[0] = 0x80 | (addr & 0x7F);;
  memcpy(&b[1], values.data(), n);
  wiringPiSPIDataRW(spi_channel, b, n+1);
  digitalWrite(NSS, HIGH); delayMicroseconds(1);
  mtx.unlock();
}

RFM69::Mode RFM69::GetMode()
{
  uint8_t r = RT._vMode(Read(RT._rOpMode));
  return (Mode)r;
}

void RFM69::SetMode(Mode m)
{
  uint8_t nv = RT._vMode.Set(Read(RT._rOpMode), m);
  Write(RT._rOpMode, nv);

  size_t limit = 50;
  do {
    if (GetMode() == m)
      return;
    delayMicroseconds(10);
  } while(--limit);

  // Device did not react after limit tries.
  responsive = false;
}

RFM69::Status::Status(RFM69 *c) :
  c(c)
{
  Update();
}

void RFM69::Status::Update()
{
}

void RFM69::UpdateFrequent()
{
  RT.Refresh(true);
}

void RFM69::UpdateInfrequent()
{
  RT.Refresh(false);
}

void RFM69::WriteConfig(const std::string &filename)
{
  RT.WriteFile(filename);
}

void RFM69::ReadConfig(const std::string &filename)
{
  RT.ReadFile(filename);
}

void RFM69::ClearFlags()
{
  uint8_t irq1 = Read(RT._rIrqFlags1);
  uint8_t irq2 = Read(RT._rIrqFlags2);
  Write(RT._rIrqFlags1, irq1 | 0x09); // RSSI, SyncAddressMatch
  Write(RT._rIrqFlags2, irq2 | 0x10); // FifoOverrun
}

void RFM69::Receive(std::vector<uint8_t> &pkt)
{
  pkt.clear();
  pkt = Read(0x00, 64);
  uint8_t irq1 = Read(RT._rIrqFlags1);
  Write(RT._rIrqFlags1, irq1 | 0x0A);
}

void RFM69::Transmit(const std::vector<uint8_t> &pkt)
{
  // uint8_t mode = Read(RT._rOpMode, RT._vMode);
  // uint8_t from_mode = mode;
  // Write(RT._rOpMode, (mode & 0xF8) | Mode::FSTX);

  // uint8_t full = Read(RT._rIrqFlags2, RT._vFifoFull);
  // size_t threshold = Read(RT._rFifoThresh, RT._vFifoThreshold);

  // size_t sent = 0;
  // while (sent < pkt.size())
  // {
  //   size_t to_send = std::min(threshold, pkt.size() - sent);
  //   const auto first = pkt.begin() + sent;
  //   const auto last = first + to_send;
  //   Write(RT._rFifo.Address(), std::vector<uint8_t>(first, last));

  //   Write(RT._rOpMode, (mode & 0xF8) | Mode::TX);
  //   mode = Read(RT._rOpMode, RT._vMode);
  //   // if (mode != Mode::TX) {
  //   //   char tmp[256];
  //   //   sprintf(tmp, "Lost TX to %u", mode);
  //   //   throw std::runtime_error(tmp);
  //   // }

  //   sent += to_send;
  //   delay(1); // give the FIFO a chance to catch up
  // }

  // Write(RT._rOpMode, (mode & 0xF8) | from_mode);
}

void RFM69::Test(const std::vector<uint8_t> &data)
{
  Transmit(data);
}

void RFM69::RegisterTable::Refresh(bool frequent)
{
  RegisterTable &rt = device.RT;
  uint8_t om = device.Read(rt._rOpMode.Address());
  uint8_t irq1 = device.Read(rt._rIrqFlags1.Address());
  uint8_t irq2 = device.Read(rt._rIrqFlags2.Address());
  uint8_t rssi = device.Read(rt._rRssiValue.Address());
  uint8_t feim = device.Read(rt._rFeiMsb.Address());
  uint8_t feil = device.Read(rt._rFeiLsb.Address());
  device.mtx.lock();
  if (buffer.size() != 0x71)
    buffer.resize(0x71, 0);
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

void RFM69::RegisterTable::WriteFile(const std::string &filename)
{
  json j, dev, regs;
  dev["Name"] = device.Name();
  j["Device"] = dev;
  for (const auto reg : registers)
    if (reg->Address() != 0x00) {
      char tmp[3];
      snprintf(tmp, 3, "%02x", (*reg)(buffer));
      regs[reg->Name()] = tmp;
    }
  j["Registers"] = regs;
  std::ofstream os(filename);
  os << std::setw(2) << j << std::endl;
}

void RFM69::RegisterTable::ReadFile(const std::string &filename)
{
  json j = json::parse(std::ifstream(filename));
  if (j["Device"]["Name"] != device.Name())
    throw std::runtime_error("device mismatch");
  for (const auto &e : j["Registers"].items()) {
    if (!e.value().is_string())
      throw std::runtime_error(std::string("invalid value for '" + e.key() + "'"));
    std::string sval = e.value().get<std::string>();
    if (sval.size() != 2)
      throw std::runtime_error(std::string("invalid value length for '" + e.key() + "'"));
    bool found = false;
    for (const auto &r : registers)
      if (r->Name() == e.key() && r->Address() != 0) {
        uint8_t val;
        sscanf(sval.c_str(), "%02hhx", &val);
        device.Write(r->Address(), val);
        found = true;
        break;
      }
    if (!found) {
      throw std::runtime_error(std::string("invalid register '") + e.key() + "'");
    }
  }
}

void RFM69::RegisterTable::Write(const Register<uint8_t, uint8_t> &reg, const uint8_t &value)
{
  device.Write(reg.Address(), value);
}
