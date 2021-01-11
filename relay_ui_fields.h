// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RELAY_FIELDS_H_
#define _RELAY_FIELDS_H_

#include "field.h"
#include "relay_device.h"
#include "ui.h"

namespace RelayUIFields {

class Indicator : public ::IndicatorField {
protected:
  size_t addr;
  RelayDevice &device;

public:
  Indicator(int row, int col, RelayDevice &device, size_t addr) :
    ::IndicatorField(UI::statusp, row, col, std::to_string(device.Offsets()[addr])), addr(addr), device(device) {}
  virtual ~Indicator() {}
  virtual bool Get() {
    return device.ReadBuffered(addr);
  }
  virtual bool ReadOnly() { return false; }
  virtual void Set(bool value) {
    device.Write(addr, value);
  }
};

}

#endif // _RELAY_FIELDS_H_
