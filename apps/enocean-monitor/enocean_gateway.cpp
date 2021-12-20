// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>

#include <ui.h>
#include <serialization.h>
#include <sleep.h>

#include "enocean_frame.h"
#include "enocean_gateway.h"
#include "enocean_telegrams.h"
#include "enocean_a5_20_01.h"
#include "enocean_a5_20_06.h"

template <typename T>
struct hex_scalar_serializer<std::shared_ptr<T>> {
  template <typename BasicJsonType, typename U = T>
  static void to_json(BasicJsonType& j, const std::shared_ptr<T>& ptr) noexcept {
    if (ptr)
      j = *ptr;
    else
      j = nullptr;
  }

  template <typename BasicJsonType, typename U = T>
  static void from_json(const BasicJsonType& j, std::shared_ptr<T>& ptr)
  {
    T tmp;
    nlohmann::from_json(j, tmp);
    ptr = std::make_shared<T>(std::move(tmp));
  }
};

namespace EnOcean
{
  enum class TeachInMethod { RPS, LEARN_1BS, LEARN_4BS_1, LEARN_4BS_2, LEARN_4BS_3, SMART, UTE };

  static const std::map<EEP, TeachInMethod> teach_in_methods = {
    { { 0x05, 0x20, 0x06 }, TeachInMethod::LEARN_4BS_3 }
  };

  void Gateway::Configuration::to_json(json& j) const
  {
    j["txid"] = txid;
    j["mid"] = mid;
    j["eep"] = eep;
    j["acting"] = acting;
    j["learning"] = learning;
    j["devices"] = devices;
  }

  void Gateway::Configuration::from_json(const json& j)
  {
    txid = j["txid"].get<TXID>();
    mid = j["mid"].get<MID>();
    eep = j["eep"].get<EEP>();
    acting = j["acting"].get<bool>();
    learning = j["learning"].get<bool>();

    auto jd = j["devices"];
    assert(jd.is_array());
    for (const auto& kv : jd)
    {
      assert(kv.is_array());
      auto key = kv[0];
      auto value = kv[1];
      TXID txid;
      hex_scalar_serializer<json>::from_json<json, TXID>(json(key), txid);
      EEP eep = value["eep"].get<EEP>();
      devices[txid] = mk_device_configuration(eep, value);
    }
  }

  Gateway::Gateway(
    std::function<void(const Frame&)> &&transmit,
    const std::string &data_log_file,
    const std::string &frame_log_file,
    const std::string &config_file,
    const std::string &cache_file,
    TXID txid) :
    transmit(transmit),
    config_file(config_file),
    cache_file(cache_file),
    rx_worker(nullptr),
    tx_worker(nullptr),
    last_rx_time(),
    frame_log_stream(nullptr),
    data_log_stream(nullptr)
  {
    decoder = std::make_shared<EnOcean::Decoder>();
    config.txid = txid;

    if (!config_file.empty())
    {
      json j;
      std::ifstream(config_file) >> j;
      config = j["controller"].get<Gateway::Configuration>();
    }

    for (const auto &kv : config.devices)
      device_states_[kv.first] = mk_device_state(kv.second->eep);

    if (!cache_file.empty())
    {
      json jcache;
      std::ifstream cf(cache_file);
      if (cf.good()) {
        cf >> jcache;
        assert(jcache.is_array());
        for (const auto& kv : jcache)
        {
          assert(kv.is_array());
          auto key = kv[0];
          auto value = kv[1];
          TXID txid;
          hex_scalar_serializer<json>::from_json<json, TXID>(json(key), txid);
          auto dit = config.devices.find(txid);
          if (dit != config.devices.end())
            device_states_[txid] = mk_device_state(dit->second->eep, value);
        }
      }
    }

    if (!frame_log_file.empty())
      frame_log_stream = new std::ofstream(frame_log_file, std::ios_base::app);
    if(!data_log_file.empty())
      data_log_stream = new std::ofstream(data_log_file, std::ios_base::app);
  }

  Gateway::~Gateway()
  {
    // save(config_file, cache_file);
    save("", cache_file);
    {
      const std::lock_guard<std::mutex> lock(mtx);
      transmit_queue.clear();
      receive_queue.clear();
    }
    delete frame_log_stream;
    delete data_log_stream;
  }

  void Gateway::receive(std::vector<uint8_t> &&packet, double rssi)
  {
    const std::lock_guard<std::mutex> lock(mtx);

    receive_queue.emplace_back(std::move(packet), rssi, std::chrono::high_resolution_clock::now());
    if (!rx_worker) {
      rx_worker = new std::thread([this](){
        do { process_rx(); }
        while (receive_queue.size() > 0);
        rx_worker = nullptr;
      });
      rx_worker->detach();
    }
  }

  void Gateway::process_rx()
  {
    WorkItem work_item;
    char lbuf[1024];
    char *p = &lbuf[0];

    {
      const std::lock_guard<std::mutex> lock(mtx);
      if (receive_queue.empty())
        return;
      work_item = std::move(receive_queue.front());
      receive_queue.pop_front();
    }

    p += snprintf(p, sizeof(lbuf)-(p-&lbuf[0]), "RX rssi=%4.0fdBm N=%d", work_item.rssi, work_item.packet.size());

    std::vector<std::shared_ptr<Frame>> frames = decoder->get_frames(work_item.packet);

    if (frames.empty())
    {
      statistics_.non_frames++;
    }
    else
    {
      statistics_.frames += frames.size();
      p += sprintf(p, " Frames:");
      for (auto& f : frames) {
        auto d = f->describe();
        p += snprintf(p, sizeof(lbuf)-(p-&lbuf[0]), " %s", d.c_str());
      }

      UI::Log(lbuf);
    }

    for (auto &&f : frames) {
      flog(work_item.rx_time, "RX", work_item.rssi, *f);
      if (!f->crc_ok())
        statistics_.crc_errors++;
      else
        process_frame(std::move(f), work_item.rssi, work_item.rx_time);
    }
  }

  void Gateway::flog(Gateway::TimePoint tp, const char *label, double rssi, const std::vector<uint8_t> &frame) const
  {
    const std::lock_guard<std::mutex> lock(flog_mtx);

    if (frame_log_stream) {
      static char time_buf[32];
      std::time_t tt = std::chrono::high_resolution_clock::to_time_t(tp);
      std::tm tm = *std::localtime(&tt);

      std::chrono::high_resolution_clock::duration total = tp.time_since_epoch();
      std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(total);
      auto us = std::chrono::duration_cast<std::chrono::microseconds>(total - seconds).count();

      char *p = &time_buf[0];
      p += strftime(p, sizeof(time_buf) - (p-time_buf), "%Y-%m-%d %H:%M:%S", &tm);
      p += snprintf(p, sizeof(time_buf) - (p-time_buf), ".%06llu", us);

      *frame_log_stream
        << time_buf << ","
        << label << ","
        << std::fixed << std::setw(7) << std::setprecision(2) << std::setfill(' ') << rssi << ","
        << to_hex(frame) << std::endl;
    }
  }

  void Gateway::dlog(Gateway::TimePoint tp, TXID from, float temperature, float valve_position) const
  {
    const std::lock_guard<std::mutex> lock(dlog_mtx);

    if (data_log_stream) {
      static char time_buf[32];
      std::time_t tt = std::chrono::high_resolution_clock::to_time_t(tp);
      std::tm tm = *std::localtime(&tt);

      std::chrono::high_resolution_clock::duration total = tp.time_since_epoch();
      std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(total);
      auto us = std::chrono::duration_cast<std::chrono::microseconds>(total - seconds).count();

      char *p = &time_buf[0];
      p += strftime(p, sizeof(time_buf) - (p-time_buf), "%Y-%m-%d %H:%M:%S", &tm);
      p += snprintf(p, sizeof(time_buf) - (p-time_buf), ".%06llu", us);

      *data_log_stream << time_buf << ","
        << std::hex << from << std::dec << ","
        << std::fixed << std::setw(6) << std::setprecision(2) << std::setfill(' ') << temperature << ","
        << std::fixed << std::setw(6) << std::setprecision(2) << std::setfill(' ') << valve_position
        << std::endl;
    }
  }

  void Gateway::process_frame(const std::shared_ptr<Frame> &f, double rssi, Gateway::TimePoint rx_time)
  {
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(rx_time - last_rx_time);
    if (diff_ms <= std::chrono::milliseconds(50) && *f == *last_frame) {
      last_rx_time = rx_time;
      return;
    }

    try
    {
      switch (f->rorg()) {
        case 0xA5: {
            Telegram_4BS t(*f);
            if (t.is_teach_in()) {
              if (config.learning) {
                Telegram_LEARN_4BS_3 tti(*f);
                if (tti.learn_type_with_eep()) {
                  UI::Log("Learn txid=%08x eep=%06x manufacturer=%03x", f->txid(), (uint32_t)tti.eep(), tti.mid());
                  auto dit = config.devices.find(f->txid());
                  if (dit == config.devices.end()) {
                    config.devices[f->txid()] = mk_device_configuration(tti.eep());
                    device_states_[f->txid()] = mk_device_state(tti.eep());
                  }
                  send(Telegram_LEARN_4BS_3(tti.eep().func, tti.eep().type, tti.mid(), config.txid, f->txid()));
                }
                else {
                  auto dit = config.devices.find(f->txid());
                  if (dit != config.devices.end())
                    send(Telegram_LEARN_4BS_3(dit->second->eep, dit->second->mid, config.txid, f->txid()));
                  else
                    UI::Log("Learn txid=%08x unknown EEP/MID", f->txid());
                }
              }
              else
                UI::Log("Ignoring teach-in request from %08x.", f->txid());
            }
            else {
              auto dit = config.devices.find(f->txid());
              if (dit == config.devices.end()) {
                UI::Log("unknown device: %08x", f->txid());
              }
              else {
                switch (dit->second->eep) {
                  case 0xA52006: {
                    A5_20_06::ACT2RCU td(*f);
                    auto s = std::dynamic_pointer_cast<A5_20_06::DeviceState>(device_states_[dit->first]);
                    s->Update(*f, rssi);
                    auto cfg = std::dynamic_pointer_cast<A5_20_06::DeviceConfiguration>(dit->second);
                    if (cfg->setpoint_selection == A5_20_06::SetpointSelection::TEMPERATURE)
                    {
                      uint8_t their_setpoint = 42;
                      auto local_offset = s->last_telegram.local_offset();
                      if (s->last_telegram.local_offset_absolute())
                        their_setpoint = local_offset;
                      else
                        their_setpoint = cfg->setpoint + (local_offset > 0x40 ? (local_offset | 0x80) : local_offset);

                      if (!cfg->dirty)
                        cfg->setpoint = their_setpoint;
                      else if (their_setpoint == cfg->setpoint)
                        cfg->dirty = false;
                      send(cfg->mk_update(txid(), f->txid(), 0));
                      dlog(rx_time, f->txid(), s->temperature(), s->valve_position());
                    }
                    else
                      UI::Log("Valve position setpoint not implemented yet");
                    break;
                  }
                  case 0xA52001: {
                    A5_20_01::ACT2RCU td(*f);
                    auto s = std::dynamic_pointer_cast<A5_20_01::DeviceState>(device_states_[dit->first]);
                    s->Update(*f, rssi);
                    auto cfg = std::dynamic_pointer_cast<A5_20_01::DeviceConfiguration>(dit->second);
                    // send_get_extended(f->txid(), *cfg);
                    if (cfg->setpoint_selection == A5_20_01::SetpointSelection::TEMPERATURE)
                      send(cfg->mk_update(txid(), f->txid(), 0));
                    else
                      UI::Log("Valve position setpoint not implemented yet");
                    dlog(rx_time, f->txid(), s->temperature(), s->valve_position());
                    break;
                  }
                  default:
                    UI::Log("Unsupported EEP %06x", (uint32_t)dit->second->eep);
                }
              }
            }
            break;
          }
        case 0xA6: {
          AddressedTelegram t(*f);
          if (t.destination() != config.txid)
            UI::Log("ignoring telegram not addressed to us");
          else {
            const std::vector<uint8_t> &bs = t;
            std::vector<uint8_t> b(bs.size()-5);
            for (size_t i = 0; i < bs.size()-5; i++)
              b[i] = bs[i+1];
            for (size_t i = 0; i < 6; i++)
              b[b.size() - 6 + i] = bs[bs.size() - 6 + i];
            auto f2 = std::make_shared<Frame>(std::move(b));
            process_frame(std::move(f2), rssi, rx_time);
          }
          break;
        }
        case 0xC5:
          handle_sys_ex_erp1(Telegram_SYS_EX_ERP1(std::move(*f)), rssi);
          break;
        case 0xD0:
          handle_signal(SignalTelegram(std::move(*f)));
          break;
        default:
          UI::Log("unhandled rorg: %02x", f->rorg());
      }
    }
    catch (std::exception &ex) {
      UI::Log("Exception: %s", ex.what());
    }
    catch (...) {
      UI::Log("Unknown exception");
    }

    last_frame = f;
    last_rx_time = rx_time;
  }

  void Gateway::send(const Frame &frame, bool force)
  {
    const std::lock_guard<std::mutex> lock(mtx);
    transmit_queue.push_back(std::make_shared<Frame>(frame));

    if ((config.acting || force) && transmit) {
      if (!tx_worker) {
        tx_worker = new std::thread([this](){
          do { process_tx(); }
          while (transmit_queue.size() > 0);
          tx_worker = nullptr;
        });
        tx_worker->detach();
      }
    }
  }

  void Gateway::process_tx()
  {
    std::shared_ptr<Frame> frame;

    {
      const std::lock_guard<std::mutex> lock(mtx);
      if (transmit_queue.empty())
        return;
      frame = std::move(transmit_queue.front());
      transmit_queue.pop_front();
    }

    sleep_ms(100);

    UI::Log("TX %s", to_hex(*frame).c_str());
    flog(std::chrono::high_resolution_clock::now(), "TX", 0.0, *frame);
    transmit(*frame);
  }

  void Gateway::send_get_product_id(TXID destination)
  {
    Telegram_SYS_EX_ERP1 t(0x02, 0x00, 0x7FF, RMCC::GET_PRODUCT_ID, {}, txid(), 0x0F);
    if (destination != 0xFFFFFFFF)
      send(AddressedTelegram(t, destination));
    else
      send(t);
  }

  void Gateway::send_unlock(TXID destination, uint32_t security_code)
  {
    std::vector<uint8_t> sc = {
      (uint8_t)((security_code >> 24) & 0xFF),
      (uint8_t)((security_code >> 16) & 0xFF),
      (uint8_t)((security_code >> 8) & 0xFF),
      (uint8_t)(security_code& 0xFF)
    };
    send(AddressedTelegram(Telegram_SYS_EX_ERP1(0x01, 0x00, 0x7FF, RMCC::UNLOCK, sc, txid(), 0x0F), destination));
  }

  void Gateway::send_lock(TXID destination, uint32_t security_code)
  {
    std::vector<uint8_t> sc = {
      (uint8_t)((security_code >> 24) & 0xFF),
      (uint8_t)((security_code >> 16) & 0xFF),
      (uint8_t)((security_code >> 8) & 0xFF),
      (uint8_t)(security_code& 0xFF)
    };
    send(AddressedTelegram(Telegram_SYS_EX_ERP1(0x02, 0x00, 0x7FF, RMCC::LOCK, sc, txid(), 0x0F), destination));
  }

  void Gateway::query_status(TXID destination)
  {
    send(AddressedTelegram(Telegram_SYS_EX_ERP1(0x01, 0x00, 0x7FF, RMCC::QUERY_STATUS, {}, txid(), 0x0F), destination));
  }

  void Gateway::handle_sys_ex_erp1(const Telegram_SYS_EX_ERP1 &t, double rssi)
  {
    // UI::Log("SYS_EX from %08x, SEQ=%u, IDX=%u", t.txid(), t.SEQ(), t.IDX());
    if (t.SEQ() == 0) {
      UI::Log("Ignoring SYS_EX with invalid SEQ");
    } else {
      auto &tset = sys_ex_store[t.txid()];

      if (!tset.empty() && tset.begin()->SEQ() != t.SEQ()) {
        UI::Log("SYS_EX sequence interrupted, discarding %u", tset.begin()->SEQ());
        tset.clear();
      }

      bool seen = false;
      for (const auto &cur : tset) {
        if (cur.IDX() == t.IDX()) {
          seen = true;
          break;
        }
      }
      if (!seen)
        tset.insert(t);

      if (tset.begin()->IDX() == 0)
      {
        size_t bytes_expected = tset.begin()->data() >> 55;
        size_t bytes_missing = bytes_expected;
        if (bytes_missing != 0) {
          for (const auto &cur : tset)
            bytes_missing -= std::min(bytes_missing, (size_t)(cur.IDX() == 0 ? 4 : 8));
        }
        if (bytes_missing == 0) {
          std::vector<uint8_t> data;
          if (bytes_expected > 0) {
            data.reserve(bytes_expected);
            for (const auto &cur : tset) {
              for (size_t i=0; i < cur.data_size() && data.size() < bytes_expected; i++)
                data.push_back(cur.raw_data()[i]);
            }
          }
          MID mid = ((tset.begin()->data() >> 44) & 0x7FF);
          uint16_t fn = ((tset.begin()->data() >> 32) & 0xFFF);
          handle_sys_ex(t.txid(), mid, fn, data, rssi);
          sys_ex_store.erase(t.txid());
        }
        // else
        //   UI::Log("SYS_EX message bytes missing: %u/%u", bytes_missing, bytes_expected);
      }
    }
  }

  void Gateway::handle_sys_ex(TXID sender, MID mid, uint16_t fn, const std::vector<uint8_t> &data, double rssi)
  {
    switch (static_cast<RMCC>(fn)) {
      case RMCC::QUERY_ID: {
        UI::Log("SYS_EX Query ID from %08x, replying", sender);
        std::vector<uint8_t> payload(4);
        payload[0] = eep().rorg;
        payload[1] = ((eep().func << 2) | (eep().type >> 5));
        payload[2] = eep().type << 3;
        payload[3] = 0;
        Telegram_SYS_EX_ERP1 t(0x01, 0x00, this->mid(), RMCC::QUERY_ID_ANSWER_EXTENDED, payload, txid(), 0x0F);
        send(AddressedTelegram(t, sender), true);
        break;
      }
      case RMCC::PING_COMMAND: {
        UI::Log("SYS_EX Ping from %08x, replying", sender);
        std::vector<uint8_t> payload(4);
        bool eep_v3 = eep().func > 0x3F || eep().type >	0x7F;
        payload[0] = eep_v3 ? 0 : eep().rorg;
        payload[1] = eep_v3 ? 0 : ((eep().func << 2) | (eep().type >> 5));
        payload[2] = eep_v3 ? 0 : eep().type << 3;
        payload[3] = rssi > 0.0 ? 0x00 : rssi < -255.0 ? 0xFF : -rssi;
        Telegram_SYS_EX_ERP1 t(0x01, 0x00, this->mid(), RMCC::PING_ANSWER, payload, txid(), 0x0F);
        send(AddressedTelegram(t, sender), true);
        break;
      }
      case RMCC::GET_PRODUCT_ID: {
        UI::Log("SYS_EX Get product id from %08x, replying", sender);
        // selection criteria (data.size() > 0) not implemented.
        std::vector<uint8_t> payload = { (uint8_t)(this->mid() >> 8), (uint8_t)(this->mid() & 0xFF), 1, 3, 3, 7};
        Telegram_SYS_EX_ERP1 t1(0x01, 0x00, 0x7FF, RMCC::GET_PRODUCT_ID_RESPONSE, payload, txid(), 0x0F);
        send(AddressedTelegram(t1, sender), true);
        Telegram_SYS_EX_ERP1 t2(0x01, 0x01, 0x7FF, RMCC::GET_PRODUCT_ID_RESPONSE, payload, txid(), 0x0F);
        send(AddressedTelegram(t2, sender), true);
        break;
      }
      case RMCC::QUERY_STATUS_ANSWER: {
        uint8_t code_set = data[0] >> 7;
        uint8_t last_seq = data[0] & 0x03;
        uint16_t last_fn = ((data[1] & 0x0F) << 8) | data[2];
        uint8_t last_rc = data[3];
        UI::Log("SYS_EX Status report from %08x: code_set=%u last_seq=%u last_fn=%03x last_rc=%u", sender, code_set, last_seq, last_fn, last_rc);
        break;
      }
      case RMCC::LOCK:
        UI::Log("SYS_EX Lock from %08x with code %08x", sender, to_hex(data).c_str());
        break;
      case RMCC::UNLOCK:
        UI::Log("SYS_EX Unlock from %08x with code %08x", sender, to_hex(data).c_str());
        break;
      case RMCC::REMOTE_COMMISSIONING_ACKNOWLEDGE:
        UI::Log("SYS_EX ACK from %08x", sender);
        break;
      case RMCC::GET_PRODUCT_ID_RESPONSE: {
        MID mid = data[0] << 8 | data[1];
        uint32_t pid = data[2] << 24 | data[3] << 16 | data[4] << 8 | data[5];
        UI::Log("SYS_EX Product ID response from %08x: manufacturer=%04x product=%08x", sender, mid, pid);
        break;
      }
      case RMCC::GET_DEVICE_CONFIGURATION_RESPONSE: {
        std::map<size_t, std::vector<uint8_t>> values;
        if (data.size() >= 4) {
          for (size_t i = 0; i < data.size() - 3; ) {
            size_t index = data[i] << 8 | data[i+1];
            size_t len = data[i+2];
            auto &value = values[index];
            value.clear();
            i += 3;
            while (i < data.size() && value.size() < len)
              value.push_back(data[i++]);
          }
        }
        char tmp[2048];
        char *p = &tmp[0];
        for (auto &kv : values) {
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), " %x=", kv.first);
          for (auto &b : kv.second)
            p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), "%02x", b);
        }
        UI::Log("SYS_EX Configuration of %08x:%s", sender, tmp);
        auto sit = device_states_.find(sender);
        if (sit != device_states_.end()) {
          for (auto &inx_val : values)
            sit->second->extended[inx_val.first] = inx_val.second;
        }
        break;
      }
      case RMCC::GET_LINK_TABLE_METADATA_RESPONSE: {
        auto remote_out_supported = data[0] & 0x80 != 0x00;
        auto remote_in_supported = data[0] & 0x40 != 0x00;
        auto link_out_supported = data[0] & 0x20 != 0x00;
        auto link_in_supported = data[0] & 0x10 != 0x00;
        auto cur_out_len = data[1];
        auto max_out_len = data[2];
        auto cur_in_len = data[3];
        auto max_in_len = data[4];
        UI::Log("SYS_EX link metadata from %08x: %u/%u out, %u/%u in [%u/%u/%u/%u]", sender, cur_out_len, max_out_len, cur_in_len, max_in_len,
          remote_out_supported, remote_in_supported, link_out_supported, link_in_supported);
        break;
      }
      case RMCC::GET_LINK_TABLE_RESPONSE: {
        char tmp[2048];
        char *p = &tmp[0];
        for (size_t i=1; i < data.size() - 8 && data.size() > 9; ) {
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), " %x: ", data[i++]);
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), "%02x", data[i++]);
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), "%02x", data[i++]);
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), "%02x", data[i++]);
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), "%02x", data[i++]);
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), " [");
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), "%02X-", data[i++]);
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), "%02X-", data[i++]);
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), "%02X", data[i++]);
          p += snprintf(p, sizeof(tmp)-(p-&tmp[0]), ", %02x]", data[i++]);
        }
        UI::Log("SYS_EX %s link table from %08x:%s", data[0] & 0x80 != 0 ? "outbound" : "inbound", sender, tmp);
        break;
      }
      default:
        UI::Log("Unhandled SYS_EX: MID=%03x FN=%03x DATA=%s", mid, fn, to_hex(data).c_str());
    }
  }

  void Gateway::save(const std::string &filename, const std::string &cache_filename) const
  {
    if (!filename.empty()) {
      json j;
      j["controller"] = config;
      std::ofstream of(filename);
      of << j << std::endl;
    }

    if (!cache_file.empty()) {
      json j = device_states_;
      std::ofstream of(cache_file);
      of << j << std::endl;
    }
  }

  void Gateway::send_get_device_configuration(TXID destination)
  {
    std::vector<uint8_t> data = { 0, 0, 0, 1, 2 };
    Telegram_SYS_EX_ERP1 t(0x02, 0x00, 0x7FF, RMCC::GET_DEVICE_CONFIGURATION_QUERY, data, txid(), 0x0F);
    send(AddressedTelegram(t, destination));
    Telegram_SYS_EX_ERP1 t2(0x02, 0x01, 0x7FF, RMCC::GET_DEVICE_CONFIGURATION_QUERY, data, txid(), 0x0F);
    send(AddressedTelegram(t2, destination));
  }

  void Gateway::send_get_extended(TXID txid, DeviceConfiguration &config)
  {
    send_unlock(txid, config.security_code);
    // send_set_extended(txid, config);
    for (auto inx_sz : config.extended) {
      std::vector<uint8_t> data(5);
      data[0] = data[2] = inx_sz.first >> 8;
      data[1] = data[3] = inx_sz.first;
      data[4] = inx_sz.second;
      Telegram_SYS_EX_ERP1 t(0x02, 0x00, 0x7FF, RMCC::GET_DEVICE_CONFIGURATION_QUERY, data, this->txid(), 0x0F);
      send(AddressedTelegram(t, txid));
      Telegram_SYS_EX_ERP1 t2(0x02, 0x01, 0x7FF, RMCC::GET_DEVICE_CONFIGURATION_QUERY, data, this->txid(), 0x0F);
      send(AddressedTelegram(t2, txid));
    }
    {
      Telegram_SYS_EX_ERP1 t(0x02, 0x00, 0x7FF, RMCC::GET_LINK_TABLE_METADATA_QUERY, {}, this->txid(), 0x0F);
      send(AddressedTelegram(t, txid));
    }
    {
      std::vector<uint8_t> data = { 0x80, 0, 0 };
      Telegram_SYS_EX_ERP1 t(0x02, 0x00, 0x7FF, RMCC::GET_LINK_TABLE_QUERY, data, this->txid(), 0x0F);
      send(AddressedTelegram(t, txid));
    }
    send_lock(txid, config.security_code);
  }

  void Gateway::send_set_extended(TXID txid, DeviceConfiguration &config)
  {
    std::vector<uint8_t> data = { 0, 0x0E, 1, 0x08 };
    Telegram_SYS_EX_ERP1 t(0x02, 0x00, 0x7FF, RMCC::SET_DEVICE_CONFIGURATION_QUERY, data, this->txid(), 0x0F);
    send(AddressedTelegram(t, txid));
  }

  void Gateway::send_set_code(TXID txid, DeviceConfiguration &config, uint32_t security_code)
  {
    send_unlock(txid, config.security_code);
    std::vector<uint8_t> sc = {
      (uint8_t)((security_code >> 24) & 0xFF),
      (uint8_t)((security_code >> 16) & 0xFF),
      (uint8_t)((security_code >> 8) & 0xFF),
      (uint8_t)(security_code& 0xFF)
    };
    send(AddressedTelegram(Telegram_SYS_EX_ERP1(0x02, 0x00, 0x7FF, RMCC::SET_CODE, sc, this->txid(), 0x0F), txid));
    send_lock(txid, security_code);
    config.security_code = security_code;
  }

  void Gateway::handle_signal(const SignalTelegram &t)
  {
    switch (t.message_index()) {
      case SignalTelegram::MessageIndex::RXWindowOpen: {
        // SignalTelegram r(SignalTelegram::MessageIndex::TriggerStatusMessageOfDevice, { 0x01 }, this->txid(), 0x0F);
        // send(AddressedTelegram(r, t.txid()));

        TXID txid = t.txid();
        auto dit = config.devices.find(txid);
        if (dit != config.devices.end()) {
          // send_get_extended(t.txid(), *dit->second);

          // send_unlock(txid, dit->second->security_code);
          // send_set_extended(txid, *dit->second);
          // send_lock(txid, dit->second->security_code);
        }

        break;
      }
      default:
        UI::Log("Signal message index %u unhandled", static_cast<uint8_t>(t.message_index()));
    }
  }

  void Gateway::ping(TXID destination)
  {
    if (destination == 0xFFFFFFFF)
      send(Telegram_SYS_EX_ERP1(0x01, 0x00, 0x7FF, RMCC::PING_COMMAND, {}, txid(), 0x0F), true);
    else
      send(AddressedTelegram(Telegram_SYS_EX_ERP1(0x01, 0x00, 0x7FF, RMCC::PING_COMMAND, {}, txid(), 0x0F), true), destination);
  }

  void Gateway::inject(const std::shared_ptr<Frame> &frame, double rssi)
  {
    if (frame->crc_ok())
      process_frame(frame, rssi, std::chrono::high_resolution_clock::now());
    else
      UI::Error("invalid crc");
  }
}
