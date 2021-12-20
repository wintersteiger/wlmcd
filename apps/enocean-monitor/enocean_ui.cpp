// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <ctime>
#include <memory>

#include <device.h>
#include <bme280.h>

#include "enocean_gateway.h"
#include "enocean_ui.h"
#include "enocean_telegrams.h"
#include "enocean_a5_20_01.h"
#include "enocean_a5_20_06.h"

EnOceanUI::EnOceanUI(const std::unique_ptr<EnOcean::Gateway> &gateway,
                     const std::vector<std::shared_ptr<DeviceBase>> radio_devices,
                     std::shared_ptr<BME280> bme280,
                     uint64_t *num_manual) :
  UI()
{
  size_t row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  fields.push_back(new Empty(row++, col));

  if (bme280)
  {
    devices.insert(bme280);

    Add(new Label(UI::statusp, row++, col, "Gateway"));
    Add(new LField<float>(UI::statusp, row++, col, 10, "Temperature", "°C", [bme280](){ return bme280->Temperature(); }));
    Add(new LField<float>(UI::statusp, row++, col, 10, "Humidity", "%", [bme280](){ return bme280->Humidity(); }));
    Add(new LField<float>(UI::statusp, row++, col, 10, "Pressure", "hPa", [bme280](){ return bme280->Pressure() / 100.0f; }));
  }

  Add(new LField<uint64_t>(UI::statusp, row++, col, 10, "Frames", "", [&gateway](){ return gateway->statistics().frames; }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 10, "Non-frames", "", [&gateway](){ return gateway->statistics().non_frames; }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 10, "CRC errors", "", [&gateway](){ return gateway->statistics().crc_errors; }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 10, "# manual", "", [num_manual](){ return *num_manual; }));

  auto &states = gateway->device_states();
  for (auto &kv : gateway->device_configurations())
  {
    auto &txid = kv.first;
    auto &cfg = kv.second;
    auto sit = states.find(txid);
    if (sit == states.end())
      throw std::runtime_error("missing device state");
    auto &state = sit->second;

    fields.push_back(new Empty(row++, col));
    if (cfg->eep == 0xA52006)
    {
      auto s = std::dynamic_pointer_cast<EnOcean::A5_20_06::DeviceState>(state);
      auto c = std::dynamic_pointer_cast<EnOcean::A5_20_06::DeviceConfiguration>(cfg);

      char tmp[32];
      snprintf(tmp, sizeof(tmp), "%08x", txid);
      fields.push_back(new Label(UI::statusp, row++, col, tmp));

      Add(new LWarningIndicator(UI::statusp, row, col, "HVST",
        [s]() { return !s->last_telegram.harvesting(); }));
      Add(new LWarningIndicator(UI::statusp, row, col + 5, "CHRG",
        [s]() { return !s->last_telegram.charged(); }));
      Add(new LWarningIndicator(UI::statusp, row, col + 10, "OBST",
        [s]() { return s->last_telegram.actuator_obstructed(); }));

      Add(new LWarningIndicator(UI::statusp, row, col + 15, "WNDW",
        [s]() { return s->last_telegram.window_open(); }));
      Add(new LWarningIndicator(UI::statusp, row, col + 20, "RERR",
        [s]() { return s->last_telegram.radio_errors(); }));
      Add(new LWarningIndicator(UI::statusp, row++, col + 25, "WEAK",
        [s]() { return s->last_telegram.radio_weak(); }));

      Add(new LField<float>(UI::statusp, row++, col, 10, "Valve position", "%",
        [s]() { return s->valve_position(); }));
      Add(new LField<const char*>(UI::statusp, row++, col, 10, "L/O mode", "",
        [s]() { return s->last_telegram.local_offset_absolute() ? "absolute" : "relative"; }));
      Add(new LField<float>(UI::statusp, row++, col, 10, "Local offset", "°C",
        [s]() { return s->local_offset(); }));
      Add(new LField<float>(UI::statusp, row++, col, 10, "Temperature", "°C",
        [s]() { return s->temperature(); }));
      Add(new LField<const char*>(UI::statusp, row++, col, 10, "Temp sensor", "",
        [s]() { return s->last_telegram.feed_temp_sensor() ? "feed" : "ambient"; }));

      fields.push_back(new Empty(row++, col));

      Add(new LField<const char*>(UI::statusp, row++, col, 10, "Report date", "", [s]() {
        static char tmp[1024] = "";
        struct tm * lt = localtime(&s->last_frame_time);
        strftime(tmp, sizeof(tmp), "%Y%m%d", lt);
        return tmp;
      }));
      Add(new LField<const char*>(UI::statusp, row++, col, 10, "Report time", "", [s]() {
        static char tmp[1024] = "";
        struct tm * lt = localtime(&s->last_frame_time);
        strftime(tmp, sizeof(tmp), "%H:%M:%S", lt);
        return tmp;
      }));
      Add(new LField<double>(UI::statusp, row++, col, 10, "Report RSSI", "", [s]() {
        return s->last_rssi;
      }));

      fields.push_back(new Empty(row++, col));

      Add(new LEnabledIndicator(UI::statusp, row++, col, "DTY", [c](){ return c->dirty; }));
      Add(new LField<const char*>(UI::statusp, row++, col, 10, "S/P selection", "",
        [c]() {
          auto t = EnOcean::A5_20_06::SetpointSelection::TEMPERATURE;
          return c->setpoint_selection == t ? "temp" : "valve"; }
      ));
      Add(new LField<float>(UI::statusp, row++, col, 10, "S/P", "°C",
        [c]() { return (c->setpoint / 80.0f) * 40.0f; },
        [c](const char *value) {},
        [c]() { c->dirty = true; c->setpoint = std::max(c->setpoint - 1, 0); },
        [c]() { c->dirty = true; c->setpoint = std::min(c->setpoint + 1, 80); }
      ));
      Add(new LField<float>(UI::statusp, row++, col, 10, "RCU temp", "°C",
        [c]() { return (c->rcu_temperature / 160.0f) * 40.0f; },
        [c](const char *value) {},
        [c]() { c->rcu_temperature = std::max(c->rcu_temperature - 1, 0); },
        [c]() { c->rcu_temperature = std::min(c->rcu_temperature + 1, 160); }
      ));
      Add(new LField<const char*>(UI::statusp, row++, col, 10, "Com interval", "min",
        [c]() {
          static const char *values[] = { "auto", "2", "5", "10", "20", "30", "60", "120" };
          return values[c->communication_interval]; },
        [c](const char* value) {},
        [c]() { c->communication_interval = std::max(c->communication_interval - 1, 0); },
        [c]() { c->communication_interval = std::min(c->communication_interval + 1, 7); }
      ));
    }
    else if (cfg->eep == 0xA52001)
    {
      auto s = std::dynamic_pointer_cast<EnOcean::A5_20_01::DeviceState>(state);
      auto c = std::dynamic_pointer_cast<EnOcean::A5_20_01::DeviceConfiguration>(cfg);

      char tmp[32];
      snprintf(tmp, sizeof(tmp), "%08x", txid);
      fields.push_back(new Label(UI::statusp, row++, col, tmp));

      Add(new LWarningIndicator(UI::statusp, row, col, "HVST",
        [s]() { return !s->last_telegram.energy_input(); }));
      Add(new LWarningIndicator(UI::statusp, row, col + 5, "CHRG",
        [s]() { return !s->last_telegram.energy_storage(); }));
      Add(new LWarningIndicator(UI::statusp, row, col + 10, "OBST",
        [s]() { return s->last_telegram.actuator_obstructed(); }));

      Add(new LWarningIndicator(UI::statusp, row, col + 15, "WNDW",
        [s]() { return s->last_telegram.window_open(); }));
      Add(new LWarningIndicator(UI::statusp, row, col + 20, "COVR",
        [s]() { return s->last_telegram.contact_cover_open(); }));
      Add(new LWarningIndicator(UI::statusp, row++, col + 25, "FAIL",
        [s]() { return s->last_telegram.sensor_failure(); }));

      Add(new LField<float>(UI::statusp, row++, col, 10, "Valve position", "%",
        [s]() { return s->valve_position(); }));
      Add(new LField<float>(UI::statusp, row++, col, 10, "Temperature", "°C",
        [s]() { return s->temperature(); }));

      fields.push_back(new Empty(row++, col));

      Add(new LField<const char*>(UI::statusp, row++, col, 10, "Report date", "", [s]() {
        static char tmp[1024] = "";
        struct tm * lt = localtime(&s->last_frame_time);
        strftime(tmp, sizeof(tmp), "%Y%m%d", lt);
        return tmp;
      }));
      Add(new LField<const char*>(UI::statusp, row++, col, 10, "Report time", "", [s]() {
        static char tmp[1024] = "";
        struct tm * lt = localtime(&s->last_frame_time);
        strftime(tmp, sizeof(tmp), "%H:%M:%S", lt);
        return tmp;
      }));
      Add(new LField<double>(UI::statusp, row++, col, 10, "Report RSSI", "", [s]() {
        return s->last_rssi;
      }));

      fields.push_back(new Empty(row++, col));

      Add(new LField<const char*>(UI::statusp, row++, col, 10, "S/P selection", "",
        [c]() {
          auto t = EnOcean::A5_20_01::SetpointSelection::TEMPERATURE;
          return c->setpoint_selection == t ? "temp" : "valve"; }
      ));
      Add(new LField<float>(UI::statusp, row++, col, 10, "S/P", "°C",
        [c]() { return (c->setpoint / 255.0f) * 40.0f; },
        [c](const char *value) {},
        [c]() { c->dirty = true; c->setpoint = std::max(c->setpoint - 1, 0); },
        [c]() { c->dirty = true; c->setpoint = std::min(c->setpoint + 1, 255); }
      ));
      Add(new LField<float>(UI::statusp, row++, col, 10, "RCU temp", "°C",
        [c]() { return (c->rcu_temperature / 255.0f) * 40.0f; },
        [c](const char *value) {},
        [c]() { c->rcu_temperature = std::max(c->rcu_temperature - 1, 0); },
        [c]() { c->rcu_temperature = std::min(c->rcu_temperature + 1, 255); }
      ));
    }
    else
      throw std::runtime_error("unsupported EEP");
  }
}

EnOceanUI::~EnOceanUI() {}
