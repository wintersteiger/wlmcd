// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cmath>
#include <csignal>
#include <ctime>
#include <random>
#include <thread>

#include <ui.h>
#include <decoder.h>
#include <integrity.h>
#include <sleep.h>

#include "radio_test_tracker.h"

// 434: FREND0 17 + 00120e3460c5c1c0
// FREND0 11 + 03171d265086cdc0 ?
// FREND0 17 + 00030f2750c8c5c0 ?

const RadioTestTracker::ID RadioTestTracker::broadcast = { 0xFF, 0xFF, 0xFF, 0xFF };

RadioTestTracker::RadioTestTracker(std::function<void(const std::vector<uint8_t>&)> &&transmit) :
  DeviceBase(),
  transmit(transmit),
  num_rx(0),
  num_tx(0),
  num_pings_rx(0),
  num_pings_tx(0),
  num_replies_rx(0),
  num_replies_tx(0),
  num_crc_errors(0),
  randomize(true),
  running(false)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint8_t> d(0, UINT8_MAX);
  id[0] = d(gen);
  id[1] = d(gen);
  id[2] = d(gen);
  id[3] = d(gen);
}

RadioTestTracker::~RadioTestTracker() {}

class Frame {
public:
  Frame(const std::vector<uint8_t> &buffer, size_t offset) :
    buffer(buffer), offset(offset)
  {}
  virtual ~Frame() {}

  uint8_t seq_no() const { return buffer[offset+1]; }
  RadioTestTracker::ID source() const { return { buffer[offset+2], buffer[offset+3], buffer[offset+4], buffer[offset+5] }; }
  RadioTestTracker::ID destination() const { return { buffer[offset+6], buffer[offset+7], buffer[offset+8], buffer[offset+9] }; }
  uint8_t crc() const { return buffer[offset+10]; }

  bool crc_ok() const {
    uint8_t crc_act = crc8(&buffer.data()[offset], 10, 0x07);
    if (crc_act != crc()) {
      UI::Log("CRC error %02x != %02x", crc_act, crc());
      return false;
    }
    return true;
  }

protected:
  const std::vector<uint8_t> &buffer;
  size_t offset;
};

void RadioTestTracker::receive(const std::vector<uint8_t> &packet)
{
  // UI::Log("PKT: %s", bytes_to_hex(packet).c_str());
  num_rx++;

  if (packet.size() >= 11)
  {
    for (size_t i=0; i < packet.size() - 10; i++)
    {
      if (packet[i] == 0x55)
      {
        Frame frame(packet, i);
        ID source = frame.source();
        ID destination = frame.destination();
        uint8_t seq_no = frame.seq_no();
        uint8_t crc_exp = frame.crc();
        bool crc_ok = frame.crc_ok();

        if (!crc_ok)
          num_crc_errors++;

        if (destination == id)
        {
          // UI::Log("Pong from %02x%02x%02x%02x: seq_no=%u",
          //           source[0], source[1], source[2], source[3],
          //           seq_no);
          num_replies_rx++;

          auto pit = parties.find(source);
          if (pit == parties.end()) {
            if (crc_ok)
              parties[source] = { seq_no, 1u, 0u, crc_ok ? 0u : 1u, 0u };
          }
          else {
            if (!crc_ok) pit->second.num_crc_errors++;
            pit->second.num_replies_rx++;
          }
        }
        else if (destination == broadcast)
        {
          // UI::Log("Ping from %02x%02x%02x%02x: seq_no=%u",
          //           source[0], source[1], source[2], source[3],
          //           seq_no);

          std::vector<uint8_t> reply = { 0x55, seq_no };

          for (auto &b : id)
            reply.push_back(b);
          for (auto &b : source)
            reply.push_back(b);
          reply.push_back(crc8(reply));

          send(reply);

          auto pit = parties.find(source);
          if (pit == parties.end()) {
            if (crc_ok)
              parties[source] = { seq_no, 0u, 1u, crc_ok ? 0u : 1u, 0u };
          }
          else {
            pit->second.num_replies_tx++;
            if (!crc_ok) pit->second.num_crc_errors++;
            if (seq_no < pit->second.last_seq_no) pit->second.num_out_of_order++;
            pit->second.last_seq_no = seq_no;
          }

          num_pings_rx++;
          num_replies_tx++;
        }
        else
        {
          UI::Log("Observing %02x%02x%02x%02x -> %02x%02x%02x%02x: seq_no=%u",
                    source[0], source[1], source[2], source[3],
                    destination[0], destination[1], destination[2], destination[3],
                    seq_no);
          i += 10;
        }
      }
    }
  }
}

void RadioTestTracker::send(const std::vector<uint8_t> &packet)
{
  if (randomize) {
    std::thread t([this, packet]() {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<> d(0.0, 1.0);
      sleep_ms(250.0 * d(gen));
      transmit(packet);
    });
    t.detach();
  }
  else
    transmit(packet);
  num_tx++;
}

void RadioTestTracker::ping()
{
  std::vector<uint8_t> packet;
  packet.push_back(0x55);
  packet.push_back(num_pings_tx++);
  for (auto &b : id)
    packet.push_back(b);
  for (auto &b : broadcast)
    packet.push_back(b);
  packet.push_back(crc8(packet));
  send(packet);
}

void RadioTestTracker::UpdateTimed()
{
  if (running)
    ping();
}