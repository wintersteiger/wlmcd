// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CC1101_UI_H_
#define _CC1101_UI_H_

#include "ui.h"

class CC1101;

class CC1101UI: public UI {
public:
  CC1101UI(CC1101 &cc1101);
  virtual ~CC1101UI() {}

  virtual std::string Name() const { return "CC1101"; }
  virtual void Layout();
};

#endif