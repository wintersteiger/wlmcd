// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "enocean_telegrams.h"

namespace EnOcean
{
  AddressedTelegram::AddressedTelegram(const Telegram &t, TXID destination) :
    Telegram()
  {
    std::vector<uint8_t> bytes = t;
    if (bytes.size() < 6)
      throw std::runtime_error("invalid telegram size");
    uint8_t status = bytes[bytes.size()-2];
    bytes.resize(bytes.size() - 6); // strip txid, status, crc
    bytes.push_back((destination >> 24) & 0xFF);
    bytes.push_back((destination >> 16) & 0xFF);
    bytes.push_back((destination >> 8) & 0xFF);
    bytes.push_back(destination & 0xFF);
    frame = Frame(0xA6, bytes, t.txid(), status);
  }

  Telegram_4BS::Telegram_4BS(const std::array<uint8_t, 4> &data, TXID source, uint8_t status)
  {
    frame = Frame(0xA5, {data.begin(), data.end()}, source, status);
  }

  Telegram_4BS::Telegram_4BS(const std::array<uint8_t, 4> &data, TXID source, TXID destination, uint8_t status)
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
    frame = Frame(0xA6, bytes, source, status);
  }

  Telegram_LEARN_4BS_3::Telegram_LEARN_4BS_3(
    uint8_t func, uint8_t type, MID mid, TXID source,
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
    frame = Frame(0xA5, payload, source, status);
  }

  Telegram_LEARN_4BS_3::Telegram_LEARN_4BS_3(
      uint8_t func, uint8_t type, MID mid, TXID source, TXID destination,
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
    frame = Frame(0xA6, payload, source, status);
  }

  Frame A5_20_01::DeviceConfiguration::mk_update(TXID source, TXID destination, uint8_t status)
  {
    std::vector<uint8_t> payload(1 + 4 + 4, 0);
    payload[0] = 0xA5;
    payload[1] = setpoint;
    payload[2] = rcu_temperature;
    payload[3] = (reference_run ? 0x01 : 0x00) << 7 |
                 (lift_set ? 0x01 : 0x00) << 6 |
                 (valve_open ? 0x01 : 0x00) << 5 |
                 (valve_closed ? 0x01 : 0x00) << 4 |
                 (summer_mode ? 0x01 : 0x00) << 3 |
                 (setpoint_selection == SetpointSelection::TEMPERATURE ? 0x01 : 0x00) << 2 |
                 (setpoint_inverse ? 0x01 : 0x00) << 1 |
                 (select_function ? 0x01 : 0x00);
    payload[4] = 0x08;
    payload[5] = (destination >> 24) & 0xFF;
    payload[6] = (destination >> 16) & 0xFF;
    payload[7] = (destination >> 8) & 0xFF;
    payload[8] = destination & 0xFF;
    return Frame(0xA6, payload, source, status);
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
                 (setpoint_selection == SetpointSelection::TEMPERATURE ? 0x01 : 0x00) << 2 |
                 (temperature_selection ? 0x01 : 0x00) << 1 |
                 (standby ? 0x01 : 0x00);
    payload[4] = 0x08;
    payload[5] = (destination >> 24) & 0xFF;
    payload[6] = (destination >> 16) & 0xFF;
    payload[7] = (destination >> 8) & 0xFF;
    payload[8] = destination & 0xFF;
    return Frame(0xA6, payload, source, status);
  }

  Telegram_SYS_EX_ERP1::Telegram_SYS_EX_ERP1(
    uint8_t SEQ, uint8_t IDX,
    MID mid, uint16_t fn, const std::vector<uint8_t> &payload,
    TXID source, uint8_t status) :
    Telegram()
  {
    std::vector<uint8_t> data(9);
    data[0] = (SEQ << 6) | IDX;
    if (IDX == 0)
    {
      if (SEQ > 3)
        throw std::runtime_error("SYS_EX SEQ size too large");
      if (IDX >= (2 << 6))
        throw std::runtime_error("SYS_EX IDX size too large");
      if (payload.size() >= (2 << 9))
        throw std::runtime_error("SYS_EX payload size too large");

      size_t data_length = payload.size();
      data[1] = (data_length >> 1) & 0x0FF;
      data[2] = ((data_length & 0x01) << 7) | ((mid & 0x7FF) >> 4);
      data[3] = ((mid & 0x00F) << 4) | ((fn & 0xFFF) >> 8);
      data[4] = fn & 0xFF;
      for (size_t i = 0; i < 4; i++)
        data[5 + i] = i < payload.size() ? payload[i] : 0;
    }
    else
    {
      size_t start = 4 + 8*(IDX-1);
      for (size_t i = 0; i < 8; i++) {
        size_t di = start + i;
        data[1 + i] = di < payload.size() ? payload[di] : 0;
      }
    }
    frame = Frame(0xC5, data, source, status);
  }
}
