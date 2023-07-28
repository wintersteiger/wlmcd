// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _S2LP_UI_H_
#define _S2LP_UI_H_

#include <memory>

#include "ui.h"

class S2LP;
class GPIOButton;

class S2LPUI : public UI {
public:
  S2LPUI(std::shared_ptr<S2LP> s2lp, const uint32_t &irqs = 0);
  virtual ~S2LPUI();

  virtual std::string Name() const override { return "S2LP"; }
  virtual void Layout() override;
};

std::shared_ptr<UI> make_s2lp_raw_ui(std::shared_ptr<S2LP> &device, std::shared_ptr<GPIOButton> reset_button = nullptr);

#endif // _S2LP_UI_H_
