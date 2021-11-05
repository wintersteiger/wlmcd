// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CCS811_UI_H_
#define _CCS811_UI_H_

#include <memory>

#include "ui.h"

class CCS811;

class CCS811UI : public UI {
public:
  CCS811UI(std::shared_ptr<CCS811> ccs811);
  virtual ~CCS811UI() = default;

  virtual std::string Name() const override { return "CCS811"; }
};

std::shared_ptr<UI> make_ccs811_raw_ui(std::shared_ptr<CCS811> &device);

#endif // _CCS811_UI_H_