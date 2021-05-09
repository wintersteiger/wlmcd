// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <sstream>
#include <iomanip>

#include "field_types.h"
#include "ina219.h"
#include "ina219_rt.h"
#include "ina219_ui.h"
#include "ina219_fields.h"
#include "raw_ui.h"

using namespace INA219UIFields;

#define EMPTY() Add(new Empty(row++, col))

INA219UI::INA219UI(INA219 &ina219) : UI()
{
  devices.insert(&ina219);

  size_t row = 1, col = 1;

  std::stringstream namess;
  namess << Name() << " (0x" << std::hex << std::setw(2) << std::setfill('0') << (int)ina219.DeviceAddress() << ")";

  Add(new TimeField(statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, namess.str()));
  EMPTY();

  Add(new OpMode(row++, col, ina219));
  Add(new BusVoltage(row++, col, ina219));
  Add(new ShuntVoltage(row++, col, ina219));
  Add(new Current(row++, col, ina219));
  Add(new Power(row++, col, ina219));
  EMPTY();

  Add(new BusVoltageRange(row++, col, ina219));
  Add(new PGAGain(row++, col, ina219));
  Add(new PGARange(row++, col, ina219));
}

INA219UI::~INA219UI() {}

void INA219UI::Layout() {
  UI::Layout();
}

std::shared_ptr<UI> make_ina219_raw_ui(std::shared_ptr<INA219> &device)
{
  return make_raw_ui<INA219, uint8_t, uint16_t>(device, device->RT);
}
