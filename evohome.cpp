// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "evohome.h"

Evohome::Decoder::Decoder() : ::Decoder() {}

Evohome::Decoder::~Decoder() {}

// Bytes are flanked by start/stop bits and reversed.
static int get_frbyte(const std::vector<uint8_t> &buf, size_t pos, uint8_t *out)
{
  size_t end = buf.size() * 8;

  if (pos < 0 || pos >= end)
    return 0;

  *out = 0;

  if (pos >= end ||
    get_bit(buf, pos++) != 0 ||
    pos + 8 >= end)
    return 0;

  for (unsigned i = 0; i < 8; i++)
    *out = *out | (get_bit(buf, pos++) << i);

  if (pos >= end ||
    get_bit(buf, pos++) != 1 ||
    pos >= end)
    return 0;

  return 10;
}

typedef struct { uint8_t t; const char s[4]; } dev_map_entry_t;

static const dev_map_entry_t device_map[] = {
  { .t = 01, .s = "CTL" }, /* Controller */
  { .t = 02, .s = "UFH" }, /* Underfloor heating (HCC80, HCE80) */
  { .t = 03, .s = " 30" }, /* HCW82?? */
  { .t = 04, .s = "TRV" }, /* Thermostatic radiator valve (HR80, HR91, HR92) */
  { .t = 07, .s = "DHW" }, /* DHW sensor (CS92) */
  { .t = 10, .s = "OTB" }, /* OpenTherm bridge (R8810) */
  { .t = 12, .s = "THm" }, /* Thermostat with setpoint schedule control (DTS92E, CME921) */
  { .t = 13, .s = "BDR" }, /* Wireless relay box (BDR91) (HC60NG too?) */
  { .t = 17, .s = " 17" }, /* Dunno - Outside weather sensor? */
  { .t = 18, .s = "HGI" }, /* Honeywell Gateway Interface (HGI80, HGS80) */
  { .t = 22, .s = "THM" }, /* Thermostat with setpoint schedule control (DTS92E) */
  { .t = 30, .s = "GWY" }, /* Gateway (e.g. RFG100?) */
  { .t = 32, .s = "VNT" }, /* (HCE80) Ventilation (Nuaire VMS-23HB33, VMN-23LMH23) */
  { .t = 34, .s = "STA" }, /* Thermostat (T87RF) */
  { .t = 63, .s = "NUL" }, /* No device */
};

// static void decode_device_id(const uint8_t device_id[3], char *buf, size_t buf_sz)
// {
//     uint8_t dev_type = device_id[0] >> 2;
//     const char *dev_name = " --";
//     for (size_t i = 0; i < sizeof(device_map) / sizeof(dev_map_entry_t); i++)
//         if (device_map[i].t == dev_type)
//             dev_name = device_map[i].s;

//     if (buf_sz < (6 + strlen(dev_name) + 1))
//         return;

//     sprintf(buf, "%3s:%06d", dev_name, (device_id[0] & 0x03) << 16 | (device_id[1] << 8) | device_id[2]);
// }

Evohome::State::State() : need_ui_rebuild(false)
{
}

Evohome::State::~State() {}

void Evohome::State::Update(const std::vector<uint8_t> &msg)
{
  unsigned ipos = 0;

  // printf("\n");
  // for (size_t i=0; i < msg.size(); i++)
  //   printf("%02x", msg[i]);
  // printf("\n");

  size_t num_zones = zones.size();
  size_t num_devices = devices.size();

  uint8_t msg_header = msg[ipos++];

  size_t num_device_ids = msg_header == 0x14 ? 1 :
                          msg_header == 0x18 ? 2 :
                          msg_header == 0x1c ? 2 :
                          msg_header == 0x10 ? 2 :
                          (msg_header >> 2) & 0x03; // total speculation.

  std::vector<uint32_t> msg_device_ids(num_device_ids, 0);
  for (size_t i = 0; i < num_device_ids; i++)
    for (size_t j = 0; j < 3; j++)
      msg_device_ids[i] = (msg_device_ids[i] << 8) | msg[ipos++];

  uint16_t msg_command = msg[ipos++] << 8;
  msg_command |= msg[ipos++];
  size_t msg_payload_length = msg[ipos++];

  std::vector<uint8_t> msg_payload(msg_payload_length, 0);
  for (size_t i = 0; i < msg_payload_length; i++)
    msg_payload[i] = msg[ipos++];

  // size_t msg_num_unparsed = msg.size() - ipos - 1;


  /* Sources of inspiration:
      https://github.com/Evsdd/The-Evohome-Protocol/wiki
      https://www.domoticaforum.eu/viewtopic.php?f=7&t=5806&start=30
      (specifically https://www.domoticaforum.eu/download/file.php?id=1396) */

#define UNKNOWN_IF(C) { if (C) throw std::runtime_error("interpretation failed"); }

  switch(msg_command) {
    case 0x1030: {
      UNKNOWN_IF(msg_payload_length != 16);
      size_t zone_idx = msg_payload[0];

      Zone &zone = zones[zone_idx];

      for (size_t i = 0; i < 5; i++) { // order fixed?
        const uint8_t *p = &msg_payload[1 + 3*i];
        // *(p+1) == 0x01 always?
        int value = *(p+2);
        switch (*p) {
          case 0xC8: zone.max_flow_temp = value; break;
          case 0xC9: zone.pump_run_time = value; break;
          case 0xCA: zone.actuator_run_time = value; break;
          case 0xCB: zone.min_flow_temp = value; break;
          case 0xCC: /* Unknown, always 0x01? */ break;
          default:
            char tmp[256];
            sprintf(tmp, "Unknown parameter to 0x1030: %x02d=%04d\n", *p, value);
            throw std::runtime_error(tmp);
        }
      }
      break;
    }
    case 0x313F: {
      UNKNOWN_IF((msg_payload_length != 1 && msg_payload_length != 9) || msg_device_ids.size() == 0);
      switch (msg_payload_length) {
        case 1: /* time request*/ break;
        case 9: {
          // uint8_t unknown_0 = msg_payload[0]; /* always == 0? */
          // uint8_t unknown_1 = msg_payload[1]; /* direction? */
          uint8_t second = msg_payload[2];
          uint8_t minute = msg_payload[3];
          // uint8_t day_of_week = msg_payload[4] >> 5;
          uint8_t hour = msg_payload[4] & 0x1F;
          uint8_t day = msg_payload[5];
          uint8_t month = msg_payload[6];
          uint8_t year[2] = { msg_payload[7],  msg_payload[8] };
          char time_str[256];
          sprintf(time_str, "%02d:%02d:%02d %02d-%02d-%04d", hour, minute, second, day, month, (year[0] << 8) | year[1]);
          devices[msg_device_ids[0]].datetime = time_str;
          break;
        }
      }
      break;
    }
    case 0x0008: {
      UNKNOWN_IF(msg_payload_length != 2);
      Zone &zone = zones[msg_payload[0]];
      zone.demand = msg_payload[1] / 2.0;
      break;
    }
    case 0x3ef0: {
      UNKNOWN_IF((msg_payload_length != 3 && msg_payload_length != 6) || msg_device_ids.size() == 0)
      switch (msg_payload_length) {
        case 3:
          devices[msg_device_ids[0]].status = 100.0 * (msg_payload[1] / 200.0) /* 0xC8 */;
          break;
        case 6:
          devices[msg_device_ids[0]].boiler_modulation_level = 100.0 * (msg_payload[1] / 200.0) /* 0xC8 */;
          devices[msg_device_ids[0]].flame_status = msg_payload[3];
          break;
      }
      break;
    }
    case 0x2309: {
      UNKNOWN_IF(msg_payload_length != 3);
      // Observation: CM921 reports a very high setpoint during binding (0x7eff); packet: 143255c1230903017efff7
      const size_t zone_idx = msg_payload[0];
      zones[zone_idx].setpoint = ((msg_payload[1] << 8) | msg_payload[2]) / 100.0;
      break;
    }
    case 0x1100: {
      UNKNOWN_IF((msg_payload_length != 5 && msg_payload_length != 8) || msg_device_ids.size() == 0);
      Device &dev = devices[msg_device_ids[0]];
      dev.domain_id = msg_payload[0];
      dev.cycle_rate = msg_payload[1] / 4.0;
      dev.minimum_on_time = msg_payload[2] / 4.0;
      dev.minimum_off_time = msg_payload[3] / 4.0;
      if (msg_payload_length == 8)
        dev.proportional_band_width = ((msg_payload[5] << 8) | msg_payload[6]) / 100.0;
      break;
    }
    case 0x0009: {
      UNKNOWN_IF(msg_payload_length != 3 || msg_device_ids.size() == 0);
      Device &dev = devices[msg_device_ids[0]];
      dev.device_number = msg_payload[0];
      switch (msg_payload[1]) {
        case 0: dev.failsafe_mode = FSM_OFF; break;
        case 1: dev.failsafe_mode = FSM_20_80; break;
        default: dev.failsafe_mode = FSM_UNKNOWN;
      }
      break;
    }
    case 0x3B00: {
      UNKNOWN_IF(msg_payload_length != 2 || msg_device_ids.size() == 0);
      Device &dev = devices[msg_device_ids[0]];
      dev.domain_id = msg_payload[0];
      dev.state = msg_payload[1] / 200.0 /* 0xC8 */;
      break;
    }
    default: /* Unknown command */
      UNKNOWN_IF(true);
  }

  if (num_zones != zones.size() || num_devices != devices.size())
    need_ui_rebuild = true;
}

const std::string& Evohome::Decoder::Decode(std::vector<uint8_t> &bytes)
{
  size_t num_bytes = bytes.size();
  size_t bpos = 0;

#define FAIL_IF(C,M) { if (C) { throw std::runtime_error(M); } }
#define FAIL_IF_EXT(C,R) { if (C) { R; throw std::runtime_error(tmp); } }

  FAIL_IF(num_bytes == 0, "no bytes");

  // skip 01, last bits of preamble
  while (get_bit(bytes, bpos) == 0 && bpos < bytes.size())
    bpos++;
  FAIL_IF(get_bit(bytes, bpos++) != 1, "Leading 1 missing.");

  // skip Manchester-breaking header
  uint8_t header[3] = { 0x33, 0x55, 0x53 };
  uint8_t b;
  for (size_t i=0; i < 3; i++) {
    int r = get_frbyte(bytes, bpos, &b);
    FAIL_IF_EXT(r != 10, {
      snprintf(tmp, sizeof(tmp), "Could not read header byte %d.", i);
    });
    bpos += r;
    FAIL_IF_EXT(b != header[i], {
      snprintf(tmp, sizeof(tmp), "header[%d] %02x != %02x mismatch.", i, b, header[i]);
    });
  }

  // Find Footer 0x35 (0x55*)?

  // read flanked and reversed bytes
  size_t end = bytes.size() * 8;
  size_t num_frbytes = 0;
  while (bpos < end) {
    if (get_frbyte(bytes, bpos, &b) != 10)
      break;
    bpos += 10;
    bytes[num_frbytes++] = b;
  }
  bytes.resize(num_frbytes);

  // Manchester decode
  size_t decoded = 0;
  uint8_t crc = 0;
  b = 0;
  if (num_frbytes % 2) num_frbytes--;
  for (size_t i=0; i < num_frbytes; i++) {
    for (size_t j=0; j < 8; j+=2) {
      size_t binx = i*8 + j;
      FAIL_IF(binx > num_frbytes*8, "Not enough data");

      uint8_t b0 = get_bit(bytes, binx);
      uint8_t b1 = get_bit(bytes, binx + 1);
      if (b0 == b1) {
        // Manchester-breaking byte means we must have reached the end.
        // (Footer begins with 0x35 which breaks Manchester.)
        goto man_end;
      }
      b = (b << 1) | b1;
    }
    if (i % 2 == 1) {
      bytes[decoded++] = b;
      crc += b;
      b = 0;
    }
  }
  man_end:

  if (crc != 0)
    throw std::runtime_error("CRC failed.");

  if (decoded == 0)
    throw std::runtime_error("Unknown decoder error.");

  bytes.resize(decoded - 1);
  state.Update(bytes);

  for (size_t i=0; i < bytes.size(); i++)
    sprintf(tmp + 2*i, "%02x", bytes[i]);

  message.str = std::string(tmp, bytes.size() * 2);
  return message.str;
}

static size_t add_bit(std::vector<uint8_t> &buf, size_t bpos, bool val)
{
  size_t byte_inx = bpos / 8;
  if (byte_inx >= buf.size())
    buf.resize(byte_inx + 1, 0);
  uint8_t &byte = buf[byte_inx];
  size_t bit_inx = bpos % 8;
  byte = val ? (byte | (0x01 << (8 - bit_inx - 1)))
             : (byte & ~(0x01 << (8 - bit_inx - 1)));
  return bpos + 1;
}

static size_t add_byte(std::vector<uint8_t> &buf, size_t bpos, uint8_t val)
{
  for (size_t i=0; i < 8; i++)
    bpos = add_bit(buf, bpos, (0x01ul << (8 - i - 1)));
  return bpos;
}

static size_t add_fmnibble(std::vector<uint8_t> &buf, size_t bpos, uint8_t val)
{
  bpos = add_bit(buf, bpos, false);
  for (size_t j=0; j < 4; j++) {
    bool v = (val & (0x01 << j)) != 0;
    bpos = add_bit(buf, bpos, v);
    bpos = add_bit(buf, bpos, !v);
  }
  return add_bit(buf, bpos, true);
}

static size_t add_fmbyte(std::vector<uint8_t> &buf, size_t bpos, uint8_t val, uint8_t *sum)
{
  if (sum) *sum += val;
  bpos = add_fmnibble(buf, bpos, val >> 4);
  return add_fmnibble(buf, bpos, val & 0x0F);
}

static size_t add_fbyte(std::vector<uint8_t> &buf, size_t bpos, uint8_t val)
{
  bpos = add_bit(buf, bpos, false);
  for (size_t j=0; j < 8; j++) {
    bool v = (val & (0x01 << j)) != 0;
    bpos = add_bit(buf, bpos, v);
  }
  return add_bit(buf, bpos, true);
}


Evohome::Encoder::Encoder() {}

Evohome::Encoder::~Encoder() {}

std::vector<uint8_t> Evohome::Encoder::Encode(const std::vector<uint8_t> &data)
{
  std::vector<uint8_t> buf;
  size_t bpos = 0;
  uint8_t bsum = 0;

  buf.reserve(1024);

  // End of preamble/sync word
  bpos = add_bit(buf, bpos, false);
  bpos = add_bit(buf, bpos, false);
  bpos = add_bit(buf, bpos, true);

  uint8_t header[3] = { 0x33, 0x55, 0x53 };
  for (auto b : header)
    bpos = add_fbyte(buf, bpos, b);

  for (auto b : data)
    bpos = add_fmbyte(buf, bpos, b, &bsum);

  bpos = add_fmbyte(buf, bpos, -bsum, &bsum);

  uint8_t footer[3] = { 0x35, 0x55, 0x55 };
  for (auto b : footer)
    bpos = add_fbyte(buf, bpos, b);

  return buf;
}
