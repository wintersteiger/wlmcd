// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _DHT22_H_
#define _DHT22_H_

#include <mutex>

#include "device.h"

class DHT22 : public Device<uint8_t, uint64_t> {
public:
  class RegisterTable;
  RegisterTable &RT;

  DHT22(int pin = 7 /* WiringPi 7 == GPIO-4 */);
  virtual ~DHT22();

  virtual const char* Name() const { return "DHT22"; }

  virtual void Reset();

  virtual void WriteConfig(const std::string &filename);
  virtual void ReadConfig(const std::string &filename);

  virtual uint64_t Read(const uint8_t &addr);
  virtual std::vector<uint64_t> Read(const uint8_t &addr, size_t length);

  virtual void Write(const uint8_t &addr, const uint64_t &value) {}
  virtual void Write(const Register<uint8_t, uint64_t> &reg, const uint64_t &value) {}
  virtual void Write(const uint8_t &addr, const std::vector<uint64_t> &values) {}

  virtual void UpdateTimed();
  virtual void UpdateFrequent();
  virtual void UpdateInfrequent();

  float Temperature() const;
  float Humidity() const;
  bool ChecksumOK() const;
  uint64_t LastReadTime() const;
  uint64_t Tries() const;
  uint64_t Reads() const;
  uint64_t BadReads() const;

protected:
  std::mutex mtx;
  int pin;

  bool checksum_ok;
  uint64_t read_time, tries, reads, bad_reads, blocked_until;

  bool block_2s;
};

#endif // _DHT22_H_
