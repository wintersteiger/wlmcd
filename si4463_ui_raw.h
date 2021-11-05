// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SI4463_UI_RAW_H_
#define _SI4463_UI_RAW_H_

#include <memory>

#include "ui.h"

class SI4463;

class SI4463UIRaw: public UI {
protected:
  std::shared_ptr<SI4463> si4463;

public:
  SI4463UIRaw(std::shared_ptr<SI4463> si4463);
  virtual ~SI4463UIRaw();

  virtual std::string Name() const { return "SI4463 (raw)"; }
};

#endif
