// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <memory>

#include "raw_ui.h"
#include "spirit1.h"
#include "spirit1_rt.h"
#include "gpio_button.h"
#include "gpio_button_field.h"

#include <spirit1_ui.h>

SPIRIT1UI::SPIRIT1UI(std::shared_ptr<SPIRIT1> spirit1)
{
  devices.insert(spirit1);
}

SPIRIT1UI::~SPIRIT1UI() {}

std::shared_ptr<UI> make_spirit1_raw_ui(std::shared_ptr<SPIRIT1> &device, std::shared_ptr<GPIOButton> reset_button)
{
  auto ui = make_raw_ui<SPIRIT1, uint8_t, uint8_t>(device, *device->RT);

  if (reset_button)
    ui->Add(new GPIOButtonField(UI::statusp, 0, 0, "RESET", *reset_button));

  return ui;
}
