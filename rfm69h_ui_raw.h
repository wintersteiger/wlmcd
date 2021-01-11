// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RFM69H_UI_RAW_H_
#define _RFM69H_UI_RAW_H_

#include <cstdint>
#include <vector>

#include "ui.h"
#include "field.h"

class RFM69HUIRaw: public UI {
protected:
  RFM69H &rfm69h;

public:
  RFM69HUIRaw(RFM69H &rfm69h);
  virtual ~RFM69HUIRaw();

  virtual std::string Name() const { return "RFM69H (raw)"; }
};

#endif
