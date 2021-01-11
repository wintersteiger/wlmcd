// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SI4463_UI_RAW_H_
#define _SI4463_UI_RAW_H_

#include <cstdint>
#include <vector>

#include "ui.h"
#include "field.h"

class SI4463UIRaw: public UI {
protected:
  SI4463 &si4463;

public:
  SI4463UIRaw(SI4463 &si4463);
  virtual ~SI4463UIRaw();

  virtual std::string Name() const { return "SI4463 (raw)"; }
};

#endif
