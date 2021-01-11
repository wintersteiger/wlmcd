// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "json.hpp"
#include "cc1101.h"
#include "cc1101_rt.h"

using json = nlohmann::json;

CC1101::CC1101(int (&GDO)[3], int CSN, const std::string &config_file, std::vector<Decoder*> decoders, double f_xosc) :
  Device(decoders),
  RT(*new RegisterTable(*this)),
  GDO(GDO), CSN(CSN), f_xosc(f_xosc),
  recv_buf(new uint8_t[1024]), recv_buf_sz(1024), recv_buf_begin(0), recv_buf_pos(0)
{
  if (GDO[0] != 0) pinMode(GDO[0], INPUT);
  if (GDO[1] != 0) pinMode(GDO[1], INPUT);
  if (GDO[2] != 0) pinMode(GDO[2], INPUT);

  Reset();

  Strobe(CommandStrobe::SFTX, 100);
  Strobe(CommandStrobe::SFRX, 100);

  if (!config_file.empty())
    ReadConfig(config_file);

  StrobeFor(SIDLE, State::IDLE, 10);
  StrobeFor(SRX, State::RX, 10);

  RT.Refresh(false);
}

CC1101::~CC1101()
{
  Reset();
  StrobeFor(CC1101::CommandStrobe::SIDLE, CC1101::State::IDLE, 10);
  delete recv_buf;
}

CC1101::Status::Status(CC1101 *c) :
  c(c)
{
  Update();
}

void CC1101::Status::Update()
{
  this->PARTNUM = c->Read(c->RT._rPARTNUM);
  this->VERSION = c->Read(c->RT._rVERSION);
  this->FREQEST = c->Read(c->RT._rFREQEST);
  this->LQI = c->Read(c->RT._rLQI);
  this->RSSI = c->Read(c->RT._rRSSI);
  this->MARCSTATE = c->Read(c->RT._rMARCSTATE);
  this->WORTIME[1] = c->Read(c->RT._rWORTIME1);
  this->WORTIME[0] = c->Read(c->RT._rWORTIME0);
  this->PKTSTATUS = c->Read(c->RT._rPKTSTATUS);
  this->VCO_VC_DAC = c->Read(c->RT._rVCO_VC_DAC);
  this->TXBYTES = c->Read(c->RT._rTXBYTES);
  this->RXBYTES = c->Read(c->RT._rRXBYTES);
  this->RCCTRL_STATUS[1] = c->Read(c->RT._rRCCTRL1_STATUS);
  this->RCCTRL_STATUS[0] = c->Read(c->RT._rRCCTRL0_STATUS);
}

double CC1101::Status::rRSSI() const {
  uint8_t rssi_offset = 74; // Tbl. 31 says so...
  int16_t rssi16 = this->RSSI;
  if (rssi16 >= 128) rssi16 -= 256;
  return (rssi16/2.0) - (double)rssi_offset;
}

double CC1101::Status::rLQI() const {
  uint8_t ilqi = 128 - (this->LQI & 0x7F);
  return 100.0 * (ilqi / 128.0);
}

CC1101::State CC1101::GetState() {
  uint8_t s = Read(RT._rMARCSTATE);
  return (State)s;
}

CC1101::Status CC1101::GetStatus() {
  return Status(this);
}

CC1101::Config CC1101::GetConfig() {
  return Config(this);
}

CC1101::Config::Config(CC1101 *c) : c(c), data(47, 0)
{
  Update();
}

void CC1101::Config::Update()
{
  data = c->Read(c->RT._rIOCFG2.Address(), 47);
  patable = c->Read(c->RT._rPATABLE.Address(), 8);
}

std::string CC1101::StateName(State st) {
  switch (st) {
  case SLEEP: return "SLEEP"; break;
  case IDLE: return "IDLE"; break;
  case XOFF: return "XOFF"; break;
  case VCOON_MC: return "VCOON_MC"; break;
  case REGON_MC: return "REGON_MC"; break;
  case MANCAL: return "MANCAL"; break;
  case VCOON: return "VCOON"; break;
  case REGON: return "REGON"; break;
  case STARTCAL: return "STARTCAL"; break;
  case BWBOOST: return "BWBOOST"; break;
  case FS_LOCK: return "FS_LOCK"; break;
  case IFADCON: return "IFADCON"; break;
  case ENDCAL: return "ENDCAL"; break;
  case RX: return "RX"; break;
  case RX_END: return "RX_END"; break;
  case RX_RST: return "RX_RST"; break;
  case TXRX_SWITCH: return "TXRX_SWITCH"; break;
  case RXFIFO_OVERFLOW: return "RXFIFO O/F"; break;
  case FSTXON: return "FSTXON"; break;
  case TX: return "TX"; break;
  case TX_END: return "TX_END"; break;
  case RXTX_SWITCH: return "RXTX_SWITCH"; break;
  case TXFIFO_UNDERFLOW: return "TXFIFO U/F"; break;
  default: return "?";
  }
}

void CC1101::Config::Write()
{
  c->Write(c->RT._rIOCFG2.Address(), data);
}

double CC1101::rFrequency() const {
  uint32_t fr = (RT.FREQ2() << 16) | (RT.FREQ1() << 8) | RT.FREQ0();
  double inc = f_xosc / 65536.0;
  double f = inc * fr;
  return f;
}

double CC1101::rDataRate() const {
  uint32_t drate_m = RT.MDMCFG3();
  uint32_t drate_e = RT.MDMCFG4() & 0x0F;
  double m = (256 + drate_m) * pow(2, drate_e);
  return ((m / pow(2, 28)) * f_xosc);
}

double CC1101::rDeviation() const {
  uint8_t d = RT.DEVIATN();
  uint32_t deviatn_m = d & 0x07;
  uint32_t deviatn_e = (d & 0x70) >> 4;
  double dm = (8 + deviatn_m) * pow(2, deviatn_e);
  return dm * f_xosc / pow(2, 17);
}

double CC1101::rFilterBW() const {
  uint8_t mdmcfg4 = RT.MDMCFG4();
  uint32_t chanbw_m = (mdmcfg4 >> 4) & 0x03;
  uint32_t chanbw_e = mdmcfg4 >> 6;
  double chanbw_u = 8 * (4 + chanbw_m) * pow(2, chanbw_e);
  return f_xosc / chanbw_u;
}

double CC1101::rIFFrequency() const {
  uint8_t freq_if = RT.FSCTRL1() & 0x1F;
  return (f_xosc / pow(2, 10)) * freq_if;
}

double CC1101::rChannelSpacing() const {
  return (f_xosc / pow(2, 18)) * (256 + RT.MDMCFG0()) * pow(2, RT.MDMCFG1() & 0x03);
}

double CC1101::rEvent0() const {
  uint16_t EVENT0 = (((uint16_t)RT.WOREVT1()) << 8) | RT.WOREVT0();
  uint8_t WOR_RES = RT.WORCTRL() & 0x03;
  return (750.0/f_xosc) * EVENT0 * pow(2, 5.0 * WOR_RES);
}

double CC1101::rRXTimeout() const {
  // EVENT0·C(RX_TIME, WOR_RES) ·26/X,
  uint16_t EVENT0 = (((uint16_t)RT.WOREVT1()) << 8) | RT.WOREVT0();
  uint8_t WOR_RES = RT.WORCTRL() & 0x03;
  uint8_t RX_TIME = RT.MCSM2() & 0x07;

  double c_map[7][4] = {
    { 3.6058, 18.0288, 32.4519, 46.8750 },
    { 1.8029,  9.0144, 16.2260, 23.4375 },
    { 0.9014,  4.5072,  8.1130, 11.7188 },
    { 0.4507,  2.2536,  4.0565,  5.8594 },
    { 0.2254,  1.1268,  2.0282,  2.9297 },
    { 0.1127,  0.5634,  1.0141,  1.4648 },
    { 0.0563,  0.2817,  0.5071,  0.7324 }
  };

  if (RX_TIME == 7)
    return 1/0.0f;
  else
    return EVENT0 * c_map[RX_TIME][WOR_RES] * (26.0 / f_xosc) * 1e3;
}

CC1101::StatusByte CC1101::Strobe(CommandStrobe cs, size_t delay_us)
{
  mtx.lock();
  uint8_t csb = cs & 0xFF;
  wiringPiSPIDataRW(0, &csb, 1);
  if (delay_us)
    delayMicroseconds(delay_us);
  mtx.unlock();
  return StatusByte(csb);
}

CC1101::StatusByte CC1101::StrobeFor(CommandStrobe cs, State st, size_t delay_us)
{
  StatusByte r = Strobe(cs, delay_us);

  State nst;
  size_t cnt = 0;
  responsive = true;
  do {
    nst = (State)(Read(RT._rMARCSTATE) & 0x1F);

    if (nst == State::RXFIFO_OVERFLOW) {
      r = Strobe(CommandStrobe::SFRX, delay_us);
      r = Strobe(cs, delay_us);
    }
    else if (nst == State::TXFIFO_UNDERFLOW) {
      r = Strobe(CommandStrobe::SFTX, delay_us);
      r = Strobe(cs, delay_us);
    }

    if (delay_us)
      delayMicroseconds(delay_us);

    if (++cnt > 50) {
      responsive = false;
      break;
    }
  } while (nst != st);
  r = Strobe(CommandStrobe::SNOP);
  return r;
}

void CC1101::Reset()
{
  digitalWrite(CSN, LOW); delayMicroseconds(10);
  digitalWrite(CSN, HIGH); delayMicroseconds(40);
  Strobe(SRES, 100);
  Strobe(SCAL, 100);
}

uint8_t CC1101::Read(const uint8_t &addr)
{
  mtx.lock();
  uint8_t res[2];
  res[0] = 0x80 | (addr & 0x7F);
  wiringPiSPIDataRW(0, res, 2);
  mtx.unlock();
  return res[1];
}

std::vector<uint8_t> CC1101::Read(const uint8_t &addr, size_t length)
{
  mtx.lock();
  std::vector<uint8_t> res;
  res.resize(length + 1);
  res[0] = addr | (length == 1 ? 0x80 : 0xC0);
  wiringPiSPIDataRW(0, res.data(), res.size()) ;
  res.erase(res.begin());
  mtx.unlock();
  return res;
}

CC1101::StatusByte CC1101::WriteS(const uint8_t &addr, const uint8_t &value)
{
  mtx.lock();
  uint8_t b[2] = { addr, value };
  wiringPiSPIDataRW(0, b, 2);
  mtx.unlock();
  return b[0];
}

CC1101::StatusByte CC1101::WriteS(const uint8_t &addr, const std::vector<uint8_t> &values)
{
  mtx.lock();
  size_t n = values.size();
  uint8_t b[n+1];
  b[0] = addr | (n == 1 ? 0x00 : 0x40);
  memcpy(&b[1], values.data(), n);
  wiringPiSPIDataRW(0, b, n+1);
  mtx.unlock();
  return b[0];
}

void CC1101::Setup(const std::vector<uint8_t> &config, const std::vector<uint8_t> &patable)
{
  Write(RT._rIOCFG2.Address(), config);

  const std::vector<uint8_t> &pa_gentle = { 0x03, 0x17, 0x1D, 0x26, 0x50, 0x86, 0xCD, 0xC0 };
  const std::vector<uint8_t> &pa_max = { 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0 };

  Write(RT._rPATABLE.Address(), pa_gentle);
}

void CC1101::Receive(std::vector<uint8_t> &packet)
{
  uint8_t crc;

  // uint8_t pktctrl0_before = Read(RT._rPKTCTRL0);
  // Write(RT._rPKTCTRL0, pktctrl0_before & 0xFC);

  size_t rxbytes_last = 0, rxbytes = 1;
  while (true)
  {
    do {
      rxbytes_last = rxbytes;
      rxbytes = Read(RT._rRXBYTES);
    } while (rxbytes != rxbytes_last);

    bool overflow = (rxbytes & 0x80) != 0;
    size_t n = rxbytes & 0x7F;

    size_t m = n <= 1 ? n : n-1;

    if (n == 0)
      break;
    else if (overflow) {
      break;
    }
    else
    {
      std::vector<uint8_t> buf = Read(RT._rFIFO.Address(), m);
      for (uint8_t &bi : buf) {
        recv_buf[recv_buf_pos++] = bi;
        recv_buf_pos %= recv_buf_sz;
      }
    }

    delayMicroseconds(1000); // give the FIFO a chance to catch up
  }

  // Write(RT._rPKTCTRL0, pktctrl0_before);

  size_t pkt_sz = recv_buf_held(), i=0;
  packet.resize(pkt_sz);
  while (recv_buf_begin != recv_buf_pos) {
    packet[i++] = recv_buf[recv_buf_begin++];
    recv_buf_begin %= recv_buf_sz;
  }

  recv_buf_begin = recv_buf_pos;
}

void CC1101::Transmit(const std::vector<uint8_t> &pkt)
{
  uint8_t pktctrl0_before = Read(RT._rPKTCTRL0);
  StatusByte sb = WriteS(RT._rPKTCTRL0.Address(), (pktctrl0_before & 0xFC) | 0x02);

  if (sb.State() == StatusByte::SState::TXFIFO_UNDERFLOW) {
    Strobe(CommandStrobe::SFTX, 10);
    StrobeFor(CommandStrobe::SIDLE, State::IDLE, 10);
    StrobeFor(CommandStrobe::SFSTXON, State::FSTXON, 10);
  }

  sb = WriteS(RT._rPKTLEN.Address(), pkt.size() % 256);
  sb = Strobe(CommandStrobe::STX, 1);

  size_t sent = 0;
  while (sent < pkt.size())
  {
    size_t to_send = std::min((size_t)sb.FIFO_BYTES_AVAILABLE(), pkt.size() - sent);
    const auto first = pkt.begin() + sent;
    const auto last = first + to_send;
    sb = WriteS(RT._rFIFO.Address(), std::vector<uint8_t>(first, last));
    switch (sb.State()) {
      case StatusByte::SState::RXFIFO_OVERFLOW:
      case StatusByte::SState::TXFIFO_UNDERFLOW: return;
      case StatusByte::SState::TX: break;
      default: Strobe(CommandStrobe::STX, 10);
    }
    sent += to_send;
    if (pkt.size() - sent < 256)
      sb = WriteS(RT._rPKTCTRL0.Address(), pktctrl0_before & 0xFC);
  }

  do {
    sb = Strobe(CommandStrobe::SNOP);
    delay(10);
  } while (sb.State() == StatusByte::SState::TX);

  Write(RT._rPKTCTRL0, pktctrl0_before);
}

void CC1101::Test(const std::vector<uint8_t> &data)
{
  Transmit(data);
}

void CC1101::UpdateFrequent()
{
  RT.Refresh(true);
}

void CC1101::UpdateInfrequent()
{
  RT.Refresh(false);
}

void CC1101::WriteConfig(const std::string &filename)
{
  RT.WriteFile(filename);
}

void CC1101::ReadConfig(const std::string &filename)
{
  RT.ReadFile(filename);
}

void CC1101::RegisterTable::Refresh(bool frequent)
{
  device.mtx.lock();
  if (buffer.size() != 0xFF)
    buffer.resize(0xFF, 0);
  device.mtx.unlock();
  if (!frequent) {
    buffer = device.Read(0x00, 47);
    device.RT.PATableBuffer = device.Read(device.RT._rPATABLE, 8);
  }
  for (size_t i=0xF0; i < 0xFE; i++)
    buffer[i] = device.Read(i);
}

void CC1101::RegisterTable::WriteFile(const std::string &filename)
{
  json j, dev, regs;
  char tmp[17];
  dev["Name"] = device.Name();
  j["Device"] = dev;
  for (const auto reg : registers) {
    if (reg->Address() == _rFIFO.Address())
      continue;
    if (reg == &device.RT._rPATABLE)
    {
      uint64_t x = 0;
      for (const uint8_t &b : device.RT.PATableBuffer)
        x = (x << 8) | b;
      snprintf(tmp, sizeof(tmp), "%016" PRIx64, x);
    }
    else
      snprintf(tmp, sizeof(tmp), "%02x", (*reg)(buffer));
    regs[reg->Name()] = tmp;
  }
  j["Registers"] = regs;
  std::ofstream os(filename);
  os << std::setw(2) << j << std::endl;
}

void CC1101::RegisterTable::ReadFile(const std::string &filename)
{
  json j = json::parse(std::ifstream(filename));
  if (j["Device"]["Name"] != device.Name())
    throw std::runtime_error("device mismatch");
  for (const auto &e : j["Registers"].items()) {
    if (!e.value().is_string())
      throw std::runtime_error(std::string("invalid value for '" + e.key() + "'"));
    std::string sval = e.value().get<std::string>();
    if ((e.key() != "PATABLE" && sval.size() != 2) ||
        (e.key() == "PATABLE" && sval.size() != 16))
      throw std::runtime_error(std::string("invalid value length for '" + e.key() + "'"));
    bool found = false;
    for (const auto reg : registers)
      if (reg->Name() == e.key() && reg != &device.RT._rFIFO) {
        uint8_t val;
        if (reg == &device.RT._rPATABLE)
        {
          std::vector<uint8_t> patable(8, 0);
          for (size_t i=0; i < 8; i++) {
            sscanf(sval.c_str() + 2*i, "%02hhx", &val);
            patable[i] = val;
          }
          device.Write(device.RT._rPATABLE.Address(), patable);
        }
        else {
          sscanf(sval.c_str(), "%02hhx", &val);
          device.Write(reg->Address(), val);
        }
        found = true;
        break;
      }
    if (!found) {
      throw std::runtime_error(std::string("invalid register '") + e.key() + "'");
    }
  }
}

void CC1101::RegisterTable::Write(const Register<uint8_t, uint8_t> &reg, const uint8_t &value)
{
  device.Write(reg.Address(), value);
}
