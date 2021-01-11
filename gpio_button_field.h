// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _GPIO_BUTTON_FIELD_H_
#define _GPIO_BUTTON_FIELD_H_

#include "field.h"
#include "gpio_button.h"

class GPIOButtonField : public IndicatorField {
public:
  GPIOButtonField(WINDOW *wndw, int row, int col, const std::string &key, GPIOButton &button, bool bumpy=false);
  virtual ~GPIOButtonField();

  virtual bool ReadOnly() { return false; }
  virtual bool Get();
  virtual void Set(bool value);
  virtual void Bump();

protected:
  GPIOButton &button;
  bool bumpy, bump_state;
};

#endif // _GPIO_BUTTON_FIELD_H_
