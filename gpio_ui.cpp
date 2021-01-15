// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <gpiod.h>

#include "gpio_device.h"
#include "field.h"
#include "gpio_ui.h"

namespace GPIOUIFields {

class Indicator : public ::IndicatorField {
protected:
  gpio_address_t addr;
  GPIODevice &device;

public:
  Indicator(int row, int col, const std::string &key, GPIODevice &device, const gpio_address_t &addr) :
    ::IndicatorField(UI::statusp, row, col, key), addr(addr), device(device) {}
  virtual ~Indicator() {}
  virtual bool Get() = 0;
};

#define IND(N, C)                                                                                                                                \
  class N##Ind : public Indicator                                                                                                                \
  {                                                                                                                                              \
  public:                                                                                                                                        \
    N##Ind(int row, int col, const std::string &key, GPIODevice &device, const gpio_address_t &addr) : Indicator(row, col, key, device, addr) {} \
    virtual ~N##Ind() {}                                                                                                                         \
    virtual bool Get() { return C; }                                                                                                             \
  };

IND(Value, device.LineState(addr).value);
IND(Direction, device.LineState(addr).direction);
IND(ActiveState, device.LineState(addr).active_state);
IND(Used, device.LineState(addr).used);
IND(OpenDrain, device.LineState(addr).open_drain);
IND(OpenSource, device.LineState(addr).open_source);

#define STR(N, C)                                                                                                                                \
  class N##Str : public StringField                                                                                                              \
  {                                                                                                                                              \
  protected:                                                                                                                                     \
    gpio_address_t addr;                                                                                                                         \
    GPIODevice &device;                                                                                                                          \
                                                                                                                                                 \
  public:                                                                                                                                        \
    N##Str(int row, int col, GPIODevice &device, const gpio_address_t &addr) : StringField(UI::statusp, row, col), addr(addr), device(device) {} \
    virtual ~N##Str() {}                                                                                                                         \
    virtual const char *Get() { return C; }                                                                                                      \
  };

STR(Name, device.LineState(addr).name);
STR(Consumer, device.LineState(addr).consumer);

STR(Offset, std::to_string(device.LineState(addr).offset).c_str())

} // GPIOUIFields

using namespace GPIOUIFields;

#define EMPTY() fields.push_back(new Empty(row++, col));

GPIOUI::GPIOUI(GPIODevice &gpio)
{
  int row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  EMPTY();

  row = 4; col = 2;

  for (size_t i = 0; i < gpio.NumChips(); i++)
  {
    auto cs = gpio.ChipState(i);
    Add(new Label(UI::statusp, row++, col, cs.name));
    Add(new Label(UI::statusp, row++, col, cs.label, 0));

    for (size_t j = 0; j < gpio.NumLines(i); j++)
    {
      Add(new DecField(UI::statusp, row, col, 2,
        [&gpio, i, j](void) {
          return gpio.LineState({i, j}).offset; }));

      Add(new CharField(UI::statusp, row, col+3,
        [&gpio, i, j](void) {
          int v = gpio.LineState({i, j}).value;
          return v == 0 ? '0' : v == 1 ? '1' : '?'; }));

      Add(new CharField(UI::statusp, row, col+5,
        [&gpio, i, j](void) {
          return gpio.LineState({i, j}).direction == GPIOD_LINE_DIRECTION_INPUT ? '<' : '>'; }));

      Add(new CharField(UI::statusp, row, col+6,
        [&gpio, i, j](void) {
          return gpio.LineState({i, j}).active_state ? '^' : '_'; }));

      Add(new CharField(UI::statusp, row, col+7,
        [&gpio, i, j](void) {
          return gpio.LineState({i, j}).used ? 'X' : ' '; }, DISABLED_PAIR));

      Add(new CharField(UI::statusp, row, col+8,
        [&gpio, i, j](void) {
          bool od = gpio.LineState({i, j}).open_drain != 0;
          bool os = gpio.LineState({i, j}).open_source != 0;
          return od && os ? 'X' : od ? 'D' : os ? 'S' : '-'; }));

      Add(new ConsumerStr(row, col+10, gpio, {i, j}));

      EMPTY();

      if (row >= 12) {
        row = 4;
        col = col + 10;
      }
    }
    row++; col = 2;
    EMPTY();
  }
}

GPIOUI::~GPIOUI()
{
}

void GPIOUI::Layout()
{
  UI::Layout();
}
