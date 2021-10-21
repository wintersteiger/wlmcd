// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "enocean_telegrams.h"

namespace EnOcean
{
  Telegram_4BS::Telegram_4BS(const std::array<uint8_t, 4> &data, TXID source, uint8_t status, Frame &f)
  {
    f = Frame(0xA5, {data.begin(), data.end()}, source, status);
  }

  Telegram_4BS::Telegram_4BS(const std::array<uint8_t, 4> &data, TXID source, TXID destination, uint8_t status, Frame &f)
  {
    std::vector<uint8_t> bytes(1 + 4 + 4, 0);
    bytes[0] = 0xA5;
    bytes[1] = data[0];
    bytes[2] = data[1];
    bytes[3] = data[2];
    bytes[4] = data[3];
    bytes[5] = (destination >> 24) & 0xFF;
    bytes[6] = (destination >> 16) & 0xFF;
    bytes[7] = (destination >> 8) & 0xFF;
    bytes[8] = destination & 0xFF;
    f = Frame(0xA6, bytes, source, status);
  }

  Telegram_LEARN_4BS_3::Telegram_LEARN_4BS_3(
    uint8_t func, uint8_t type, MID mid, TXID source, Frame &f,
    bool learn_status, bool learn_eep_supported, bool learn_result,
    uint8_t status)
  {
    std::vector<uint8_t> payload(4, 0);
    payload[0] = func << 2 | (type & 0x7F) >> 5;
    payload[1] = (type & 0x7F) << 3 | (mid & 0x07FF) >> 8;
    payload[2] = mid & 0xFF;
    bool learn_type = type != 0 && func != 0 && mid != 0;
    bool learn_bit = false;
    payload[3] =  (learn_type ? 0x01 : 0x00) << 7 |
                  (learn_eep_supported ? 0x01 : 0x00) << 6 |
                  (learn_result ? 0x01 : 0x00) << 5 |
                  (learn_status ? 0x01 : 0x00) << 4 |
                  (learn_bit ? 0x01 : 0x00 << 3);
    f = Frame(0xA5, payload, source, status);
  }

  Telegram_LEARN_4BS_3::Telegram_LEARN_4BS_3(
      uint8_t func, uint8_t type, MID mid, TXID source, TXID destination,
      Frame &f,
      bool learn_status, bool learn_eep_supported, bool learn_result,
      uint8_t status)
  {
    std::vector<uint8_t> payload(1 + 4 + 4, 0);
    payload[0] = 0xA5;
    payload[1] = func << 2 | (type & 0x7F) >> 5;
    payload[2] = (type & 0x7F) << 3 | (mid & 0x07FF) >> 8;
    payload[3] = mid & 0xFF;
    bool learn_type = type != 0 && func != 0 && mid != 0;
    bool learn_bit = false;
    payload[4] =  (learn_type ? 0x01 : 0x00) << 7 |
                  (learn_eep_supported ? 0x01 : 0x00) << 6 |
                  (learn_result ? 0x01 : 0x00) << 5 |
                  (learn_status ? 0x01 : 0x00) << 4 |
                  (learn_bit ? 0x01 : 0x00 << 3);
    payload[5] = (destination >> 24) & 0xFF;
    payload[6] = (destination >> 16) & 0xFF;
    payload[7] = (destination >> 8) & 0xFF;
    payload[8] = destination & 0xFF;
    f = Frame(0xA6, payload, source, status);
  }

  Frame A5_20_06::DeviceConfiguration::mk_update(TXID source, TXID destination, uint8_t status)
  {
    std::vector<uint8_t> payload(1 + 4 + 4, 0);
    payload[0] = 0xA5;
    payload[1] = setpoint;
    payload[2] = rcu_temperature;
    payload[3] = (reference_run ? 0x01 : 0x00) << 7 |
                 (communication_interval & 0x07) << 4 |
                 (summer_mode ? 0x01 : 0x00) << 3 |
                 (set_point_selection ? 0x01 : 0x00) << 2 |
                 (temperature_selection ? 0x01 : 0x00) << 1 |
                 (standby ? 0x01 : 0x00);
    payload[4] = 0;
    payload[5] = (destination >> 24) & 0xFF;
    payload[6] = (destination >> 16) & 0xFF;
    payload[7] = (destination >> 8) & 0xFF;
    payload[8] = destination & 0xFF;
    return Frame(0xA6, payload, source, status);
  }
}
