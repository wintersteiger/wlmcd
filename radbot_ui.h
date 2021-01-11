// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RADBOT_UI_H_
#define _RADBOT_UI_H_

#include <ui.h>
#include <radbot.h>

class RadbotUI : public UI {
protected:
  time_t last_update;
  const Radbot::State &state;

public:
  RadbotUI(const Radbot::State &state);
  virtual ~RadbotUI();

  void Update(bool full) { UI::Update(full); }

  virtual std::string Name() const { return "Radbot"; }
};

#endif
