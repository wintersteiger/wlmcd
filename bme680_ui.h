// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _BME680_UI_RAW_H_
#define _BME680_UI_RAW_H_

#include <memory>

#include "ui.h"
#include "register_table.h"

class BME680;

class BME680UI : public UI {
public:
  BME680UI(std::shared_ptr<BME680> bme680);
  virtual ~BME680UI() = default;

  virtual std::string Name() const override { return "BME680"; }
};

std::shared_ptr<UI> make_bme680_raw_ui(std::shared_ptr<BME680> &device);

#endif