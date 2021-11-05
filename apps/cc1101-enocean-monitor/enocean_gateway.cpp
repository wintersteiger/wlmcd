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
    { 0x052006, TeachInMethod::LEARN_4BS_3 }
  };

  EEP eep_from_hex(const std::string &k) {
    EEP r = 0;
    auto bs = from_hex(k);
    for (auto b : bs)
      r = r << 8 | b;
    return r;
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
      0xA52006,
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

  void Gateway::receive(const Frame &f)
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
                UI::Log("Learn txid=%08x eep=%06x manufacturer=%06x", f.txid(), tti.eep(), tti.mid());

                devices[f.txid()] = {  tti.eep(), tti.mid(),
                                       std::make_shared<A5_20_06::DeviceState>(),
                                       std::make_shared<A5_20_06::DeviceConfiguration>()};

                switch (tti.eep()) {
                  case 0xA52006: {
                    // Bi-directional 4BS teach-in
                    Frame fo;
                    Telegram_LEARN_4BS_3 reply((tti.eep() & 0xFF00) >> 8, (tti.eep() & 0x00FF), tti.mid(), config.txid, f.txid(), fo);
                    send(fo);
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
                      auto setpoint = cfg->setpoint;
                      if (!cfg->dirty)
                      {
                        auto local_offset = s->last_telegram.local_offset();
                        if (s->last_telegram.local_offset_absolute())
                          setpoint = local_offset;
                        else
                          setpoint += local_offset > 0x40 ? (local_offset | 0x80) : local_offset;
                        cfg->setpoint = setpoint;
                      }
                      Frame fo = cfg->mk_update(txid(), f.txid(), 0);
                      send(fo);
                    }
                    else
                      UI::Log("Valve position setpoint not implemented yet");
                    cfg->dirty = false;
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
          else
            UI::Log("addressed telegrams not implemented yet");
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
    if (transmit) transmit(frame);
  }
}