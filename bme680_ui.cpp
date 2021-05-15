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

  Add(new Label(UI::statusp, row++, col + 18, "Data"));
  Add(new LField<float>(UI::statusp, row++, col, 10, "Pressure", "hPa", [bme680]() -> float {
    return bme680->RT->Pressure() / 100.0;
  }));
  Add(new LField<float>(UI::statusp, row++, col, 10, "Temperature", "°C ", [bme680]() -> float {
    return bme680->RT->Temperature();
  }));
  Add(new LField<float>(UI::statusp, row++, col, 10, "Humidity  ", "%", [bme680]() -> float {
    return bme680->RT->Humidity();
  }));
  Add(new LField<uint32_t>(UI::statusp, row++, col, 10, "Gas resistance", "Ohm", [bme680]() -> uint32_t {
    return bme680->RT->GasResistance();
  }));

  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Oversampling"));
  const char *ovrsmplng_values[] = { "disabled", "1x", "2x", "4x", "8x", "16x" };

  Add(new LField<const char*>(UI::statusp, row++, col, 10, "Humidity", "", [bme680, ovrsmplng_values]() {
    uint8_t t = bme680->RT->osrs_h_2_0();
    t = t <= 5 ? t : 5;
    return ovrsmplng_values[t];
  }));

  Add(new LField<const char*>(UI::statusp, row++, col, 10, "Temperature", "", [bme680, ovrsmplng_values]() {
    uint8_t t = bme680->RT->osrs_t_2_0();
    t = t <= 5 ? t : 5;
    return ovrsmplng_values[t];
  }));

  Add(new LField<const char*>(UI::statusp, row++, col, 10, "Pressure", "", [bme680, ovrsmplng_values]() {
    uint8_t t = bme680->RT->osrs_p_2_0();
    t = t <= 5 ? t : 5;
    return ovrsmplng_values[t];
  }));

  Add(new Empty(row++, col));

  Add(new LField<uint8_t>(UI::statusp, row++, col, 10, "IIR filt coef", "", [bme680]() {
    return (1 << bme680->RT->filter_2_0()) - 1;
  }));

  Add(new Empty(row++, col));
  Add(new Label(UI::statusp, row++, col, "Heater profile"));

  #define GAS_SET_POINT(X)  Add(new LField<std::string>(UI::statusp, row++, col, 12, "Set point "#X , "", [bme680]() { \
    static constexpr uint16_t factors[] = {1, 4, 16, 64}; \
    uint8_t val = bme680->RT->Gas_wait_##X(); \
    if (val == 0) return std::string(12, '-'); else { \
      std::stringstream ss; \
      ss << std::fixed << std::setw(3) << std::setprecision(0) << bme680->RT->HeaterSetPointTemperature(bme680->RT->Res_heat_##X(), 25) << "°C " ; \
      ss << std::setw(4) << (int)(factors[val >> 6] * (val & 0x3F)) << "ms"; \
      return ss.str(); \
    } \
  }));

  TEN(GAS_SET_POINT);

  Add(new LField<uint8_t>(UI::statusp, row++, col, 12, "Selected", "", [bme680]() {
    return bme680->RT->nb_conv_3_0();
  }));
}

std::shared_ptr<UI> make_bme680_raw_ui(std::shared_ptr<BME680> &device)
{
  return make_raw_ui<BME680, uint8_t, uint8_t>(device, *device->RT);
}
