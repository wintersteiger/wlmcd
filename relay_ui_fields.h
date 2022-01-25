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
  virtual bool ReadOnly() const override { return false; }
  virtual void Set(bool value) {
    device.Write(addr, value);
  }
  virtual void Bump() {
    device.Write(addr, !Get());
  }
};


class Selector : public ::IndicatorField {
protected:
  size_t addr;
  RelayDevice &device;
  std::string left, right;

public:
  Selector(int row, int col, RelayDevice &device, size_t addr, const std::string &left, const std::string &right) :
    ::IndicatorField(UI::statusp, row, col, ""), addr(addr), device(device), left(left), right(right) {
      key_width = 0;
      units_width =0;
      value = "**  ";
      value_width = value.size();
    }
  virtual ~Selector() {}
  virtual bool Get() override {
    return device.ReadBuffered(addr);
  }
  virtual bool ReadOnly() const override { return false; }
  virtual void Set(bool value) override {
    device.Write(addr, value);
  }
  virtual void Bump() {
    Set(!Get());
  }
  virtual void Update(bool full) override
  {
    if (wndw) {
      if (attributes != -1) wattron(wndw, attributes);
      if (full) {
        if (active) wattron(wndw, A_STANDOUT);
        snprintf(tmp, sizeof(tmp), "%%- %ds", left.size());
        mvwprintw(wndw, row, col, tmp, left.c_str());
        if (active) wattroff(wndw, A_STANDOUT);
      }

      if (colors != -1) wattron(wndw, COLOR_PAIR(colors));
      value = Get() ? "**  " : "  **";
      mvwprintw(wndw, row, col + key_width + 2, " %s ", value.c_str());
      if (colors != -1) wattroff(wndw, COLOR_PAIR(colors));

      if (full) {
        if (active) wattron(wndw, A_STANDOUT);
        snprintf(tmp, sizeof(tmp), "%%- %ds", right.size());
        mvwprintw(wndw, row, col, tmp, right.c_str());
        if (active) wattroff(wndw, A_STANDOUT);
      }
      if (attributes != -1) wattroff(wndw, attributes);
    }
  }
};


}

#endif // _RELAY_FIELDS_H_
