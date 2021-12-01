// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <fstream>

#include <ui.h>
#include <serialization.h>

#include "enocean.h"
#include "enocean_telegrams.h"
#include "enocean_gateway.h"

template <typename T>
struct hex_scalar_serializer<std::shared_ptr<T>> {
  template <typename BasicJsonType, typename U = T>
  static void to_json(BasicJsonType& j, const std::shared_ptr<T>& ptr) noexcept {
    if (ptr.get()) {
      j = *ptr;
    } else {
      j = nullptr;
    }
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
    j["num_devices"] = devices.size();
    j["devices"] = devices;
  }

  void Gateway::Configuration::from_json(const json& j)
  {
    txid = j["txid"].get<TXID>();
    mid = j["mid"].get<MID>();
    eep = j["eep"].get<EEP>();
    acting = j["acting"].get<bool>();
    learning = j["learning"].get<bool>();
    devices = j["devices"].get<std::map<TXID, std::shared_ptr<DeviceConfiguration>>>();

    // for (const auto& kv : j["devices"].items())
    // {
    //   UI::Log("Device: %s", kv.key().c_str());
    //   json jeep = kv.value()["eep"];
    //   EEP eep = jeep.get<EEP>();
    //   TXID txid;
    //   hex_scalar_serializer<json>::from_json<json, TXID>(json(kv.key()), txid);
    //   UI::Log("Device EEP: %u ID: %u", eep, txid);
    //   switch (eep) {
    //     case 0xA52006: {
    //       auto dcfg = std::make_shared<A5_20_06::DeviceConfiguration>();
    //       devices[txid] = dcfg;
    //       // devices[txid] = { eep,
    //       //                   kv.value()["mid"].get<MID>(),
    //       //                   std::make_shared<A5_20_06::DeviceState>(),
    //       //                   dcfg };
    //         break;
    //     }
    //     default:
    //       throw std::runtime_error("unsupported EEP");
    //   }
    // }
  }

  Gateway::Gateway(
    std::function<void(const Frame&)> &&transmit,
    const std::string &config_file,
    const std::string &cache_file,
    TXID txid) :
    transmit(transmit),
    config_file(config_file)
  {
    config.txid = txid;

    // if (!config_file.empty())
    // {
    //   json j;
    //   std::ifstream(config_file) >> j;
    //   config = j["controller"].get<Gateway::Configuration>();
    // }

    auto dcfg = std::make_shared<A5_20_06::DeviceConfiguration>();
    dcfg->eep = { 0xA5, 0x20, 0x06 };
    dcfg->mid = 0x0049;
    config.devices[0x0580CC3A] = dcfg;
    devices[0x0580CC3A] = {
      std::make_shared<A5_20_06::DeviceState>(),
      dcfg};

    auto dcfg2 = std::make_shared<A5_20_01::DeviceConfiguration>();
    dcfg2->eep = { 0xA5, 0x20, 0x01 };
    dcfg2->mid = 0x0049;
    config.devices[0x0583AF74] = dcfg2;
    devices[0x0583AF74] = {
      std::make_shared<A5_20_01::DeviceState>(),
      dcfg2};
  }

  Gateway::~Gateway()
  {
    if (!config_file.empty())
      save(config_file);
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
              if (config.learning) {
                Telegram_LEARN_4BS_3 tti(f);
                if (tti.learn_type_with_eep()) {
                  UI::Log("Learn txid=%08x eep=%06x manufacturer=%03x", f.txid(), tti.eep(), tti.mid());

                  auto dcfg = std::make_shared<A5_20_06::DeviceConfiguration>();
                  config.devices[f.txid()] = dcfg;
                  devices[f.txid()] = { std::make_shared<A5_20_06::DeviceState>(), dcfg };
                  switch (tti.eep()) {
                    case 0xA52001:
                    case 0xA52006: {
                      send(Telegram_LEARN_4BS_3(tti.eep().func, tti.eep().type, tti.mid(), config.txid, f.txid()));
                      break;
                    }
                    default: break;
                  }
                }
                else {
                  auto dit = devices.find(f.txid());
                  if (dit != devices.end())
                    send(Telegram_LEARN_4BS_3(tti.eep().func, tti.eep().type, tti.mid(), config.txid, f.txid()));
                  else
                    UI::Log("Learn txid=%08x unknown EEP/MID", f.txid());
                }
              }
              else
                UI::Log("Ignoring teach-in request from %08x.", f.txid());
            }
            else {
              auto dit = devices.find(f.txid());
              if (dit == devices.end()) {
                UI::Log("unknown device: %08x", f.txid());
              }
              else {
                switch (dit->second.configuration->eep) {
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
                  case 0xA52001: {
                    A5_20_01::ACT2RCU td(f);
                    auto s = std::dynamic_pointer_cast<A5_20_01::DeviceState>(dit->second.state);
                    s->Update(f);
                    auto cfg = std::dynamic_pointer_cast<A5_20_01::DeviceConfiguration>(dit->second.configuration);
                    if (cfg->setpoint_selection == A5_20_01::DeviceConfiguration::SetpointSelection::TEMPERATURE)
                      send(cfg->mk_update(txid(), f.txid(), 0));
                    else
                      UI::Log("Valve position setpoint not implemented yet");
                    break;
                  }
                  default:
                    UI::Log("Unsupported EEP %06x", dit->second.configuration->eep);
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

  void Gateway::send(const Frame &frame, bool force)
  {
    if ((config.acting || force) && transmit)
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
        send(AddressedTelegram(t, sender), true);
        break;
      }
      default:
        UI::Log("Unhandled SYS_EX function %u", fn);
    }
  }

  void Gateway::save(const std::string &filename) const
  {
    json j;
    j["controller"] = config;

    // for (const auto &s : devices)
    //   j["state"][std::to_string(s.first)] = s.second.state;

    std::ofstream of(filename);
    of << j << std::endl;
  }

  void Gateway::ping()
  {
    send(Telegram_SYS_EX_ERP1(0x01, 0x00, 0x7FF, 0x06, {}, txid(), 0x00), true);
  }
}
