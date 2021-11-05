// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "radio_test_tracker.h"
#include "radio_test_ui.h"

RadioTestUI::RadioTestUI(const std::vector<std::shared_ptr<DeviceBase>> radio_devices, std::shared_ptr<RadioTestTracker> tracker) :
  UI(),
  row(0),
  col(0),
  tracker(tracker)
{
  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  fields.push_back(new Empty(row++, col));

  Add(new LField<std::string>(UI::statusp, row++, col, 8, "Our ID", "",
      [tracker]() { return bytes_to_hex(tracker->id); }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# packets RX", "",
      [tracker]() { return tracker->num_rx; }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# packets TX", "",
      [tracker]() { return tracker->num_tx; }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# pings RX", "",
      [tracker]() { return tracker->num_pings_rx; }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# pings TX", "",
      [tracker]() { return tracker->num_pings_tx; }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# replies RX", "",
      [tracker]() { return tracker->num_replies_rx; }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# replies TX", "",
      [tracker]() { return tracker->num_replies_tx; }));
  Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# CRC errors", "",
      [tracker]() { return tracker->num_crc_errors; }));
}

RadioTestUI::~RadioTestUI() {}

void RadioTestUI::Update(bool full)
{
  bool changed = false;

  for (const auto &kv : tracker->parties)
  {
    const RadioTestTracker::ID &id = kv.first;
    const RadioTestTracker::PartyStatistics &p = kv.second;


    if (tracker_ids.find(id) == tracker_ids.end())
    {
      row++;
      fields.push_back(new Empty(row++, col));
      Add(new LField<std::string>(UI::statusp, row++, col, 8, "Their ID", "",
        [id]() { return bytes_to_hex(id); }));
      Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "Last seq no", "",
        [&p]() { return p.last_seq_no; }));
      Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# replies RX", "",
        [&p]() { return p.num_replies_rx; }));
      Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# replies TX", "",
        [&p]() { return p.num_replies_tx; }));
      Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# CRC errors", "",
        [&p]() { return p.num_crc_errors; }));
      Add(new LField<uint64_t>(UI::statusp, row++, col, 8, "# out of order", "",
        [&p]() { return p.num_out_of_order; }));
      Add(new LField<double>(UI::statusp, row++, col, 8, "Frame loss", "%",
        [&p, t = this->tracker]() { return 100.0 * (1.0 - p.num_replies_rx/(double)t->num_pings_tx); }));

      tracker_ids.insert(id);
      changed = true;
    }

    if (changed)
      UI::Layout();
  }

  UI::Update(full || changed);
}