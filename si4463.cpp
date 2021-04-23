// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "si4463.h"
#include "si4463_rt.h"

static void SI4463_fRX(void)
{
  printf("nIRQ\n");
  fflush(stdout);
}

SI4463::SI4463(int ceSPI, int nSEL, int nIRQ, int SDN, uint32_t XO_FREQ, std::vector<Decoder*> decoders) :
  Device(decoders),
  RTS(*new RegisterTableSet(*this)),
  ceSPI(ceSPI),
  nSEL(nSEL),
  nIRQ(nIRQ),
  SDN(SDN),
  XO_FREQ(XO_FREQ),
  combuf(32, 0)
{
  Reset();

  UpdateInfrequent();

  if (RTS.PartInfo.PART_15_8() != 0x44 || RTS.PartInfo.PART_7_0() != 0x63)
    throw std::runtime_error("Device unresponsive or part number mismatch");

  // wiringPiISR(nIRQ, INT_EDGE_FALLING, SI4463_fRX);
}

SI4463::~SI4463() {}

void SI4463::PowerUp()
{
  mtx.lock();

  cmdbuf = {
    0x00, // BOOT_OPTIONS
    0x01, // XTAL_OPTIONS
    (uint8_t)((XO_FREQ >> 24) & 0xFF), // XO_FREQ
    (uint8_t)((XO_FREQ >> 16) & 0xFF),
    (uint8_t)((XO_FREQ >> 8) & 0xFF),
    (uint8_t)(XO_FREQ & 0xFF)
  };

  RW(cmdbuf, 0);
  mtx.unlock();
}

void SI4463::Nop()
{
  mtx.lock();
  cmdbuf = { Command::NOP };
  RW(cmdbuf, 0);
  mtx.unlock();
}

std::vector<uint8_t> SI4463::PartInfo()
{
  mtx.lock();
  cmdbuf = { Command::PART_INFO };
  RW(cmdbuf, 8);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::FuncInfo()
{
  mtx.lock();
  cmdbuf = { Command::FUNC_INFO };
  RW(cmdbuf, 6);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

void SI4463::SetProperty(uint8_t group, uint8_t num_props, uint8_t start_prop, const std::vector<uint8_t> &values)
{
  if (!(0x01 <= num_props || num_props <= 0x0c))
    throw std::runtime_error("invalid number of properties");
  if (values.size() != num_props)
    throw std::runtime_error("invalid number of values");
  mtx.lock();
  cmdbuf.resize(3 + values.size());
  cmdbuf[0] = Command::SET_PROPERTY;
  cmdbuf[1] = num_props;
  cmdbuf[2] = start_prop;
  memcpy(&cmdbuf[3], values.data(), values.size());
  RW(cmdbuf, 0);
  mtx.unlock();
}

std::vector<uint8_t> SI4463::GetProperty(uint8_t group, uint8_t num_props, uint8_t start_prop)
{
  if (!(0x01 <= num_props || num_props <= 0x0c))
    throw std::runtime_error("invalid number of properties");
  mtx.lock();
  cmdbuf = { Command::GET_PROPERTY, num_props, start_prop };
  RW(cmdbuf, num_props);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::GPIOPinCfg(uint8_t gpio[4], uint8_t nirq, uint8_t sdo, uint8_t gen_config)
{
  mtx.lock();
  cmdbuf = { Command::GPIO_PIN_CFG, gpio[0], gpio[1], gpio[2], gpio[3], nirq, sdo, gen_config };
  RW(cmdbuf, 7);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::FIFOInfo(uint8_t fifo)
{
  mtx.lock();
  cmdbuf = { Command::FIFO_INFO, fifo };
  RW(cmdbuf, 2);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::GetIntStatus(uint8_t PHClrPend, uint8_t modemClrPend, uint8_t chipClrPend)
{
  mtx.lock();
  cmdbuf = { Command::GET_INT_STATUS, PHClrPend, modemClrPend, chipClrPend };
  RW(cmdbuf, 8);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::RequestDeviceState()
{
  mtx.lock();
  cmdbuf = { Command::REQUEST_DEVICE_STATE };
  RW(cmdbuf, 2);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

void SI4463::ChangeState(uint8_t nextState1)
{
  mtx.lock();
  cmdbuf = { Command::CHANGE_STATE, nextState1 };
  RW(cmdbuf, 0);
  mtx.unlock();
}

std::vector<uint8_t> SI4463::ReadCmdBuff()
{
  mtx.lock();
  cmdbuf = { Command::READ_CMD_BUFF };
  RW(cmdbuf, 16);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::FRRARead()
{
  mtx.lock();
  cmdbuf = { Command::FRR_A_READ, 0x00, 0x00, 0x00, 0x00 };
  wiringPiSPIDataRW(ceSPI, cmdbuf.data(), 5);
  std::vector<uint8_t> res = std::vector<uint8_t>(cmdbuf.begin() + 1, cmdbuf.end());
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::FRRBRead()
{
  mtx.lock();
  cmdbuf = { Command::FRR_B_READ, 0x00, 0x00, 0x00, 0x00 };
  wiringPiSPIDataRW(ceSPI, cmdbuf.data(), 5);
  std::vector<uint8_t> res = std::vector<uint8_t>(cmdbuf.begin() + 1, cmdbuf.end());
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::FRRCRead()
{
  mtx.lock();
  cmdbuf = { Command::FRR_C_READ, 0x00, 0x00, 0x00, 0x00 };
  wiringPiSPIDataRW(ceSPI, cmdbuf.data(), 5);
  std::vector<uint8_t> res = std::vector<uint8_t>(cmdbuf.begin() + 1, cmdbuf.end());
  mtx.unlock();
  return res;
}

void SI4463::IRCal(uint8_t searching_step_size, uint8_t searching_RSSI_avg, uint8_t RX_chain_setting1, uint8_t RX_chain_setting2)
{
  mtx.lock();
  cmdbuf = { Command::IRCAL, searching_step_size, searching_RSSI_avg, RX_chain_setting1, RX_chain_setting2 };
  RW(cmdbuf, 0);
  mtx.unlock();
}

std::vector<uint8_t> SI4463::IRCalManual(uint8_t IRCalAmp, uint8_t IRCalPH)
{
  mtx.lock();
  cmdbuf = { Command::IRCAL_MANUAL, IRCalAmp, IRCalPH };
  RW(cmdbuf, 2);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

void SI4463::StartTX(uint8_t channel, uint8_t condition, uint16_t tx_len, uint8_t tx_delay, uint8_t num_repeat)
{
  mtx.lock();
  cmdbuf = { Command::START_TX, channel, condition, (uint8_t)((tx_len >> 8) & 0xFF), (uint8_t)(tx_len & 0xFF), tx_delay, num_repeat };
  RW(cmdbuf, 0);
  mtx.unlock();
}

void SI4463::TXHop(uint8_t channel, uint32_t frac, uint16_t vco_cnt, uint16_t pll_settle_time)
{
  if ((frac >> 24) != 0)
    throw std::runtime_error("frace too large");
  mtx.lock();
  cmdbuf.resize(9, 0);
  cmdbuf[0] = Command::TX_HOP;
  cmdbuf[1] = channel;
  cmdbuf[2] = (frac >> 16) & 0xFF;
  cmdbuf[3] = (frac >> 8) & 0xFF;
  cmdbuf[4] = frac & 0xFF;
  cmdbuf[5] = (vco_cnt >> 8) & 0xFF;
  cmdbuf[6] = vco_cnt & 0xFF;
  cmdbuf[7] = (pll_settle_time >> 8) & 0xFF;
  cmdbuf[8] = pll_settle_time & 0xFF;
  RW(cmdbuf, 0);
  mtx.unlock();
}

void SI4463::WriteTXFIFO(const std::vector<uint8_t> &data)
{
  mtx.lock();
  cmdbuf = data;
  cmdbuf.insert(cmdbuf.begin(), Command::WRITE_TX_FIFO);
  wiringPiSPIDataRW(ceSPI, cmdbuf.data(), cmdbuf.size());
  mtx.unlock();
}

std::vector<uint8_t> SI4463::PacketInfo(uint8_t field_number, uint16_t len, uint16_t len_diff)
{
  mtx.lock();
  uint8_t len_15_8 = (len >> 8) & 0xFF;
  uint8_t len_7_0 = len & 0xFF;
  uint8_t len_diff_15_8 = (len_diff >> 8) & 0xFF;
  uint8_t len_diff_7_0 = len_diff & 0x0FF;
  cmdbuf = { Command::PACKET_INFO, field_number, len_15_8, len_7_0, len_diff_15_8, len_diff_7_0 };
  RW(cmdbuf, 2);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::GetModemStatus(uint8_t modem_clr_pend)
{
  mtx.lock();
  cmdbuf = { Command::GET_MODEM_STATUS, modem_clr_pend };
  RW(cmdbuf, 8);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

std::vector<uint8_t> SI4463::GetADCReading(uint8_t adc_en, uint8_t adc_cfg)
{
  mtx.lock();
  cmdbuf = { Command::GET_ADC_READING, adc_en, adc_cfg };
  RW(cmdbuf, 6);
  std::vector<uint8_t> res = cmdbuf;
  mtx.unlock();
  return res;
}

uint8_t SI4463::Read(const uint16_t &addr)
{
  return Read(addr, 1)[0];
}

std::vector<uint8_t> SI4463::Read(const uint16_t &addr, size_t length)
{
  return GetProperty((addr >> 8) & 0xFF, length, addr & 0xFF);
}

void SI4463::RW(std::vector<uint8_t> &values, size_t n)
{
  wiringPiSPIDataRW(ceSPI, values.data(), values.size());

  combuf.resize(n + 2);
  do {
    // We should really read only 2 bytes and only if combuf[1] == 0xFF we
    // should keep nSEL low until the data bytes are received. Not sure
    // wiringpi supports keeping nSEL low.
    combuf[0] = Command::READ_CMD_BUFF;
    combuf[1] = 0x00;
    wiringPiSPIDataRW(ceSPI, combuf.data(), n+2);
  } while (combuf[1] != 0xFF);

  values.resize(n);
  memcpy(values.data(), combuf.data() + 2, n);
}

void SI4463::Write(const uint16_t &addr, const uint8_t &value)
{
  Write(addr, std::vector<uint8_t>(1, value));
}

void SI4463::Write(const uint16_t &addr, const std::vector<uint8_t> &values)
{
  SetProperty((addr >> 8) & 0xFF, values.size(), addr & 0xFF, values);
}

void SI4463::Reset()
{
  if (SDN != -1) {
    digitalWrite(SDN, HIGH); delayMicroseconds(25); // Shutdown
    digitalWrite(SDN, LOW); delayMicroseconds(25); // POR
  }

  PowerUp();
}

void SI4463::UpdateFrequent()
{
  RTS.Refresh(true);
}

void SI4463::UpdateInfrequent()
{
  RTS.Refresh(false);
}

void SI4463::Write(std::ostream &os) {}
void SI4463::Read(std::istream &is) {}
