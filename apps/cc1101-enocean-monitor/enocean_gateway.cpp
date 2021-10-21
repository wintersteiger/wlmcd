// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <ui.h>

#include "enocean_telegrams.h"
#include "enocean_gateway.h"

namespace EnOcean
{

  enum class TeachInMethod { RPS, LEARN_1BS, LEARN_4BS_1, LEARN_4BS_2, LEARN_4BS_3, SMART, UTE };

  static const std::map<EEP, TeachInMethod> teach_in_methods = {
    { 0x052006, TeachInMethod::LEARN_4BS_3 }
  };

  Gateway::Gateway(std::function<void(const Frame&)> &&transmit, TXID txid) :
    _transmit(transmit),
    _txid(txid),
    _learning_enabled(false)
  {
    _devices[0x0580CC3A] = { 0xA52006, 0x49,
      std::make_shared<EnOcean::A5_20_06::DeviceState>(),
      std::make_shared<EnOcean::A5_20_06::DeviceConfiguration>() };
  }

  Gateway::~Gateway() {}

  void Gateway::receive(const Frame &f)
  {
    const std::lock_guard<std::mutex> lock(mtx);

    switch (f.rorg()) {
      case 0xA5: {
          EnOcean::Telegram_4BS t(f);
          if (t.is_teach_in()) {
            if (_learning_enabled) {
              EnOcean::Telegram_LEARN_4BS_3 tti(f);
              UI::Log("Learn txid=%08x eep=%06x manufacturer=%06x", f.txid(), tti.eep(), tti.mid());
              _devices[f.txid()] = { tti.eep() };

              switch (tti.eep()) {
                case 0xA52006: {
                  // Bi-directional 4BS teach-in
                  EnOcean::Frame fo;
                  EnOcean::Telegram_LEARN_4BS_3 reply((tti.eep() & 0xFF00) >> 8, (tti.eep() & 0x00FF), tti.mid(), _txid, f.txid(), fo);
                  send(fo);
                  break;
                }
                default: break;
              }
            }
          }
          else {
            auto dit = _devices.find(f.txid());
            if (dit == _devices.end()) {
              UI::Log("unknown device: %08x", f.txid());
            }
            else {
              switch (dit->second.eep) {
                case 0xA52006: {
                  EnOcean::A5_20_06::ACT2RCU td(f);
                  UI::Log("Report from %08x: TMP=%.2f°C LO=%.2f°C ",
                                  f.txid(),
                                  (td.temperature() / 80.0) * 40.0,
                                  (td.local_offset() / 80.0) * 40.0);
                  auto s = std::dynamic_pointer_cast<A5_20_06::DeviceState>(dit->second.state);
                  s->Update(f);
                  auto cfg = std::dynamic_pointer_cast<A5_20_06::DeviceConfiguration>(dit->second.configuration);
                  auto local_offset = s->last_telegram.local_offset();
                  if (!s->last_telegram.local_offset_absolute())
                    UI::Log("relative absolute offset?");
                  if (local_offset != cfg->setpoint) {
                    // Respond to local setpoint change. We should also send the config the first time we see the device?
                    cfg->setpoint = local_offset;
                    Frame fo = cfg->mk_update(txid(), f.txid(), 0);
                    send(fo);
                  }
                  break;
                }
                default: {
                  UI::Log("Unsupported EEP %06x", dit->second);
                }
              }
            }
          }
          break;
        }
      default:
        UI::Log("unhandled rorg: %02x", f.rorg());
    }
  }

  void Gateway::send(const Frame &frame)
  {
    if (_transmit) _transmit(frame);
  }
}