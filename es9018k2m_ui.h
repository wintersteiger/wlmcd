// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ES9018K2M_UI_H_
#define _ES9018K2M_UI_H_

#include <memory>

#include "ui.h"
#include "register_table.h"

class ES9018K2M;

class ES9018K2MUI: public UI {
public:
  ES9018K2MUI(std::shared_ptr<ES9018K2M> &es9018k2m);
  virtual ~ES9018K2MUI();

  virtual std::string Name() const { return "ES9018K2M"; }
};

std::shared_ptr<UI> make_es9018k2m_raw_ui(std::shared_ptr<ES9018K2M> &es9018k2m);

#endif