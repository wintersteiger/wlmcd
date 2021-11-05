// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SX1278_UI_H_
#define _SX1278_UI_H_

#include <memory>

#include "ui.h"

class SX1278;

class SX1278UI: public UI {
protected:
  std::shared_ptr<SX1278> sx1278;
  size_t num_status_fields;

public:
  SX1278UI(std::shared_ptr<SX1278> sx1278);
  virtual ~SX1278UI();

  virtual std::string Name() const { return "SX1278"; }
  virtual void Layout();
};

#endif
