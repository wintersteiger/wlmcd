// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SX1278_UI_RAW_H_
#define _SX1278_UI_RAW_H_

#include "ui.h"

class SX1278;

class SX1278UIRaw: public UI {
protected:
  std::shared_ptr<SX1278> sx1278;

public:
  SX1278UIRaw(std::shared_ptr<SX1278> sx1278);
  virtual ~SX1278UIRaw();

  virtual std::string Name() const { return "SX1278 (raw)"; }
};

#endif
