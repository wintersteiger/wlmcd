// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RADIO_TEST_TRACKER_
#define _RADIO_TEST_TRACKER_

#include <functional>
#include <vector>
#include <mutex>
#include <iostream>
#include <map>

#include <device.h>

class RadioTestTracker : public DeviceBase {
public:
  typedef std::array<uint8_t, 4> ID;

  struct PartyStatistics {
    uint64_t last_seq_no;
    uint64_t num_pings_tx;
    uint64_t num_replies_rx;
    uint64_t num_replies_tx;
    uint64_t num_crc_errors;
    uint64_t num_out_of_order;
  };

  RadioTestTracker(std::function<void(const std::vector<uint8_t>&)> &&transmit);
  virtual ~RadioTestTracker();

  void receive(const std::vector<uint8_t> &packet);
  void send(const std::vector<uint8_t> &packet);
  void ping();
  void run() { running = true; }
  void stop() { running = false; };

  ID id;
  uint64_t num_rx;
  uint64_t num_tx;
  uint64_t num_pings_rx;
  uint64_t num_pings_tx;
  uint64_t num_replies_rx;
  uint64_t num_replies_tx;
  uint64_t num_crc_errors;

  std::map<ID, PartyStatistics> parties;

  virtual const char* Name() const { return "Radio Test Tracker"; }
  virtual void UpdateTimed() override;
  virtual void Write(std::ostream &os) const override {}
  virtual void Read(std::istream &os) override {}

protected:
  std::mutex mtx;
  std::function<void(const std::vector<uint8_t>&)> transmit;
  static const ID broadcast;
  bool randomize;
  bool running;
};

#endif // _RADIO_TEST_TRACKER_
