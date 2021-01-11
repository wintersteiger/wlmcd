// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CC1101_UI_RAW_H_
#define _CC1101_UI_RAW_H_

#include <cstdint>
#include <vector>

#include "ui.h"
#include "field.h"

class CC1101UIRaw: public UI {
public:
  CC1101UIRaw(CC1101 &cc1101);
  virtual ~CC1101UIRaw() {}

  virtual std::string Name() const { return "CC1101 (raw)"; }
};

#endif