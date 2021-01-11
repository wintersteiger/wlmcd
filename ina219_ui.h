// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _INA219_UI_H_
#define _INA219_UI_H_

#include <cstdint>
#include <vector>

#include "ui.h"
#include "field.h"

class INA219;

class INA219UI : public UI {
public:
  INA219UI(INA219 &ina219);
  virtual ~INA219UI();

  virtual std::string Name() const { return "INA219"; }
  virtual void Layout();
};

#endif // _INA219_UI_RAW_H_
