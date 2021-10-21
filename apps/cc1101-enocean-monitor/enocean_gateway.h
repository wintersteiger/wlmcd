// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_GATEWAY_H_
#define _ENOCEAN_GATEWAY_H_

#include <memory>
#include <functional>
#include <mutex>

#include "enocean.h"

namespace EnOcean
{
  class Telegram;
  class DeviceState;
  class DeviceConfiguration;

  struct Device {
    EEP eep;
    MID mid;

    std::shared_ptr<DeviceState> state = nullptr;
    std::shared_ptr<DeviceConfiguration> configuration = nullptr;
  };

  class Gateway {
  public:
    Gateway(std::function<void(const Frame&)> &&transmit, TXID txid = 0xAABBCCDD);
    virtual ~Gateway();

    void receive(const Frame &frame);
    void send(const Frame &frame);

    EEP eep() const { return _eep; }
    TXID txid() const { return _txid; }
    MID mid() const { return _mid; }

    const std::map<TXID, Device> devices() { return _devices; }

  protected:
    std::function<void(const Frame&)> _transmit;
    std::map<TXID, Device> _devices;
    TXID _txid;
    EEP _eep = 0xA53808;
    MID _mid = 0x7FF;
    std::mutex mtx;
    bool _learning_enabled;
  };
}

#endif // _ENOCEAN_GATEWAY_H_
