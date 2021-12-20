// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_GATEWAY_H_
#define _ENOCEAN_GATEWAY_H_

#include <memory>
#include <functional>
#include <mutex>
#include <set>
#include <list>
#include <thread>
#include <chrono>
#include <ostream>

#include "enocean_codec.h"
#include "enocean_frame.h"
#include "enocean_telegrams.h"

namespace EnOcean
{
  class Gateway {
  public:
    typedef struct {
      uint64_t frames = 0;
      uint64_t non_frames = 0;
      uint64_t crc_errors = 0;
    } Statistics;

    Gateway(std::function<void(const Frame&)> &&transmit,
            const std::string &data_log_file = "data.csv",
            const std::string &frame_log_file = "log.csv",
            const std::string &config_file = "enocean-gateway.json",
            const std::string &cache_file = "enocean-gateway.cache.json",
            TXID txid = 0xAABBCCDD);
    virtual ~Gateway();

    void receive(std::vector<uint8_t> &&packet, double rssi);
    void send(const Frame &frame, bool force = false);

    EEP eep() const { return config.eep; }
    TXID txid() const { return config.txid; }
    MID mid() const { return config.mid; }

    const std::map<TXID, std::shared_ptr<DeviceConfiguration>> device_configurations() { return config.devices; }
    const std::map<TXID, std::shared_ptr<DeviceState>> device_states() { return device_states_; }

    void set_learning(bool enabled) { config.learning = enabled; }
    void ping(TXID destination = 0xFFFFFFFF);

    void save(const std::string &filename, const std::string &cache_filename) const;

    const Statistics& statistics() const { return statistics_; }

    void inject(const std::shared_ptr<Frame> &frame, double rssi);

  protected:
    Statistics statistics_;
    std::shared_ptr<Decoder> decoder;
    std::function<void(const Frame&)> transmit;
    std::string config_file, cache_file;
    mutable std::mutex mtx, flog_mtx, dlog_mtx;

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
    std::map<TXID, std::shared_ptr<DeviceState>> device_states_;

    struct Telegram_SYS_EX_ERP1_CMP {
       bool operator()(const Telegram_SYS_EX_ERP1 &lhs, const Telegram_SYS_EX_ERP1 &rhs) const {
         return lhs.IDX() < rhs.IDX();
       }
    };

    std::map<TXID, std::set<Telegram_SYS_EX_ERP1, Telegram_SYS_EX_ERP1_CMP>> sys_ex_store;

  using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    void process_rx();
    void process_tx();
    void process_frame(const std::shared_ptr<Frame> &frame, double rssi, TimePoint rx_time);
    void transmit_frame(const std::shared_ptr<Frame> &frame);
    void handle_sys_ex_erp1(const Telegram_SYS_EX_ERP1 &t, double rssi);
    void handle_sys_ex(TXID sender, MID mid, uint16_t fn, const std::vector<uint8_t> &data, double rssi);
    void handle_signal(const SignalTelegram &t);

    void send_get_product_id(TXID destination);
    void send_unlock(TXID destination, uint32_t security_code);
    void send_lock(TXID destination, uint32_t security_code);
    void query_status(TXID destination);

    struct WorkItem {
      WorkItem() = default;
      WorkItem(std::vector<uint8_t> &&packet, double rssi, TimePoint rx_time) :
        packet(std::move(packet)), rssi(rssi), rx_time(rx_time)
      {}
      std::vector<uint8_t> packet; double rssi; TimePoint rx_time;
    };

    std::list<WorkItem> receive_queue;
    std::list<std::shared_ptr<Frame>> transmit_queue;
    std::thread *rx_worker, *tx_worker;
    std::vector<std::shared_ptr<EnOcean::Frame>> frames;
    std::shared_ptr<Frame> last_frame;
    TimePoint last_rx_time;

    std::ostream *frame_log_stream, *data_log_stream;
    void flog(TimePoint time, const char *label, double rssi, const std::vector<uint8_t> &frame) const;

    void send_set_code(TXID txid, DeviceConfiguration &config, uint32_t security_code);
    void send_get_device_configuration(TXID destination);
    void send_get_extended(TXID txid, DeviceConfiguration &config);
    void send_set_extended(TXID txid, DeviceConfiguration &config);

    void dlog(Gateway::TimePoint tp, TXID from, float temperature, float valve_position) const;
  };
}

#endif // _ENOCEAN_GATEWAY_H_
