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

  SPIRIT1(unsigned spi_bus, unsigned spi_channel, double f_xo = 50.0*1e6);
  virtual ~SPIRIT1();

  virtual const char* Name() const { return "SPIRIT1"; }

  const double& F_xo() const { return f_xo; }

  void Reset();

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

protected:
  std::mutex mtx;
  const double f_xo;
  std::vector<std::pair<uint8_t, uint8_t>> address_blocks;
  uint8_t status_bytes[2];

  void FindAddressBlocks();
};

#endif // _SPIRIT1_H_
