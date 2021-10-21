// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <ctime>
#include <memory>

#include <device.h>

#include "enocean_gateway.h"
#include "enocean_ui.h"
#include "enocean_telegrams.h"

EnOceanUI::EnOceanUI(const std::unique_ptr<EnOcean::Gateway>& gateway,
                     const std::vector<std::shared_ptr<DeviceBase>> radio_devices) :
  UI()
{
  for (auto d: radio_devices)
    this->devices.insert(d.get());

  size_t row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  fields.push_back(new Empty(row++, col));

  for (auto &kv : gateway->devices())
  {
    auto &txid = kv.first;
    auto &dev = kv.second;

    if (dev.eep != 0xA52006)
      throw std::runtime_error("unsupported EEP");

    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%08x", txid);
    fields.push_back(new Label(UI::statusp, row++, col, tmp));

    auto s = std::dynamic_pointer_cast<EnOcean::A5_20_06::DeviceState>(dev.state);
    auto c = std::dynamic_pointer_cast<EnOcean::A5_20_06::DeviceConfiguration>(dev.configuration);

    Add(new LField<float>(UI::statusp, row++, col, 10, "Valve position", "%",
      [s]() { return (s->last_telegram.valve_position() / 80.0f) * 40.0f; },
      [s](const char *value) {}
    ));
    Add(new LField<const char*>(UI::statusp, row++, col, 10, "L/O mode", "",
      [s]() { return s->last_telegram.local_offset_absolute() ? "absolute" : "relative"; }));
    Add(new LField<float>(UI::statusp, row++, col, 10, "Local offset", "°C",
      [s]() { return (s->last_telegram.local_offset() / 80.0f) * 40.0f; }));
    Add(new LField<float>(UI::statusp, row++, col, 10, "Temperature", "°C",
      [s]() { return (s->last_telegram.temperature() / 80.0f) * 40.0f; }));
    Add(new LField<const char*>(UI::statusp, row++, col, 10, "Temp sensor", "",
      [s]() { return s->last_telegram.feed_temp_sensor() ? "feed" : "ambient"; }));
    Add(new LField<bool>(UI::statusp, row++, col, 10, "Energy input", "",
      [s]() { return s->last_telegram.harvesting(); }));
    Add(new LField<const char*>(UI::statusp, row++, col, 10, "Energy storage", "",
      [s]() { return s->last_telegram.charged() ? "sufficient" : "low"; }));
    Add(new LField<const char*>(UI::statusp, row++, col, 10, "Window", "",
      [s]() { return s->last_telegram.window_open() ? "open" : "closed"; }));
    Add(new LField<bool>(UI::statusp, row++, col, 10, "Radio errors", "",
      [s]() { return s->last_telegram.radio_errors(); }));
    Add(new LField<bool>(UI::statusp, row++, col, 10, "Weak signal", "",
      [s]() { return s->last_telegram.radio_weak(); }));
    Add(new LField<bool>(UI::statusp, row++, col, 10, "Obstructed", "",
      [s]() { return s->last_telegram.actuator_obstructued(); }));

    fields.push_back(new Empty(row++, col));

    Add(new LField<const char*>(UI::statusp, row++, col, 10, "Report date", "", [s]() {
      static char tmp[1024] = "";
      struct tm * lt = localtime(&s->last_frame_time);
      strftime(tmp, sizeof(tmp), "%Y-%m-%d", lt);
      return tmp;
    }));
    Add(new LField<const char*>(UI::statusp, row++, col, 10, "Report time", "", [s]() {
      static char tmp[1024] = "";
      struct tm * lt = localtime(&s->last_frame_time);
      strftime(tmp, sizeof(tmp), "%H:%M:%S", lt);
      return tmp;
    }));

    fields.push_back(new Empty(row++, col));

    Add(new LField<float>(UI::statusp, row++, col, 10, "Set point", "°C",
      [c]() { return (c->setpoint / 80.0f) * 40.0f; },
      [c](const char *value) {},
      [c]() { c->setpoint = std::max(c->setpoint - 1, 0); },
      [c]() { c->setpoint = std::min(c->setpoint + 1, 100); }
    ));
    Add(new LField<float>(UI::statusp, row++, col, 10, "RCU temp", "°C",
      [c]() { return (c->rcu_temperature / 160.0f) * 40.0f; },
      [c](const char *value) {},
      [c]() { c->rcu_temperature = std::max(c->rcu_temperature - 1, 0); },
      [c]() { c->rcu_temperature = std::min(c->rcu_temperature + 1, 160); }
    ));
    Add(new LField<uint8_t>(UI::statusp, row++, col, 10, "Com interval", "min",
      [c]() {
        static uint8_t values[] = {0, 2, 5, 10, 20, 30, 60, 120};
        return values[c->communication_interval]; },
      [c](const char* value) {},
      [c]() { c->communication_interval = std::max(c->communication_interval - 1, 0); },
      [c]() { c->communication_interval = std::min(c->communication_interval + 1, 7); }
    ));

  }
}

EnOceanUI::~EnOceanUI() {}
