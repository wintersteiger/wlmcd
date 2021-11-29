// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_GATEWAY_H_
#define _ENOCEAN_GATEWAY_H_

#include <memory>
#include <functional>
#include <mutex>
#include <set>

#include "enocean.h"
#include "enocean_telegrams.h"

namespace EnOcean
{
  struct Device {
    std::shared_ptr<DeviceState> state = nullptr;
    std::shared_ptr<DeviceConfiguration> configuration = nullptr;
  };

  class Gateway {
  public:
    Gateway(std::function<void(const Frame&)> &&transmit,
            const std::string &config_file = "enocean-gateway.json",
            const std::string &cache_file = "enocean-gateway.cache.json",
            TXID txid = 0xAABBCCDD);
    virtual ~Gateway();

    void receive(const Frame &frame, double rssi);
    void send(const Frame &frame);

    EEP eep() const { return config.eep; }
    TXID txid() const { return config.txid; }
    MID mid() const { return config.mid; }

    const std::map<TXID, Device> device_map() { return this->devices; }

    void set_learning(bool enabled) { config.learning = enabled; }

    void save(const std::string &filename) const;

  protected:
    std::function<void(const Frame&)> transmit;
    std::string config_file, cache_file;
    std::map<TXID, Device> devices;
    std::mutex mtx;

    class Configuration : public DeviceConfiguration {
    public:
      Configuration() {}
      virtual ~Configuration() {}

      TXID txid = 0xAABBCCDD;
      MID mid = 0x00B;
      EEP eep = { 0xA5, 0x38, 0x08 };

      bool learning = false;
      bool acting = true;

      std::map<TXID, std::shared_ptr<DeviceConfiguration>> devices;

      virtual void to_json(json& j) const override;
      virtual void from_json(const json& j) override;
    };

    Configuration config;

    struct Telegram_SYS_EX_ERP1_CMP {
       bool operator()(const Telegram_SYS_EX_ERP1 &lhs, const Telegram_SYS_EX_ERP1 &rhs) const {
         return lhs.IDX() < rhs.IDX();
       }
    };

    std::map<TXID, std::set<Telegram_SYS_EX_ERP1, Telegram_SYS_EX_ERP1_CMP>> sys_ex_store;

    void handle_sys_ex(TXID sender, MID mid, uint8_t fn, const std::vector<uint8_t> &data, double rssi);
  };
}

#endif // _ENOCEAN_GATEWAY_H_
