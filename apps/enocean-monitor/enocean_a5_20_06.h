// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_A5_20_06_
#define _ENOCEAN_A5_20_06_

#include "enocean_frame.h"
#include "enocean_telegrams.h"

namespace EnOcean
{
  namespace A5_20_06
  {
    class ACT2RCU : public Telegram_4BS  {
      public:
        ACT2RCU() : Telegram_4BS() {}
        ACT2RCU(Frame &&f) : Telegram_4BS(std::move(f)) {}
        ACT2RCU(const Frame &f) : Telegram_4BS(f) {}
        virtual ~ACT2RCU() {}

        uint8_t valve_position() const { return frame.data()[0]; }
        bool local_offset_absolute() const { return frame.data()[1] & 0x80; }
        uint8_t local_offset() const { return frame.data()[1] & 0x7F; }
        uint8_t temperature() const { return frame.data()[2]; }

        bool feed_temp_sensor() const { return frame.data()[3] & 0x80; }
        bool harvesting() const { return frame.data()[3] & 0x40; }
        bool charged() const { return frame.data()[3] & 0x20; }
        bool window_open() const { return frame.data()[3] & 0x10; }

        bool data_telegram() const { return frame.data()[3] & 0x08; }
        bool radio_errors() const { return frame.data()[3] & 0x04; }
        bool radio_weak() const { return frame.data()[3] & 0x02; }
        bool actuator_obstructed() const { return frame.data()[3] & 0x01; }

        virtual void to_json(json& j) const { j = frame; }
        virtual void from_json(const json& j) { frame = j; }
      };

      class RCU2ACT : public Telegram_4BS {
      public:
        RCU2ACT(Frame &&f) : Telegram_4BS(std::move(f)) {}
        virtual ~RCU2ACT() {}

        uint8_t valve_position() const { return frame.data()[0]; }

        uint8_t temperature() const { return frame.data()[1]; }

        bool reference_run() const { return frame.data()[2] & 0x80; }
        uint8_t radio_interval() const { return (frame.data()[2] & 0x70) >> 4; }
        bool summer_bit() const { return frame.data()[2] & 0x08; }
        bool temperature_setpoint_mode() const { return frame.data()[2] & 0x04; }
        bool feed_temperature_requested() const { return frame.data()[2] & 0x02; }
        bool standby() const { return frame.data()[2] & 0x01; }

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

        float valve_position() const { return last_telegram.valve_position(); }
        float local_offset() const { return (last_telegram.local_offset() / 80.0f) * 40.0f; }
        float temperature() const { return (last_telegram.temperature() / 80.0f) * 40.0f; }
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

        uint8_t setpoint = 42; // == 21 C
        uint8_t rcu_temperature = 84; // == 21 C
        bool reference_run = false;
        uint8_t communication_interval = 3;
        bool summer_mode = false;
        SetpointSelection setpoint_selection = SetpointSelection::TEMPERATURE;
        bool temperature_selection = false;
        bool standby = false;

        virtual Frame mk_update(TXID source, TXID destination, uint8_t status);

        virtual void to_json(json& j) const override
        {
          EnOcean::DeviceConfiguration::to_json(j);
          j["setpoint"] = setpoint;
          j["rcu_temperature"] = rcu_temperature;
          j["reference_run"] = reference_run;
          j["communication_interval"] = communication_interval;
          j["summer_mode"] = summer_mode;
          j["setpoint_selection"] = setpoint_selection;
          j["temperature_selection"] = temperature_selection;
          j["standby"] = standby;
        }

        virtual void from_json(const json& j) override
        {
          EnOcean::DeviceConfiguration::from_json(j);
          setpoint = j["setpoint"];
          rcu_temperature = j["rcu_temperature"];
          reference_run = j["reference_run"];
          communication_interval = j["communication_interval"];
          summer_mode = j["summer_mode"];
          setpoint_selection = j["setpoint_selection"].get<SetpointSelection>();
          temperature_selection = j["temperature_selection"];
          standby = j["standby"];
        }
      };
    };
}

inline void to_json(json& j, const EnOcean::A5_20_06::ACT2RCU& p) { p.to_json(j); }
inline void from_json(const json& j, EnOcean::A5_20_06::ACT2RCU& p) { return p.from_json(j); }
inline void to_json(json& j, const EnOcean::A5_20_06::DeviceState& p) { p.to_json(j); }
inline void from_json(const json& j, EnOcean::A5_20_06::DeviceState& p) { return p.from_json(j); }

#endif // _ENOCEAN_A5_20_06_
