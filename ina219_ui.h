// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _INA219_UI_H_
#define _INA219_UI_H_

#include <cstdint>
#include <vector>
#include <memory>

#include "ui.h"

class INA219;

template <typename D, typename AT, typename VT>
class RegisterTable;

class INA219UI : public UI {
public:
  INA219UI(INA219 &ina219);
  virtual ~INA219UI();

  virtual std::string Name() const override { return "INA219"; }
  virtual void Layout() override;
};

std::shared_ptr<UI> make_ina219_raw_ui(std::shared_ptr<INA219> &device);

#endif // _INA219_UI_RAW_H_
