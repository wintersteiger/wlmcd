// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_TELEGRAMS_H_
#define _ENOCEAN_TELEGRAMS_H_

#include <vector>
#include <array>

#include "json.hpp"

#include "enocean_frame.h"

namespace EnOcean
{
  class DeviceState {
  public:
    DeviceState() {};
    virtual ~DeviceState() {}
    virtual void to_json(json& j) const {}
    virtual void from_json(const json& j) {}

    std::map<uint16_t, std::vector<uint8_t>> extended;
    double last_rrsi = 0.0;
  };

  std::shared_ptr<DeviceState> mk_device_state(const EEP &eep);
  std::shared_ptr<DeviceState> mk_device_state(const EEP &eep, const json &j);

  class DeviceConfiguration {
  public:
    DeviceConfiguration() {};
    virtual ~DeviceConfiguration() {}

    EEP eep;
    MID mid;
    bool dirty = false;
    uint32_t security_code = 0;
    std::string ddf;
    std::vector<std::pair<uint16_t, uint8_t>> extended;

    virtual void to_json(json& j) const {
      j["eep"] = eep;
      j["mid"] = mid;
      j["security_code"] = security_code;
      if (!extended.empty())
        j["extended"] = extended;
      j["ddf"] = ddf;
    }

    virtual void from_json(const json& j) {
      auto eep_it = j.find("eep"); if (eep_it != j.end()) eep = eep_it->get<EEP>();
      auto mid_it = j.find("mid"); if (mid_it != j.end()) mid = mid_it->get<MID>();
      auto sec_it = j.find("security_code"); if (sec_it != j.end()) security_code = sec_it->get<uint32_t>();
      auto ext_it = j.find("extended"); if (ext_it != j.end()) extended = ext_it->get<std::vector<std::pair<uint16_t, uint8_t>>>();
      auto ddf_it = j.find("ddf"); if (ddf_it != j.end()) ddf = ddf_it->get<std::string>();
    }
  };

  std::shared_ptr<DeviceConfiguration> mk_device_configuration(const EEP &eep);
  std::shared_ptr<DeviceConfiguration> mk_device_configuration(const EEP &eep, const json &j);

  class Telegram {
  public:
    Telegram() {}
    Telegram(Frame &&f) : frame(std::move(f)) {}
    Telegram(const Frame &f) : frame(f) {}
    virtual ~Telegram() {}

    operator const std::vector<uint8_t>&() const { return frame; }
    operator const Frame&() const { return frame; }

    TXID txid() const { return frame.txid(); }

    virtual void to_json(json& j) const {}
    virtual void from_json(const json& j) {}

  protected:
    Frame frame;
  };

  class AddressedTelegram : public Telegram {
  public:
    AddressedTelegram(Frame &&f) : Telegram(std::move(f)) {
      if (frame.rorg() != 0xA6)
        throw std::runtime_error("not an addressed telegram");
    }
    AddressedTelegram(const Frame &f) : Telegram(f) {
      if (frame.rorg() != 0xA6)
        throw std::runtime_error("not an addressed telegram");
    }
    AddressedTelegram(const Telegram &t, TXID destination);
    virtual ~AddressedTelegram() {}

    TXID destination() const {
      return  (frame.data()[frame.size() - 11] << 24) |
              (frame.data()[frame.size() - 10] << 16) |
              (frame.data()[frame.size() - 9] << 8) |
              (frame.data()[frame.size() - 8]);
    }
  };

  class Telegram_4BS : public Telegram {
  public:
    Telegram_4BS() : Telegram(Frame({ 0xA5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })) {}
    Telegram_4BS(Frame &&f) : Telegram(std::move(f)) {
      if (frame.rorg() != 0xA5 || frame.size() != 11)
        throw std::runtime_error("not a 4BS telegram");
    }
    Telegram_4BS(const Frame &f) : Telegram(f) {
      if (frame.rorg() != 0xA5 || frame.size() != 11)
        throw std::runtime_error("not a 4BS telegram");
    }
    Telegram_4BS(const std::array<uint8_t, 4> &data, TXID source, uint8_t status);
    Telegram_4BS(const std::array<uint8_t, 4> &data, TXID source, TXID destination, uint8_t status);
    virtual ~Telegram_4BS() {}

    bool is_teach_in() const { return (frame.data()[3] & 0x08) == 0; }
    uint8_t rorg() const { return frame.rorg(); }
  };

  class Telegram_LEARN_4BS_3 : public Telegram_4BS {
  public:
    Telegram_LEARN_4BS_3() : Telegram_4BS() {}
    Telegram_LEARN_4BS_3(Frame &&f) : Telegram_4BS(std::move(f)) {}
    Telegram_LEARN_4BS_3(const Frame &f) : Telegram_4BS(f) {}
    Telegram_LEARN_4BS_3(
      uint8_t func, uint8_t type, MID mid, TXID source,
      bool learn_status = true, bool learn_eep_supported = true, bool learn_result = true,
      uint8_t status = 0);
    Telegram_LEARN_4BS_3(
      uint8_t func, uint8_t type, MID mid, TXID source, TXID destination,
      bool learn_status = true, bool learn_eep_supported = true, bool learn_result = true,
      uint8_t status = 0);
    Telegram_LEARN_4BS_3(
      const EEP &eep, MID mid, TXID source, TXID destination,
      bool learn_status = true, bool learn_eep_supported = true, bool learn_result = true,
      uint8_t status = 0);
    virtual ~Telegram_LEARN_4BS_3() {}

    uint8_t func() const { return frame.data()[0] >> 2; }
    uint8_t type() const { return ((frame.data()[0] & 0x03) << 5) | (frame.data()[1] >> 3); }
    MID mid() const { return (frame.data()[1] & 0x07 << 8) | frame.data()[2]; }
    EEP eep() const { return { rorg(), func(), type() }; }
    bool learn_type_with_eep() const { return (frame.data()[3] & 0x80) != 0; }
  };

  enum class RMCC : uint16_t {
    // Remote Management Control Commands
    RESERVED = 0x000,
    UNLOCK = 0x001,
    LOCK = 0x002,
    SET_CODE = 0x003,
    QUERY_ID = 0x004,
    QUERY_ID_ANSWER = 0x604,
    QUERY_ID_ANSWER_EXTENDED = 0x704,
    ACTION_COMMAND = 0x005,
    PING_COMMAND = 0x006,
    PING_ANSWER = 0x606,
    QUERY_FUNCTION = 0x007,
    QUERY_STATUS = 0x008,
    QUERY_STATUS_ANSWER = 0x608,

    START_SESSION = 0x009,
    CLOSE_SESSION = 0x00A,

    // Remote Commissioning Control Commands
    REMOTE_COMMISSIONING_ACKNOWLEDGE = 0x240,
    GET_LINK_TABLE_METADATA_QUERY = 0x210,
    GET_LINK_TABLE_METADATA_RESPONSE = 0x810,
    GET_LINK_TABLE_QUERY = 0x211,
    GET_LINK_TABLE_RESPONSE = 0x811,
    SET_LINK_TABLE_CONTENT = 0x212,
    GET_LINK_TABLE_GP_ENTRY_QUERY = 0x213,
    GET_LINK_TABLE_GP_ENTRY_RESPONSE = 0x813,
    SET_LINK_TABLE_GP_ENTRY_CONTENT = 0x214,
    GET_SECURITY_PROFILE_QUERY = 0x215,
    GET_SECURITY_PROFILE_RESPONSE = 0x815,
    SET_SECURITY_PROFILE = 0x216,
    REMOTE_SET_LEARN_MODE = 0x220,
    TRIGGER_OUTBOUND_REMOTE_TEACH_REQUEST = 0x221,
    GET_DEVICE_CONFIGURATION_QUERY = 0x230,
    GET_DEVICE_CONFIGURATION_RESPONSE = 0x830,
    SET_DEVICE_CONFIGURATION_QUERY = 0x231,
    GET_LINK_BASED_CONFIGURATION_QUERY = 0x232,
    GET_LINK_BASED_CONFIGURATION_RESPONSE = 0x832,
    SET_LINK_BASED_CONFIGURATION_QUERY = 0x233,
    GET_DEVICE_SECURITY_INFORMATION_QUERY = 0x234,
    GET_DEVICE_SECURITY_INFORMATION_RESPONSE = 0x834,
    SET_DEVICE_SECURITY_INFORMATION = 0x235,
    APPLY_CHANGES = 0x226,
    RESET_DEVICE_DEFAULTS = 0x224,
    RADIO_LINK_TEST_CONTROL = 0x225,
    GET_PRODUCT_ID = 0x227,
    GET_PRODUCT_ID_RESPONSE = 0x827,
  };

  enum class RMSC : uint8_t {
    OK_ = 0x00,
    WRONG_TARGET_ID = 0x01,
    WRONG_UNLOCK_CODE = 0x02,
    WRONG_EEP = 0x03,
    WRONG_MANUFACTURER_ID = 0x04,
    WRONG_DATA_SIZE = 0x05,
    NO_CODE_SET = 0x06,
    NOT_SENT = 0x07,
    RPC_FAILED = 0x08,
    MESSAGE_TIME_OUT = 0x09,
    TOO_LONG_MESSAGE = 0x0A,
    MESSAGE_PART_ALREADY_RECEIVED = 0x0B,
    MESSAGE_PART_NOT_RECEIVED = 0x0C,
    ADDRESS_OUT_OF_RANGE = 0x0D,
    CODE_DATA_SIZE_EXCEEDED = 0x0E,
    WRONG_DATA = 0x0F
  };

  class Telegram_SYS_EX_ERP1 : public Telegram {
  public:
    Telegram_SYS_EX_ERP1() : Telegram(Frame(std::vector<uint8_t>(16, 0))) {}
    Telegram_SYS_EX_ERP1(Frame &&f) : Telegram(std::move(f)) {
      if (frame.rorg() != 0xC5 || frame.size() != 16)
        throw std::runtime_error("not a SYS_EX telegram");
    }
    Telegram_SYS_EX_ERP1(const Frame &f) : Telegram(f) {
      if (frame.rorg() != 0xC5 || frame.size() != 16)
        throw std::runtime_error("not a SYS_EX telegram");
    }
    Telegram_SYS_EX_ERP1(uint8_t SEQ, uint8_t IDX, MID mid, RMCC fn, const std::vector<uint8_t> &payload, TXID source, uint8_t status);
    virtual ~Telegram_SYS_EX_ERP1() {}

    uint16_t SEQ() const { return frame.data()[0] >> 6; }
    uint32_t IDX() const { return frame.data()[0] & 0x3F; }
    uint64_t data() const {
      uint64_t r = 0;
      for (size_t i = 1; i < 9; i++)
        r = r << 8 | frame.data()[i];
      return r;
    };
    RMCC rmcc() const { return static_cast<RMCC>((frame.data()[2] & 0x00F) << 8 | frame.data()[3]); }
    size_t data_size() const {
      return IDX() == 0 ? 4 : 8;
    }
    const uint8_t* raw_data() const {
      return IDX() == 0 ? &frame.data()[5] : &frame.data()[1];
    }
  };

  class SignalTelegram : public Telegram {
  public:
    enum class MessageIndex : uint8_t {
      SMARTAckMailboxEmpty = 0x01,
      SMARTACKMailboxDoesNotExist = 0x02,
      SMARTACKResetTriggerLRNRequest = 0x03,
      TriggerStatusMessageOfDevice = 0x04,
      LastUnicastMessageAcknowledge = 0x05,
      EnergyStatusOfDevice = 0x06,
      RevisionOfDevice = 0x07,
      Heartbeat = 0x08,
      RXWindowOpen = 0x09,
      RXChannelQuality = 0x0A,
      DutyCycleStatus = 0x0B,
      ConfigurationOfDeviceChanged = 0x0C,
      EnergyDeliveryOfTheHarvester = 0x0D,
      TXModeOff = 0x0E,
      TXModeOn = 0x0F,
      BackupBatteryStatus = 0x10,
      LearnodeStatus = 0x11,
      ProductID = 0x12
    };

    SignalTelegram(Frame &&f) : Telegram(std::move(f)) {
      if (frame.rorg() != 0xD0 || frame.size() < 8 || frame.size() > 21)
        throw std::runtime_error("not a signal telegram");
    }
    SignalTelegram(MessageIndex message_index, const std::vector<uint8_t> &optional_data, TXID source, uint8_t status);
    virtual ~SignalTelegram() {}

    MessageIndex message_index() const { return static_cast<MessageIndex>(frame.data()[0]); }
    const uint8_t* optional_data() const { return &frame.data()[2]; }
  };
}

inline void to_json(json& j, const EnOcean::DeviceState& p) { p.to_json(j); }
inline void from_json(const json& j, EnOcean::DeviceState& p) { return p.from_json(j); }
inline void to_json(json& j, const EnOcean::DeviceConfiguration& p) { p.to_json(j); }
inline void from_json(const json& j, EnOcean::DeviceConfiguration& p) { return p.from_json(j); }
inline void to_json(json& j, const EnOcean::Telegram& p) { p.to_json(j); }
inline void from_json(const json& j, EnOcean::Telegram& p) { return p.from_json(j); }

#endif // _ENOCEAN_TELEGRAMS_H_
