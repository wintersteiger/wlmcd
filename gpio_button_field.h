// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _GPIO_BUTTON_FIELD_H_
#define _GPIO_BUTTON_FIELD_H_

#include <memory>

#include "field.h"
#include "gpio_button.h"

class GPIOButtonField : public IndicatorField {
public:
  GPIOButtonField(WINDOW *wndw, int row, int col, const std::string &key, std::shared_ptr<GPIOButton> button, bool bumpy=false);
  virtual ~GPIOButtonField();

  virtual bool ReadOnly() const override { return false; }
  virtual bool Get() override;
  virtual void Set(bool value) override;
  virtual void Flip() override;
  virtual bool Flippable() const override { return true; }

protected:
  std::shared_ptr<GPIOButton> button;
  bool bumpy, bump_state;
};

#endif // _GPIO_BUTTON_FIELD_H_
