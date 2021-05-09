// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <sstream>
#include <iomanip>

#include "raw_ui.h"
#include "bme680.h"
#include "bme680_rt.h"

#include "bme680_ui.h"

BME680UI::BME680UI(std::shared_ptr<BME680> bme680)
{
  devices.insert(bme680.get());

  size_t row = 1, col = 1;

  std::stringstream namess;
  namess << Name() << " (0x" << std::hex << std::setw(2) << std::setfill('0') << (int)bme680->DeviceAddress() << ")";

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, namess.str()));
  Add(new Empty(row++, col));

  Add(new LField<uint32_t>(UI::statusp, row++, col, 10, "Pressure", "hPa", [bme680]() -> uint32_t {
    return bme680->RT->Pressure() / 100;
  }));

  Add(new LField<uint32_t>(UI::statusp, row++, col, 10, "Temperature", "°C", [bme680]() -> uint32_t {
    return bme680->RT->Temperature();
  }));

  Add(new LField<uint32_t>(UI::statusp, row++, col, 10, "Humidity", "%", [bme680]() -> uint32_t {
    return bme680->RT->Humidity();
  }));

  Add(new LField<uint32_t>(UI::statusp, row++, col, 10, "Gas resistance", "Ohm", [bme680]() -> uint32_t {
    return bme680->RT->GasResistance();
  }));
}

std::shared_ptr<UI> make_bme680_raw_ui(std::shared_ptr<BME680> &device)
{
  return make_raw_ui<BME680, uint8_t, uint8_t>(device, *device->RT);
}
