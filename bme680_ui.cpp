// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "raw_ui.h"
#include "bme680.h"
#include "bme680_rt.h"

#include "bme680_ui.h"

std::shared_ptr<UI> make_bme680_raw_ui(std::shared_ptr<BME680> &device)
{
  return make_raw_ui<BME680, uint8_t, uint8_t>(device, *device->RT);
}
