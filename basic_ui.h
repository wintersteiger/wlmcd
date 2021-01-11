// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _BASIC_UI_H_
#define _BASIC_UI_H_

#include <string>

#include "ui.h"
#include "basic.h"

class BasicUI : public ::UI {
protected:
  const Basic::State &state;

public:
  BasicUI(const Basic::State &state);
  virtual ~BasicUI();

  virtual void Update(bool full);

  virtual std::string Name() const { return "Basic"; }
};

#endif