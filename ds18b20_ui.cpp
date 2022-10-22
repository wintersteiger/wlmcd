// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "ds18b20_ui.h"

DOUBLE_FIELD(UI::statusp, Temperature, "Temperature", DS18B20, "C", device.Temperature());

#define EMPTY() fields.push_back(new Empty(row++, col));

DS18B20UI::DS18B20UI(std::shared_ptr<DS18B20> ds18b20)
{
  devices.insert(ds18b20);

  size_t row = 1, col = 1;

  Add(new TimeField(statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  EMPTY();

  fields.push_back(new Temperature(row, col, *ds18b20));
}

DS18B20UI::~DS18B20UI()
{
}

void DS18B20UI::Layout()
{
  UI::Layout();
}
