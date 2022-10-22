// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _DS18B20_UI_H_
#define _DS18B20_UI_H_

#include "ui.h"
#include "ds18b20.h"

class DS18B20UI : public UI {
public:
  DS18B20UI(std::shared_ptr<DS18B20> ds18b20);
  virtual ~DS18B20UI();

  virtual std::string Name() const { return "DS18B20"; }
  virtual void Layout();
};

#endif // _DS18B20_UI_H_
