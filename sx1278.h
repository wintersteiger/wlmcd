// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SX1278_H_
#define _SX1278_H_

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <mutex>

#include "device.h"
#include "register.h"

class SX1278 : public Device<uint8_t, uint8_t>
{
public:
  class NormalRegisterTable;
  NormalRegisterTable &Normal;

  typedef enum {
    SLEEP = 0x00, // Sleep mode
    STDBY = 0x01, // Standby mode
    FSTX  = 0x02, // Frequency synthesiser to TX frequency
    TX    = 0x03, // Transmit mode
    FSRX  = 0x04, // Frequency synthesiser to RX frequency
    RX    = 0x05, // Receive mode
  } Mode;

  class Status {
    SX1278 *c;
  public:
    Status() : c(NULL) {}
    Status(SX1278 *c);
    Status(const SX1278::Status &other) : c(other.c) {}
    ~Status() {};

    void Update();
  };

protected:
  std::mutex mtx;
  const double f_xosc, f_step;
  int NSS;

public:
  SX1278(int NSS, std::vector<Decoder*> decoders = {}, double f_xosc = 32.0*1e6);
  virtual ~SX1278();

  virtual const char* Name() const { return "SX1278"; }

  const double& F_XOSC() const { return f_xosc; }
  const double& F_STEP() const { return f_step; }

  void Reset();
  void ClearFlags();

  Mode GetMode();
  void SetMode(Mode m);

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

  uint8_t Read(const Register<uint8_t, uint8_t> &r, const Variable<uint8_t> &v) { return v(Device::Read(r)); }
  void Write(const Register<uint8_t, uint8_t> &r, const Variable<uint8_t> &v, const uint8_t &val) {
    return Device::Write(r, v.Set(Device::Read(r), val));
  }

  virtual void UpdateFrequent();
  virtual void UpdateInfrequent();

  virtual void Write(std::ostream &os);
  virtual void Read(std::istream &is);
};

#endif // _SX1278_H_
