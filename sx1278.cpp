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
#include "sx1278.h"
#include "sx1278_rt.h"

using json = nlohmann::json;

static std::map<int, SX1278*> gpio_to_device;

SX1278::SX1278(int NSS, std::vector<Decoder*> decoders, double f_xosc) :
  Device(decoders),
  Normal(*new NormalRegisterTable(*this)),
  f_xosc(f_xosc),
  f_step(f_xosc / pow(2, 19)),
  NSS(NSS)
{
  pinMode(NSS, OUTPUT);

  Reset();

  uint8_t mode = Read(Normal._rOpMode);
  SetMode(Mode::SLEEP);
  Write(Normal._rOpMode, (mode & 0x78)); // FSK/OOK Mode
  SetMode(Mode::STDBY);
  responsive = (Read(Normal._rOpMode) & 0x07) == 0x01;
  ClearFlags();
  SetMode(Mode::RX);

  Normal.Refresh(false);

  gpio_to_device[NSS] = this;
  // wiringPiISR(GDO[2], INT_EDGE_RISING, SX1278_RX);
}

SX1278::~SX1278() {
  delete &Normal;
}

void SX1278::Reset()
{
  digitalWrite(NSS, LOW); delayMicroseconds(1);
  digitalWrite(NSS, HIGH); delayMicroseconds(1);
  ClearFlags();
}

uint8_t SX1278::Read(const uint8_t &addr)
{
  mtx.lock();
  digitalWrite(NSS, LOW); delayMicroseconds(1);
  uint8_t res[2];
  res[0] = addr & 0x7F;
  wiringPiSPIDataRW(0, res, 2);
  digitalWrite(NSS, HIGH); delayMicroseconds(1);
  mtx.unlock();
  return res[1];
}

std::vector<uint8_t> SX1278::Read(const uint8_t &addr, size_t length)
{
  mtx.lock();
  digitalWrite(NSS, LOW); delayMicroseconds(1);
  std::vector<uint8_t> res;
  res.resize(length + 1);
  res[0] = addr & 0x7F;
  wiringPiSPIDataRW(0, res.data(), res.size());
  res.erase(res.begin());
  digitalWrite(NSS, HIGH); delayMicroseconds(1);
  mtx.unlock();
  return res;
}

void SX1278::Write(const uint8_t &addr, const uint8_t &value)
{
  mtx.lock();
  digitalWrite(NSS, LOW); delayMicroseconds(1);
  uint8_t res[2];
  res[0] = 0x80 | (addr & 0x7F);
  res[1] = value;
  wiringPiSPIDataRW(0, res, 2);
  digitalWrite(NSS, HIGH); delayMicroseconds(1);
  mtx.unlock();
}

void SX1278::Write(const uint8_t &addr, const std::vector<uint8_t> &values)
{
  mtx.lock();
  digitalWrite(NSS, LOW); delayMicroseconds(1);
  size_t n = values.size();
  uint8_t b[n+1];
  b[0] = 0x80 | (addr & 0x7F);
  memcpy(&b[1], values.data(), n);
  wiringPiSPIDataRW(0, b, n+1);
  digitalWrite(NSS, HIGH); delayMicroseconds(1);
  mtx.unlock();
}

SX1278::Mode SX1278::GetMode()
{
  uint8_t mode = Normal._vMode(Read(Normal._rOpMode));
  if (mode > 5)
    throw std::runtime_error("Invalid mode");
  return (Mode)mode;
}

void SX1278::SetMode(Mode m)
{
  uint8_t v = GetMode();
  Write(Normal._rOpMode, (v & 0xF8) | m);
}

SX1278::Status::Status(SX1278 *c) :
  c(c)
{
  Update();
}

void SX1278::Status::Update()
{
}

void SX1278::UpdateFrequent()
{
  Normal.Refresh(true);
}

void SX1278::UpdateInfrequent()
{
  Normal.Refresh(false);
}

void SX1278::Write(std::ostream &os)
{
  Normal.Write(os);
}

void SX1278::Read(std::istream &is)
{
  Normal.Read(is);
}

void SX1278::ClearFlags()
{
  uint8_t irq1 = Read(Normal._rIrqFlags1);
  uint8_t irq2 = Read(Normal._rIrqFlags2);
  Write(Normal._rIrqFlags1, irq1 | 0x0A); // RSSI, Preamble
  Write(Normal._rIrqFlags2, irq2 | 0x01); // Low battery
}

void SX1278::Receive(std::vector<uint8_t> &pkt)
{
  pkt.clear();
  pkt = Read(0x00, 64);
  uint8_t irq1 = Read(Normal._rIrqFlags1);
  Write(Normal._rIrqFlags1, irq1 | 0x0A);
}

void SX1278::Transmit(const std::vector<uint8_t> &pkt)
{
  uint8_t mode = Read(Normal._rOpMode, Normal._vMode);
  uint8_t from_mode = mode;
  Write(Normal._rOpMode, (mode & 0xF8) | Mode::FSTX);

  uint8_t full = Read(Normal._rIrqFlags2, Normal._vFifoFull);
  size_t threshold = Read(Normal._rFifoThresh, Normal._vFifoThreshold);

  size_t sent = 0;
  while (sent < pkt.size())
  {
    size_t to_send = std::min(threshold, pkt.size() - sent);
    const auto first = pkt.begin() + sent;
    const auto last = first + to_send;
    Write(Normal._rFifo.Address(), std::vector<uint8_t>(first, last));

    Write(Normal._rOpMode, (mode & 0xF8) | Mode::TX);
    mode = Read(Normal._rOpMode, Normal._vMode);
    // if (mode != Mode::TX) {
    //   char tmp[256];
    //   sprintf(tmp, "Lost TX to %u", mode);
    //   throw std::runtime_error(tmp);
    // }

    sent += to_send;
    delay(1); // give the FIFO a chance to catch up
  }

  // Write(Normal._rOpMode, (mode & 0xF8) | from_mode);
}

void SX1278::Test(const std::vector<uint8_t> &data)
{
  Transmit(data);
}

void SX1278::NormalRegisterTable::Refresh(bool frequent)
{
  NormalRegisterTable &rt = device.Normal;
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

void SX1278::NormalRegisterTable::Write(std::ostream &os)
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

void SX1278::NormalRegisterTable::Read(std::istream &is)
{
  json j = json::parse(is);
  if (j["Device"]["Name"] != device.Name())
    throw std::runtime_error("device mismatch");
  for (const auto &e : j["registers"].items()) {
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

