// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SX1278_UI_H_
#define _SX1278_UI_H_

#include <cstdint>
#include <vector>

#include "ui.h"
#include "sx1278.h"

class SX1278UI: public UI {
protected:
  SX1278 &sx1278;
  size_t num_status_fields;

public:
  SX1278UI(SX1278 &sx1278);
  virtual ~SX1278UI();

  virtual std::string Name() const { return "SX1278"; }
  virtual void Layout();
};

#endif
