// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ES9028PRO_UI_H_
#define _ES9028PRO_UI_H_

#include <memory>

#include "ui.h"
#include "register_table.h"

class ES9028PRO;

class ES9028PROUI: public UI {
public:
  ES9028PROUI(std::shared_ptr<ES9028PRO> &es9028pro);
  virtual ~ES9028PROUI();

  virtual std::string Name() const { return "ES9028PRO"; }
};

std::shared_ptr<UI> make_es9028pro_raw_ui(std::shared_ptr<ES9028PRO> &es9028pro);

#endif