// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RFM69_H_
#define _RFM69_H_

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <mutex>

#include "device.h"
#include "register.h"

// Supposedly a second-source Semtech SX1231H

class RFM69 : public Device<uint8_t, uint8_t>
{
public:
  class RegisterTable;
  RegisterTable &RT;

  typedef enum {
    SLEEP = 0x00, // Sleep mode (SLEEP)
    STDBY = 0x01, // Standby mode (STDBY)
    FS    = 0x02, // Frequency Synthesizer mode (FS)
    TX    = 0x03, // Transmit mode (TX)
    RX    = 0x04, // Receive mode (RX)
  } Mode;

  class Status {
    RFM69 *c;
  public:
    Status() : c(NULL) {}
    Status(RFM69 *c);
    Status(const RFM69::Status &other) : c(other.c) {}
    ~Status() {};

    void Update();
  };

protected:
  std::mutex mtx;
  int spi_channel;
  const double f_xosc, f_step;
  int NSS, INT, RST;

public:
  RFM69(int spi_channel, int NSS, int INT = -1, int RST = -1, std::vector<Decoder*> decoders = {}, double f_xosc = 32.0*1e6);
  virtual ~RFM69();

  virtual const char* Name() const { return "RFM69"; }

  const double& F_XOSC() const { return f_xosc; }
  const double& F_STEP() const { return f_step; }

  void Reset();
  void ClearFlags();

  Mode GetMode();
  void SetMode(Mode m);

  void Receive(std::vector<uint8_t> &pkt);
  void Transmit(const std::vector<uint8_t> &pkt);

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

  virtual void WriteConfig(const std::string &filename);
  virtual void ReadConfig(const std::string &filename);

  virtual void Test(const std::vector<uint8_t> &data);
};

#endif // _RFM69_H_
