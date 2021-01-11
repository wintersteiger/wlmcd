// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _DHT22_UI_H_
#define _DHT22_UI_H_

#include <cstdint>
#include <vector>

#include "ui.h"
#include "field.h"

class DHT22;

class DHT22UI : public UI {
public:
  DHT22UI(DHT22 &DHT22);
  virtual ~DHT22UI();

  virtual std::string Name() const { return "DHT22"; }
  virtual void Layout();
};

#endif // _DHT22_UI_RAW_H_
