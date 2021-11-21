// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <fstream>

#include <ui.h>

#include <json.hpp>
using json = nlohmann::json;

#include "enocean_telegrams.h"
#include "enocean_gateway.h"

namespace EnOcean
{
  enum class TeachInMethod { RPS, LEARN_1BS, LEARN_4BS_1, LEARN_4BS_2, LEARN_4BS_3, SMART, UTE };

  static const std::map<EEP, TeachInMethod> teach_in_methods = {
    { { 0x05, 0x20, 0x06 }, TeachInMethod::LEARN_4BS_3 }
  };

  EEP eep_from_hex(const std::string &k) {
    EEP r = { 0, 0, 0 };
    auto bs = from_hex(k);
    if (bs.size() != 3)
      return r;
    return { bs[0], bs[1], bs[2] };
  }

  MID mid_from_hex(const std::string &k) {
    MID r = 0;
    auto bs = from_hex(k);
    for (auto b : bs)
      r = r << 8 | b;
    return r;
  }

  TXID txid_from_hex(const std::string &k) {
    TXID r = 0;
    auto bs = from_hex(k);
    for (auto b : bs)
      r = r << 8 | b;
    return r;
  }

  void Gateway::Configuration::to_json(json& j) const
  {
    // j["txid"] = to_hex(txid);
    // j["mid"] = to_hex(mid);
    // j["eep"] = to_hex(eep);
    // std::map<TXID, DeviceConfiguration> devices;
  }

  void Gateway::Configuration::from_json(const json& j) {}

  Gateway::Gateway(
    std::function<void(const Frame&)> &&transmit,
    const std::string &config_file,
    TXID txid) :
    transmit(transmit),
    config_file(config_file),
    learning_enabled(false)
  {
    config.txid = txid;

    // if (!config_file.empty()) {
    //   json j;
    //   std::ifstream(config_file) >> j;

    //   for (const auto& kv : j["devices"].items())
    //   {
    //     UI::Log("Device: %s", kv.key().c_str());
    //     auto eep_str = kv.value()["eep"].get<std::string>();
    //     auto eep = eep_from_hex(eep_str);
    //     UI::End();
    //     printf("eep: %s", eep_str);
    //     switch (eep) {
    //       case 0xA52006:
    //         devices[txid_from_hex(kv.key())] = {
    //           eep,
    //           mid_from_hex(kv.value()["mid"]),
    //           std::make_shared<A5_20_06::DeviceState>(),
    //           std::make_shared<A5_20_06::DeviceConfiguration>() };
    //           break;
    //         default:
    //           throw std::runtime_error("unsupported EEP");
    //     }
    //   }
    // }
    devices[0x0580CC3A] = {
      { 0xA5, 0x20, 0x06 },
      0x0049,
      std::make_shared<A5_20_06::DeviceState>(),
      std::make_shared<A5_20_06::DeviceConfiguration>() };
  }

  Gateway::~Gateway()
  {
    // std::ofstream o(config_file);
    // json j = config;
    // o << j << std::endl;
  }

  void Gateway::receive(const Frame &f, double rssi)
  {
    const std::lock_guard<std::mutex> lock(mtx);

    try
    {
      switch (f.rorg()) {
        case 0xA5: {
            Telegram_4BS t(f);
            if (t.is_teach_in()) {
              if (learning_enabled) {
                Telegram_LEARN_4BS_3 tti(f);
                UI::Log("Learn txid=%08x eep=%06x manufacturer=%03x", f.txid(), tti.eep(), tti.mid());

                devices[f.txid()] = {  tti.eep(), tti.mid(),
                                       std::make_shared<A5_20_06::DeviceState>(),
                                       std::make_shared<A5_20_06::DeviceConfiguration>()};

                switch (tti.eep()) {
                  case 0xA52006: {
                    Telegram_LEARN_4BS_3 reply(tti.eep().func, tti.eep().type, tti.mid(), config.txid, f.txid());
                    send(reply);
                    break;
                  }
                  default: break;
                }
              }
            }
            else {
              auto dit = devices.find(f.txid());
              if (dit == devices.end()) {
                UI::Log("unknown device: %08x", f.txid());
              }
              else {
                switch (dit->second.eep) {
                  case 0xA52006: {
                    A5_20_06::ACT2RCU td(f);
                    auto s = std::dynamic_pointer_cast<A5_20_06::DeviceState>(dit->second.state);
                    s->Update(f);
                    auto cfg = std::dynamic_pointer_cast<A5_20_06::DeviceConfiguration>(dit->second.configuration);
                    if (cfg->setpoint_selection == A5_20_06::DeviceConfiguration::SetpointSelection::TEMPERATURE)
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
                      send(cfg->mk_update(txid(), f.txid(), 0));
                    }
                    else
                      UI::Log("Valve position setpoint not implemented yet");
                    break;
                  }
                  default:
                    UI::Log("Unsupported EEP %06x", dit->second);
                }
              }
            }
            break;
          }
        case 0xA6: {
          AddressedTelegram t(f);
          if (t.destination() != config.txid)
            UI::Log("ignoring telegram not addressed to us");
          else {
            const std::vector<uint8_t>& bs = t;
            std::vector<uint8_t> b(bs.begin()+1, bs.end());
            UI::Log("addressed telegrams not implemented yet");
          }
          break;
        }
        case 0xC5: {
          Telegram_SYS_EX_ERP1 t(f);
          UI::Log("SYS_EX from %08x, SEQ=%u, IDX=%u", f.txid(), t.SEQ(), t.IDX());
          if (t.SEQ() == 0) {
            UI::Log("invalid SEQ, aborting");
          } else {
            auto &tset = sys_ex_store[f.txid()];
            // frame in tset-element is lost?

            if (!tset.empty() && tset.begin()->SEQ() != t.SEQ()) {
              UI::Log("SYS_EX sequence interrupted, discarding %u", tset.begin()->SEQ());
              tset.clear();
            }

            tset.insert(t);
            if (tset.begin()->IDX() == 0)
            {
              size_t idx = 0;
              size_t bytes_expected = tset.begin()->data() >> 55;
              size_t bytes_missing = bytes_expected;
              if (bytes_missing != 0) {
                for (const auto &cur : tset) {
                  if (cur.IDX() != idx)
                    break;
                  bytes_missing -= idx == 0 ? 4 : 8;
                }
              }
              if (bytes_missing == 0) {
                std::vector<uint8_t> data;
                if (bytes_expected > 0) {
                  for (const auto &cur : tset) {
                    size_t start = cur.IDX() == 0 ? 6 : 2;
                    for (size_t i=start; i < 11; i++)
                      data.push_back(cur.raw()[i]);
                  }
                }
                MID mid = ((tset.begin()->data() >> 44) & 0x7FF);
                uint8_t fn = ((tset.begin()->data() >> 32) & 0xFFF);
                handle_sys_ex(f.txid(), mid, fn, data, rssi);
                sys_ex_store.erase(f.txid());
              }
              else
                UI::Log("SYS_EX message bytes missing: %u", bytes_missing);
            }

          }
          break;
        }
        default:
          UI::Log("unhandled rorg: %02x", f.rorg());
      }
    }
    catch (std::exception &ex) {
      UI::Log("Exception: %s", ex.what());
    }
    catch (...) {
      UI::Log("Unknown exception");
    }
  }

  void Gateway::send(const Frame &frame)
  {
    if (transmit)
      transmit(frame);
  }

  void Gateway::handle_sys_ex(TXID sender, MID mid, uint8_t fn, const std::vector<uint8_t> &data, double rssi)
  {
    UI::Log("SYS_EX message: MID=%03x FN=%03x DATA=%s", mid, fn, bytes_to_hex(data).c_str());
    switch (fn) {
      case 0x06: {
        std::vector<uint8_t> payload(4);
        bool eep_v3 = eep().func > 0x3F || eep().type >	0x7F;
        payload[0] = eep_v3 ? 0 : eep().rorg;
        payload[1] = eep_v3 ? 0 : ((eep().func << 2) | (eep().type >> 5));
        payload[2] = eep_v3 ? 0 : eep().type << 3;
        payload[3] = rssi > 0.0 ? 0x00 : rssi < -255.0 ? 0xFF : -rssi;
        Telegram_SYS_EX_ERP1 t(0x01, 0x00, this->mid(), 0x606, payload, txid(), 0x00);
        send(AddressedTelegram(t, sender));
        break;
      }
      default:
        UI::Log("Unhandled SYS_EX function %u", fn);
    }
  }
}