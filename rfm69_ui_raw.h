// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RFM69_UI_RAW_H_
#define _RFM69_UI_RAW_H_

#include <cstdint>
#include <vector>
#include <memory>

#include "ui.h"
#include "field.h"

class RFM69UIRaw: public UI {
protected:
  std::shared_ptr<RFM69> rfm69;

public:
  RFM69UIRaw(std::shared_ptr<RFM69> rfm69);
  virtual ~RFM69UIRaw();

  virtual std::string Name() const { return "RFM69 (raw)"; }
};

#endif
