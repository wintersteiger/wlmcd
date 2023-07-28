// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <chrono>

#include "json.hpp"
using json = nlohmann::json;

#include "sleep.h"
#include "s2lp.h"
#include "s2lp_rt.h"

S2LP::S2LP(unsigned spi_bus, unsigned spi_channel, const std::string &config_file, double f_xo) :
  Device<uint8_t, uint8_t>(),
  SPIDev(spi_bus, spi_channel, 10000000),
  RT(new RegisterTable(*this)),
  f_xo(f_xo),
  tx_done(true),
  irq_mask(0xFFFFFFFF)
{
  FindAddressBlocks();
  Reset();

  if (!config_file.empty()) {
    auto is = std::ifstream(config_file);
    if (is)
      Read(is);
  }

  RT->Refresh(false);

  // if (RT->PARTNUM() != 0x03 || RT->VERSION() != 0xC1)
  //   throw std::runtime_error("unknown part no/version");

  irq_mask = RT->IRQ_MASK3() << 24 | RT->IRQ_MASK2() << 16 | RT->IRQ_MASK1() << 8 | RT->IRQ_MASK0();

  EnableIRQs();
}

S2LP::~S2LP() {}

void S2LP::FindAddressBlocks()
{
  if (address_blocks.empty())
  {
    bool used_addrs[256] = {false};
    for (const auto &r : *RT)
      if (r->Readable())
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

void S2LP::Reset()
{
  Strobe(Command::SRES);

  // if (F_xo() > 26e6) {
  //   uint8_t rv = Read(RT->_rSYNTH_CONFIG_1.Address());
  //   rv = RT->_vREFDIV.Set(rv, 1);
  //   Write(RT->_rSYNTH_CONFIG_1.Address(), rv);
  // }
}

uint8_t S2LP::Read(const uint8_t &addr)
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

std::vector<uint8_t> S2LP::Read(const uint8_t &addr, size_t length)
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

void S2LP::Write(const uint8_t &addr, const uint8_t &value)
{
  const std::lock_guard<std::mutex> lock(mtx);
  std::vector<uint8_t> b(3);
  b[0] = 0x00;
  b[1] = addr;
  b[2] = value;
  SPIDev::Transfer(b);
  status_bytes[0] = b[0];
  status_bytes[1] = b[1];
}

void S2LP::Write(const uint8_t &addr, const std::vector<uint8_t> &values)
{
  const std::lock_guard<std::mutex> lock(mtx);
  size_t n = values.size();
  std::vector<uint8_t> b(n+2);
  b[0] = 0x00;
  b[1] = addr;
  memcpy(&b[2], values.data(), n);
  SPIDev::Transfer(b);
  status_bytes[0] = b[0];
  status_bytes[1] = b[1];
}

void S2LP::Strobe(S2LP::Command cmd, size_t delay_us)
{
  const std::lock_guard<std::mutex> lock(mtx);
  std::vector<uint8_t> b = { 0x80, static_cast<uint8_t>(cmd) };
  SPIDev::Transfer(b);
}

void S2LP::StrobeFor(S2LP::Command cmd, S2LP::State st, size_t delay_us)
{
  Strobe(cmd, delay_us);

  State nst;
  size_t cnt = 0;
  responsive = true;
  do {
    nst = static_cast<State>(Read(RT->_rMC_STATE0) >> 1);

    if (delay_us)
      sleep_us(delay_us);

    if (++cnt > 50) {
      responsive = false;
      break;
    }
  } while (nst != st);
}

void S2LP::Goto(Radio::State state)
{
  switch (state) {
    case Radio::State::Idle:
      StrobeFor(Command::READY, State::READY, 10);
      break;
    case Radio::State::RX:
      if (GetState() != Radio::State::RX) {
        if ((status_bytes[0] & 0x02) == 0)
          Strobe(Command::FLUSHRXFIFO);
        Strobe(Command::RX);
      }
      break;
    case Radio::State::TX:
      Strobe(Command::TX);
      break;
    default:
      throw std::runtime_error("unhandled radio state");
  }
}

Radio::State S2LP::GetState() const
{
  switch (status_bytes[0] >> 1) {
    case 0x33: return Radio::State::RX;
    case 0x5F: return Radio::State::TX;
  }
  return Radio::State::Idle;
}

bool S2LP::RXReady()
{
  status_bytes[0] = Read(RT->_rMC_STATE1);
  status_bytes[1] = Read(RT->_rMC_STATE0);
  return (status_bytes[0] & 0x02) == 0;
}

double S2LP::RSSI()
{
  uint8_t rl = Read(RT->_rRSSI_LEVEL);
  return (rl/2.0) - 130.0;
}

double S2LP::LQI()
{
  uint8_t sqi = Read(RT->_rLINK_QUALIF1) & 0x7F;
  return 100.0 * sqi / 255.0;
}

void S2LP::Receive(std::vector<uint8_t> &pkt)
{
  DisableIRQs();
  pkt.clear();

  do
  {
    uint8_t num_available = Read(RT->_rRX_FIFO_STATUS);
    auto t = Read(0xFF, num_available);
    pkt.insert(pkt.end(), t.begin(), t.end());
  }
  while ((status_bytes[0] & 0x02) == 0 && pkt.size() < 255);

  if (RT->RX_MODE() == 0x01)
    Strobe(Command::SABORT);

  EnableIRQs();
}

uint32_t S2LP::GetIRQs()
{
  return (Read(RT->_rIRQ_STATUS3) << 24) |
         (Read(RT->_rIRQ_STATUS2) << 16) |
         (Read(RT->_rIRQ_STATUS1) << 8) |
          Read(RT->_rIRQ_STATUS0);
}

void S2LP::EnableIRQs()
{
  Write(RT->_rIRQ_MASK3, (irq_mask >> 24) & 0xFF);
  Write(RT->_rIRQ_MASK2, (irq_mask >> 16) & 0xFF);
  Write(RT->_rIRQ_MASK1, (irq_mask >> 8) & 0xFF);
  Write(RT->_rIRQ_MASK0, (irq_mask) & 0xFF);
}

void S2LP::DisableIRQs()
{
  Write(RT->_rIRQ_MASK3, 0);
  Write(RT->_rIRQ_MASK2, 0);
  Write(RT->_rIRQ_MASK1, 0);
  Write(RT->_rIRQ_MASK0, 0);
}

void S2LP::Transmit(const std::vector<uint8_t> &pkt)
{
  Strobe(Command::SABORT);
  StrobeFor(Command::READY, State::READY, 100);

  DisableIRQs();

  Strobe(Command::FLUSHTXFIFO);
  Write(0xFF, pkt);

  tx_done = false;

  EnableIRQs();

  Strobe(Command::TX);

  size_t retries = 100;
  while (!tx_done && retries-- != 0)
    sleep_us(100);

  tx_done = true;
}

void S2LP::UpdateFrequent() { RT->Refresh(true); }
void S2LP::UpdateInfrequent() { RT->Refresh(false); }

void S2LP::Write(std::ostream &os) const { RT->Write(os); }
void S2LP::Read(std::istream &is) { RT->Read(is); }

double S2LP::rFrequency() const
{
  uint32_t SYNT = (RT->SYNT_27_24() << 24) | (RT->SYNT_23_16() << 16) | (RT->SYNT_15_8() << 8) | RT->SYNT_7_0();
  double B = RT->BS() == 0 ? 4.0 : 8.0;
  double D = RT->REFDIV() == 0 ? 1.0 :  2.0;
  double Q = F_xo() / ((B*D)/2.0);
  return Q * (SYNT/pow(2, 20));
}

void S2LP::wFrequency(double f)
{
  double B = RT->BS() == 0 ? 4.0 : 8.0;
  double D = RT->REFDIV() == 0 ? 1.0 :  2.0;
  double Q = F_xo() / ((B*D)/2.0);
  uint32_t SYNT = (f * pow(2, 20)) / Q;
  Write(RT->_rSYNT0, RT->_vSYNT_7_0, SYNT);
  Write(RT->_rSYNT1, RT->_vSYNT_15_8, (SYNT >> 8) & 0xFF);
  Write(RT->_rSYNT2, RT->_vSYNT_23_16, (SYNT >> 16) & 0xFF);
  Write(RT->_rSYNT3, RT->_vSYNT_27_24, (SYNT >> 24) & 0x0F);
}

double S2LP::rDeviation() const
{
  double B = RT->BS() == 0 ? 4.0 : 8.0;
  double D = RT->REFDIV() == 0 ? 1.0 :  2.0;
  auto fdev_e = RT->FDEV_E();
  auto fdev_m = RT->FDEV_M();
  if (fdev_e == 0)
    return (f_xo / pow(2, 19)) * (D * fdev_m * B / 8.0) / (D*B);
  else
    return (f_xo / pow(2, 19)) * (D * (256 + fdev_m) * pow(2, fdev_e-1) * B / 8.0) / (D*B);
}

double S2LP::rDatarate() const
{
  uint16_t datarate_m = (RT->DATARATE_M_15_8() << 8) | RT->DATARATE_M_7_0();
  uint8_t datarate_e = RT->DATARATE_E();
  if (datarate_e == 0)
    return f_dig * datarate_m / pow(2, 32.0);
  else if (datarate_e == 15)
    return f_dig / (8.0 * datarate_m);
  else
    return f_dig * ((pow(2, 16.0) + datarate_m) * pow(2, datarate_e)) / pow(2, 33.0);
}

void S2LP::wDatarate(double f)
{
  uint8_t drate_e = log2(f * pow(2, 20) / f_dig);
  uint16_t drate_m = ((f * pow(2, 28)) / (f_dig * pow(2, drate_e))) - 256;
  if (drate_m == 0)
    drate_e++;
  Write(RT->_rMOD1, RT->_vDATARATE_M_15_8, drate_m >> 8);
  Write(RT->_rMOD1, RT->_vDATARATE_M_7_0, drate_m & 0x00FF);
  Write(RT->_rMOD0, RT->_vDATARATE_E, drate_e);
}

static float filter_bandwidths[9][10] = {
  { 800.1, 450.9, 224.7, 112.3, 56.1, 28.0, 14.0, 7.0, 3.5, 1.8 },
  { 795.1, 425.9, 212.4, 106.2, 53.0, 26.5, 13.3, 6.6, 3.3, 1.7 },
  { 768.4, 403.2, 201.1, 100.5, 50.2, 25.1, 12.6, 6.3, 3.1, 1.6 },
  { 736.8, 380.8, 190.0,  95.0, 47.4, 23.7, 11.9, 5.9, 3.0, 1.5 },
  { 705.1, 362.1, 180.7,  90.3, 45.1, 22.6, 11.3, 5.6, 2.8, 1.4 },
  { 670.9, 341.7, 170.6,  85.3, 42.6, 21.3, 10.6, 5.3, 2.7, 1.3 },
  { 642.3, 325.4, 162.4,  81.2, 40.6, 20.3, 10.1, 5.1, 2.5, 1.3 },
  { 586.7, 294.5, 147.1,  73.5, 36.7, 18.4,  9.2, 4.6, 2.3, 1.2 },
  { 541.4, 270.3, 135.0,  67.5, 33.7, 16.9,  8.4, 4.2, 2.1, 1.1 },
};

double S2LP::rFilterBandwidth() const
{
  return filter_bandwidths[RT->CHFLT_M()][RT->CHFLT_E()] * f_dig/26e6;
}

double S2LP::rRSSIThreshold() const
{
  return RT->RSSI_TH();
}

uint64_t S2LP::IRQHandler()
{
  uint32_t irqs = GetIRQs();
  if (irqs & 0x00000004)
    tx_done = true;
  if (irqs & 0x00000002) {
    Strobe(Command::FLUSHRXFIFO);
    irqs = irqs & ~0x00000002;
  }
  return irqs;
}

void S2LP::RegisterTable::Refresh(bool frequent)
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
    device.f_dig = PD_CLKDIV() ? device.f_xo : device.f_xo / 2.0;
  }
}

void S2LP::RegisterTable::Write(std::ostream &os) const
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

void S2LP::RegisterTable::Read(std::istream &is)
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

uint8_t S2LP::pqi() { return RT->PQI(); }

uint8_t S2LP::sqi() { return RT->SQI(); }

bool S2LP::cs() { return RT->CS(); }
