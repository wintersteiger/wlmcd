// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SPIRIT1_UI_H_
#define _SPIRIT1_UI_H_

#include <memory>

#include "ui.h"

class SPIRIT1;
class GPIOButton;

class SPIRIT1UI : public UI {
public:
  SPIRIT1UI(std::shared_ptr<SPIRIT1> spirit1);
  virtual ~SPIRIT1UI();

  virtual std::string Name() const { return "SPIRIT1"; }
};

std::shared_ptr<UI> make_spirit1_raw_ui(std::shared_ptr<SPIRIT1> &device, std::shared_ptr<GPIOButton> reset_button = nullptr);

#endif // _SPIRIT1_UI_H_
