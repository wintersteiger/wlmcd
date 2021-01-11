// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _GPIO_UI_H_
#define _GPIO_UI_H_

#include "ui.h"

class GPIODevice;

class GPIOUI : public UI {
public:
  GPIOUI(GPIODevice &gpio);
  virtual ~GPIOUI();

  virtual std::string Name() const { return "GPIO"; }
  virtual void Layout();
};

#endif
