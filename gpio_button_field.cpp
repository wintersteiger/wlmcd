// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <unistd.h>

#include "sleep.h"
#include "gpio_button_field.h"

GPIOButtonField::GPIOButtonField(WINDOW *wndw, int row, int col, const std::string &key, std::shared_ptr<GPIOButton> button, bool bumpy) :
  IndicatorField(wndw, row, col, key),
  button(button),
  bumpy(bumpy),
  bump_state(false)
{}

GPIOButtonField::~GPIOButtonField() {}

bool GPIOButtonField::Get()
{
  if (bumpy)
    return bump_state;
  else
    return button->ReadBuffered();
}

void GPIOButtonField::Set(bool value)
{
  button->Write(value);
  if (bumpy)
    bump_state = value;
}

void GPIOButtonField::Flip()
{
  if (bumpy) {
    bool val = button->ReadBuffered();
    button->Write(!val);
    sleep_ms(50);
    button->Write(val);
    bump_state = !bump_state;
  }
  else {
    Set(!Get());
  }
}
