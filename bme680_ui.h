// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _BME680_UI_RAW_H_
#define _BME680_UI_RAW_H_

#include <memory>

#include "ui.h"
#include "register_table.h"

class BME680;

std::shared_ptr<UI> make_bme680_raw_ui(std::shared_ptr<BME680> &device);

#endif