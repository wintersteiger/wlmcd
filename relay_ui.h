// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RELAY_UI_H_
#define _RELAY_UI_H_

#include <memory>

#include "ui.h"
#include "relay_device.h"

class RelayUI : public UI {
public:
  RelayUI(std::shared_ptr<RelayDevice> relay);
  virtual ~RelayUI();

  virtual std::string Name() const { return "Relay"; }
  virtual void Layout();
};

#endif // _RELAY_UI_H_
