// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <cstdint>
#include <vector>

#include "register.h"
#include "decoder.h"

class DeviceBase {
public:
  DeviceBase() : responsive(true) {}
  virtual ~DeviceBase() {}

  virtual const char* Name() const = 0;

  virtual void UpdateTimed() {}
  virtual double TimerFrequency() const { return 0; }
  virtual void UpdateFrequent() {}
  virtual void UpdateInfrequent() {}

  virtual void Write(std::ostream &os) const = 0;
  virtual void Read(std::istream &is) = 0;

  virtual void Reset() {}

  bool Responsive() const { return responsive; }

protected:
  bool responsive;
};

template <typename AT, typename VT>
class Device : public DeviceBase {
public:
  Device() {}
  Device(std::vector<Decoder*> &decoders) : decoders(decoders) {}
  virtual ~Device() {}

  using DeviceBase::Read;
  using DeviceBase::Write;

  virtual VT Read(const AT &addr) = 0;
  virtual std::vector<VT> Read(const AT &addr, size_t length) = 0;

  virtual void Write(const AT &addr, const VT &value) = 0;
  virtual void Write(const AT &addr, const std::vector<VT> &values) = 0;

  virtual VT Read(const Register<AT, VT> &r) {
    return Read(r.Address());
  }

  virtual std::vector<VT> Read(const Register<AT, VT> &r, size_t length) {
    return Read(r.Address(), length);
  }

  virtual void Write(const Register<AT, VT> &r, const VT &value) {
    Write(r.Address(), value);
  }

  virtual void Write(const Register<AT, VT> &r, const std::vector<VT> &values) {
    Write(r.Address(), values);
  }

  void Write(const Register<uint8_t, uint8_t> &r, const Variable<uint8_t> &v, const uint8_t &val) {
    return Write(r, v.Set(Device::Read(r), val));
  }

protected:
  std::vector<Decoder*> decoders;
};

#endif
