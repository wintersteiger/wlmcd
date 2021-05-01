// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _INA219_UI_RAW_H_
#define _INA219_UI_RAW_H_

#include "ui.h"

class INA219;

class INA219UIRaw : public UI {
public:
  INA219UIRaw(INA219 &ina219);
  virtual ~INA219UIRaw();

  virtual std::string Name() const { return "INA219 (raw)"; }
};

#endif // _INA219_UI_RAW_H_
