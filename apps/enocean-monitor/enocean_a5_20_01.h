// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_A5_20_01_
#define _ENOCEAN_A5_20_01_

#include "enocean_telegrams.h"

namespace EnOcean
{
  namespace A5_20_01
  {
    class ACT2RCU : public Telegram_4BS {
    public:
      ACT2RCU() : Telegram_4BS() {}
      ACT2RCU(Frame &&f) : Telegram_4BS(std::move(f)) {}
      ACT2RCU(const Frame &f) : Telegram_4BS(f) {}
      virtual ~ACT2RCU() {}

      uint8_t current_value() const { return frame.data()[0]; } // 0-100 = 0-100 %

      bool service_on() const { return frame.data()[1] & 0x80; }
      bool energy_input() const { return frame.data()[1] & 0x40; }
      bool energy_storage() const { return frame.data()[1] & 0x20; }
      bool battery_capacity() const { return frame.data()[1] & 0x10; }
      bool contact_cover_open() const { return frame.data()[1] & 0x08; }
      bool sensor_failure() const { return frame.data()[1] & 0x04; }
      bool window_open() const { return frame.data()[1] & 0x02; }
      bool actuator_obstructed() const { return frame.data()[1] & 0x01; }

      uint8_t temperature() const { return frame.data()[2]; } // 0-255 = 0-40 C

      virtual void to_json(json& j) const { j = frame; }
      virtual void from_json(const json& j) { frame = j; }
    };

    class RCU2ACT : public Telegram_4BS {
    public:
      RCU2ACT(Frame &&f) : Telegram_4BS(std::move(f)) {}
      virtual ~RCU2ACT() {}

      uint8_t valve_position() const { return frame.data()[0]; }

      uint8_t temperature() const { return frame.data()[1]; }

      bool run_init_sequence() const { return frame.data()[2] & 0x80; }
      bool lift_set() const { return frame.data()[2] & 0x40; }
      bool valve_open() const { return frame.data()[2] & 0x20; }
      bool valve_closed() const { return frame.data()[2] & 0x10; }
      bool summer_bit() const { return frame.data()[2] & 0x08; }
      bool temperature_setpoint_mode() const { return frame.data()[2] & 0x04; }
      bool setpoint_inverse() const { return frame.data()[2] & 0x02; }
      bool select_function() const { return frame.data()[2] & 0x01; }

      bool data_telegram() const { return frame.data()[3] & 0x08; }
    };

    class DeviceState : public EnOcean::DeviceState {
    public:
      DeviceState() {}
      virtual ~DeviceState() {}

      void Update(const EnOcean::Frame& frame, double rssi) {
        last_frame_time = time(NULL);
        last_telegram = ACT2RCU(frame);
        last_rssi = rssi;
      }

      time_t last_frame_time = 0;
      ACT2RCU last_telegram;
      double last_rssi = 0.0;

      virtual void to_json(json& j) const override {
        j["last_frame_time"] = last_frame_time;
        j["last_telegram"] = last_telegram;
        j["last_rssi"] = last_rssi;
      }

      virtual void from_json(const json& j) override {
        auto ftm_it = j.find("last_frame_time"); if (ftm_it != j.end()) last_frame_time = ftm_it->get<time_t>();
        auto tgm_it = j.find("last_telegram"); if (tgm_it != j.end()) last_telegram = tgm_it->get<ACT2RCU>();
        auto rssi_it = j.find("last_rssi"); if (rssi_it != j.end()) last_rssi = rssi_it->get<double>();
      }

      float valve_position() const { return last_telegram.current_value(); }
      float temperature() const { return (last_telegram.temperature() / 255.0f) * 40.0f; }
    };

    enum SetpointSelection { VALVE_POSITION = 0, TEMPERATURE = 1 };

    NLOHMANN_JSON_SERIALIZE_ENUM( SetpointSelection, {
      {SetpointSelection::VALVE_POSITION, "valve_position"},
      {SetpointSelection::TEMPERATURE, "temperature"},
    });

    class DeviceConfiguration : public EnOcean::DeviceConfiguration {
    public:
      DeviceConfiguration() : EnOcean::DeviceConfiguration() {}
      virtual ~DeviceConfiguration() {}

      uint8_t setpoint = 134; // == 21 C
      uint8_t rcu_temperature = 134; // == 21 C
      bool reference_run = false;
      bool lift_set = false;
      bool valve_open = false;
      bool valve_closed = false;
      bool summer_mode = false;
      SetpointSelection setpoint_selection = SetpointSelection::TEMPERATURE;
      bool setpoint_inverse = false;
      bool select_function = false;

      virtual Frame mk_update(TXID source, TXID destination, uint8_t status);

      virtual void to_json(json& j) const override
      {
        EnOcean::DeviceConfiguration::to_json(j);
        j["setpoint"] = setpoint;
        j["rcu_temperature"] = rcu_temperature;
        j["reference_run"] = reference_run;
        j["lift_set"] = lift_set;
        j["valve_open"] = valve_open;
        j["valve_closed"] = valve_closed;
        j["summer_mode"] = summer_mode;
        j["setpoint_selection"] = setpoint_selection;
        j["setpoint_inverse"] = setpoint_inverse;
        j["select_function"] = select_function;
      }

      virtual void from_json(const json& j) override
      {
        EnOcean::DeviceConfiguration::from_json(j);
        setpoint = j["setpoint"];
        rcu_temperature = j["rcu_temperature"];
        reference_run = j["reference_run"];
        lift_set = j["lift_set"];
        valve_open = j["valve_open"];
        valve_closed = j["valve_closed"];
        summer_mode = j["summer_mode"];
        setpoint_selection = j["setpoint_selection"].get<SetpointSelection>();
        setpoint_inverse = j["setpoint_inverse"];
        select_function = j["select_function"];
      }
    };
  };
}

inline void to_json(json& j, const EnOcean::A5_20_01::ACT2RCU& p) { p.to_json(j); }
inline void from_json(const json& j, EnOcean::A5_20_01::ACT2RCU& p) { return p.from_json(j); }
inline void to_json(json& j, const EnOcean::A5_20_01::DeviceState& p) { p.to_json(j); }
inline void from_json(const json& j, EnOcean::A5_20_01::DeviceState& p) { return p.from_json(j); }

#endif // _ENOCEAN_A5_20_01_
