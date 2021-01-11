// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SX1278_UI_RAW_H_
#define _SX1278_UI_RAW_H_

#include <cstdint>
#include <vector>

#include "ui.h"
#include "field.h"

class SX1278UIRaw: public UI {
protected:
  SX1278 &sx1278;

public:
  SX1278UIRaw(SX1278 &sx1278);
  virtual ~SX1278UIRaw();

  virtual std::string Name() const { return "SX1278 (raw)"; }
};

#endif
