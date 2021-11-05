// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RFM69_UI_H_
#define _RFM69_UI_H_

#include <cstdint>
#include <vector>

#include "ui.h"

class RFM69;
class GPIOButton;

class RFM69UI: public UI {
protected:
  std::shared_ptr<RFM69> rfm69;
  size_t num_status_fields;

public:
  RFM69UI(std::shared_ptr<RFM69> rfm69, GPIOButton *reset_button = NULL);
  virtual ~RFM69UI();

  virtual std::string Name() const { return "RFM69"; }
  virtual void Layout();
};

#endif
