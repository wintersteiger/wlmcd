// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RADIO_TEST_UI_
#define _RADIO_TEST_UI_

#include <memory>
#include <vector>
#include <set>

#include <ui.h>
#include <device.h>

#include "radio_test_tracker.h"


class RadioTestUI : public UI
{
public:
  RadioTestUI(const std::vector<std::shared_ptr<DeviceBase>> radio_devices, std::shared_ptr<RadioTestTracker> tracker);
  virtual ~RadioTestUI();

  void Update(bool full);

  virtual std::string Name() const { return "RTest"; }

protected:
  size_t row, col;
  std::shared_ptr<RadioTestTracker> tracker;
  std::set<RadioTestTracker::ID> tracker_ids;
};

#endif // _RADIO_TEST_UI_
