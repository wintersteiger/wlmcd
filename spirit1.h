// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SPIRIT1_H_
#define _SPIRIT1_H_

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <mutex>

#include "device.h"
#include "register.h"
#include "spidev.h"
#include "radio.h"

class SPIRIT1 : public Device<uint8_t, uint8_t>, public SPIDev, public Radio
{
public:
  class RegisterTable;
  RegisterTable *RT;

  SPIRIT1(unsigned spi_bus, unsigned spi_channel, const std::string &config_file, double f_xo = 50.0*1e6);
  virtual ~SPIRIT1();

  enum class Command {
    TX = 0x60, RX = 0x61, READY = 0x62, STANDBY = 0x63, SLEEP = 0x64, LOCKRX = 0x65, LOCKTX = 0x66,
    SABORT = 0x67, LDC_RELOAD = 0x68, SEQUENCE_UPDATE = 0x69, AES_ENC = 0x6A, AES_KEY = 0x6B,
    AES_DEC = 0x6C, AES_KEYDEC = 0x6D, SRES = 0x70, FLUSHRXFIFO = 0x71, FLUSHTXFIFO = 0x72
  };

  enum class State {
    STANDBY = 0x40, SLEEP = 0x36, READY = 0x03, LOCK = 0x0F, RX = 0x33, TX = 0x5F
  };

  virtual const char* Name() const override { return "SPIRIT1"; }

  const double& F_xo() const { return f_xo; }
  const double& F_clk() const { return f_clk; }

  virtual void Reset() override;

  void Strobe(Command cmd, size_t delay_us = 0);
  void StrobeFor(Command cmd, State st, size_t delay_us = 0);

  virtual void Goto(Radio::State state) override;
  virtual Radio::State GetState() const override;
  virtual void Receive(std::vector<uint8_t> &pkt) override;
  virtual void Transmit(const std::vector<uint8_t> &pkt) override;
  virtual bool RXReady() override;

  using Device::Read;
  using Device::Write;

  virtual uint8_t Read(const uint8_t &addr) override;
  virtual std::vector<uint8_t> Read(const uint8_t &addr, size_t length) override;

  virtual void Write(const uint8_t &addr, const uint8_t &value) override;
  virtual void Write(const uint8_t &addr, const std::vector<uint8_t> &values) override;

  uint8_t Read(const Register<uint8_t, uint8_t> &r) override { return Device::Read(r); }
  void Write(const Register<uint8_t, uint8_t> &r, const uint8_t &v) override { Device::Write(r, v); }

  uint8_t Read(const Register<uint8_t, uint8_t> &r, const Variable<uint8_t> &v) {
    return v(Device::Read(r));
  }
  void Write(const Register<uint8_t, uint8_t> &r, const Variable<uint8_t> &v, const uint8_t &val) {
    return Device::Write(r, v.Set(Device::Read(r), val));
  }

  virtual void UpdateFrequent() override;
  virtual void UpdateInfrequent() override;

  virtual void Write(std::ostream &os) override;
  virtual void Read(std::istream &is) override;

  const uint8_t* StatusBytes() const { return &status_bytes[0]; }

  double rFrequency() const;
  void wFrequency(double f);

  double rDeviation() const;

  double rDatarate() const;
  void wDatarate(double f);

  double rFilterBandwidth() const;
  double rRSSIThreshold() const;

  virtual double RSSI() override;
  virtual double LQI() override;

  virtual uint64_t IRQHandler() override;

protected:
  std::mutex mtx;
  double f_xo, f_clk;
  std::vector<std::pair<uint8_t, uint8_t>> address_blocks;
  uint8_t status_bytes[2];
  volatile bool tx_done;

  void FindAddressBlocks();

  uint32_t irq_mask;
  uint32_t GetIRQs();
  void EnableIRQs();
  void DisableIRQs();
};

#endif // _SPIRIT1_H_
