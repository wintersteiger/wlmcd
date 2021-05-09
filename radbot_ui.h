// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RADBOT_UI_H_
#define _RADBOT_UI_H_

#include <memory>

#include "ui.h"
#include "radbot.h"

class DeviceBase;

class RadbotUI : public UI {
protected:
  time_t last_update;
  const Radbot::State &state;

public:
  RadbotUI(const Radbot::State &state, std::vector<std::shared_ptr<DeviceBase>> devices);
  virtual ~RadbotUI();

  void Update(bool full) { UI::Update(full); }

  virtual std::string Name() const { return "Radbot"; }
};

#endif
