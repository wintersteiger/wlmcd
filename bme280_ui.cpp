// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <sstream>
#include <iomanip>

#include "raw_ui.h"
#include "bme280.h"
#include "bme280_rt.h"

#include "bme280_ui.h"

BME280UI::BME280UI(std::shared_ptr<BME280> bme280)
{
  devices.insert(bme280);

  size_t row = 1, col = 1;

  std::stringstream namess;
  namess << Name() << " (0x" << std::hex << std::setw(2) << std::setfill('0') << (int)bme280->DeviceAddress() << ")";

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, namess.str()));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Data"));
  Add(new LField<float>(UI::statusp, row++, col, 10, "Pressure", "hPa", [bme280]() -> float {
    return bme280->RT->Pressure() / 100.0;
  }));
  Add(new LField<float>(UI::statusp, row++, col, 10, "Temperature", "°C ", [bme280]() -> float {
    return bme280->RT->Temperature();
  }));
  Add(new LField<float>(UI::statusp, row++, col, 10, "Humidity  ", "%", [bme280]() -> float {
    return bme280->RT->Humidity();
  }));

  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Oversampling"));
  const char *ovrsmplng_values[] = { "disabled", "1x", "2x", "4x", "8x", "16x" };

  Add(new LField<const char*>(UI::statusp, row++, col, 10, "Humidity", "", [bme280, ovrsmplng_values]() {
    uint8_t t = bme280->RT->osrs_h_2_0();
    t = t <= 5 ? t : 5;
    return ovrsmplng_values[t];
  }));

  Add(new LField<const char*>(UI::statusp, row++, col, 10, "Temperature", "", [bme280, ovrsmplng_values]() {
    uint8_t t = bme280->RT->osrs_t_2_0();
    t = t <= 5 ? t : 5;
    return ovrsmplng_values[t];
  }));

  Add(new LField<const char*>(UI::statusp, row++, col, 10, "Pressure", "", [bme280, ovrsmplng_values]() {
    uint8_t t = bme280->RT->osrs_p_2_0();
    t = t <= 5 ? t : 5;
    return ovrsmplng_values[t];
  }));

  Add(new Empty(row++, col));

  Add(new LField<uint8_t>(UI::statusp, row++, col, 10, "IIR filt coef", "", [bme280]() {
    return (1 << bme280->RT->filter_2_0()) - 1;
  }));
}

std::shared_ptr<UI> make_bme280_raw_ui(std::shared_ptr<BME280> &device)
{
  return make_raw_ui<BME280, uint8_t, uint8_t>(device, *device->RT);
}
