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

class SPIRIT1 : public Device<uint8_t, uint8_t>, SPIDev
{
public:
  class RegisterTable;
  RegisterTable *RT;

  SPIRIT1(unsigned spi_bus, unsigned spi_channel, const std::string &config_file, double f_xo = 52.0*1e6);
  virtual ~SPIRIT1();

  enum class Command {
    TX = 0x60, RX = 0x61, READY = 0x62, STANDBY = 0x63, SLEEP = 0x64, LOCKRX = 0x65, LOCKTX = 0x66,
    SABORT = 0x67, LDC_RELOAD = 0x68, SEQUENCE_UPDATE = 0x69, AES_ENC = 0x6A, AES_KEY = 0x6B,
    AES_DEC = 0x6C, AES_KEYDEC = 0x6D, SRES = 0x70, FLUSHRXFIFO = 0x71, FLUSHTXFIFO = 0x72
  };

  virtual const char* Name() const { return "SPIRIT1"; }

  const double& F_xo() const { return f_xo; }
  const double& F_clk() const { return f_clk; }

  void Reset();

  void Execute(Command cmd);
  void Receive(std::vector<uint8_t> &pkt);
  void Transmit(const std::vector<uint8_t> &pkt);

  using Device::Read;
  using Device::Write;

  virtual uint8_t Read(const uint8_t &addr);
  virtual std::vector<uint8_t> Read(const uint8_t &addr, size_t length);

  virtual void Write(const uint8_t &addr, const uint8_t &value);
  virtual void Write(const uint8_t &addr, const std::vector<uint8_t> &values);

  uint8_t Read(const Register<uint8_t, uint8_t> &r) { return Device::Read(r); }
  void Write(const Register<uint8_t, uint8_t> &r, const uint8_t &v) { Device::Write(r, v); }

  uint8_t Read(const Register<uint8_t, uint8_t> &r, const Variable<uint8_t> &v) {
    return v(Device::Read(r));
  }
  void Write(const Register<uint8_t, uint8_t> &r, const Variable<uint8_t> &v, const uint8_t &val) {
    return Device::Write(r, v.Set(Device::Read(r), val));
  }

  virtual void UpdateFrequent();
  virtual void UpdateInfrequent();

  virtual void Write(std::ostream &os);
  virtual void Read(std::istream &is);

  const uint8_t* StatusBytes() const { return &status_bytes[0]; }

  double rFrequency() const;
  double rDeviation() const;
  double rDatarate() const;
  double rFilterBandwidth() const;

  void setFrequency(double f);

protected:
  std::mutex mtx;
  double f_xo, f_clk;
  std::vector<std::pair<uint8_t, uint8_t>> address_blocks;
  uint8_t status_bytes[2];

  void FindAddressBlocks();
};

#endif // _SPIRIT1_H_
