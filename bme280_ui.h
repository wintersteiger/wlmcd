// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _BME280_UI_RAW_H_
#define _BME280_UI_RAW_H_

#include <memory>

#include "ui.h"
#include "register_table.h"

class BME280;

class BME280UI : public UI {
public:
  BME280UI(std::shared_ptr<BME280> bme280);
  virtual ~BME280UI() = default;

  virtual std::string Name() const override { return "BME280"; }
};

std::shared_ptr<UI> make_bme280_raw_ui(std::shared_ptr<BME280> &device);

#endif