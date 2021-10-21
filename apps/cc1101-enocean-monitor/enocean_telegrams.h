// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_TELEGRAMS_H_
#define _ENOCEAN_TELEGRAMS_H_

#include <vector>
#include <array>

#include "enocean.h"

namespace EnOcean
{
  class Telegram {
  public:
    Telegram() : frame(std::move(Frame())) {}
    Telegram(const Frame &f) : frame(f) {}
    virtual ~Telegram() {}

  protected:
    const Frame& frame;
  };

  class DeviceState { public: DeviceState() {}; virtual ~DeviceState() {}};

  class DeviceConfiguration {
    public:
    DeviceConfiguration() {};
    virtual ~DeviceConfiguration() {}
    virtual Frame mk_update(TXID source, TXID destination, uint8_t status) = 0;
  };

  class Telegram_4BS : public Telegram {
  public:
    Telegram_4BS() : Telegram(Frame(std::vector<uint8_t>(11, 0))) {}
    Telegram_4BS(const Frame &f) : Telegram(f) {
      if (frame.rorg() != 0xA5 || frame.size() != 11)
        throw std::runtime_error("not a 4BS telegram");
    }
    Telegram_4BS(const std::array<uint8_t, 4> &data, TXID source, uint8_t status, Frame &f);
    Telegram_4BS(const std::array<uint8_t, 4> &data, TXID source, TXID destination, uint8_t status, Frame &f);
    virtual ~Telegram_4BS() {}

    bool is_teach_in() const { return (frame.data()[3] & 0x08) == 0; }
    uint8_t rorg() const { return frame.rorg(); }
  };

  class Telegram_LEARN_4BS_3 : public Telegram_4BS {
  public:
    Telegram_LEARN_4BS_3() : Telegram_4BS() {}
    Telegram_LEARN_4BS_3(const Frame &f) : Telegram_4BS(f) {}
    Telegram_LEARN_4BS_3(
      uint8_t func, uint8_t type, MID mid, TXID source,
      Frame &f,
      bool learn_status = true, bool learn_eep_supported = true, bool learn_result = true,
      uint8_t status = 0);
    Telegram_LEARN_4BS_3(
      uint8_t func, uint8_t type, MID mid, TXID source, TXID destination,
      Frame &f,
      bool learn_status = true, bool learn_eep_supported = true, bool learn_result = true,
      uint8_t status = 0);
    virtual ~Telegram_LEARN_4BS_3() {}

    uint8_t func() const { return frame.data()[0] >> 2; }
    uint8_t type() const { return ((frame.data()[0] & 0x03) << 5) | (frame.data()[1] >> 3); }
    MID mid() const { return (frame.data()[1] & 0x07 << 8) | frame.data()[2]; }
    EEP eep() const { return (rorg() << 16) | (func() << 8) | type(); }
  };

  namespace A5_20_06 {
    class ACT2RCU : public Telegram_4BS {
    public:
      ACT2RCU() : Telegram_4BS() {}
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
      bool actuator_obstructued() const { return frame.data()[3] & 0x01; }
    };

    class RCU2ACT : public Telegram_4BS {
    public:
      RCU2ACT(const Frame &f) : Telegram_4BS(f) {}
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
      DeviceState() :
        last_frame({0xA5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}),
        last_telegram(last_frame)
      {}
      virtual ~DeviceState() {}

      void Update(const EnOcean::Frame& frame) {
        last_frame_time = time(NULL);
        last_frame = frame;
      }

      time_t last_frame_time = 0;
      Frame last_frame;
      ACT2RCU last_telegram;
    };

    class DeviceConfiguration : public EnOcean::DeviceConfiguration {
    public:
      DeviceConfiguration() {}
      virtual ~DeviceConfiguration() {}

      uint8_t setpoint = 42; // == 21 C
      uint8_t rcu_temperature = 84; // == 21 C
      bool reference_run = false;
      uint8_t communication_interval = 1;
      bool summer_mode = false;
      bool set_point_selection = true; // temperature setpoints
      bool temperature_selection = false;
      bool standby = false;

      virtual Frame mk_update(TXID source, TXID destination, uint8_t status) override;
    };
  };
}

#endif // _ENOCEAN_TELEGRAMS_H_
