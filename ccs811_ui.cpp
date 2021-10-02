// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <sstream>
#include <iomanip>

#include "raw_ui.h"
#include "ccs811.h"
#include "ccs811_rt.h"

#include "ccs811_ui.h"

// CCS811UI::CCS811UI(std::shared_ptr<CCS811> ccs811)
// {
//   devices.insert(ccs811.get());

//   size_t row = 1, col = 1;

//   std::stringstream namess;
//   namess << Name() << " (0x" << std::hex << std::setw(2) << std::setfill('0') << (int)ccs811->DeviceAddress() << ")";

//   Add(new TimeField(UI::statusp, row, col));
//   Add(new Label(UI::statusp, row++, col + 18, namess.str()));
//   Add(new Empty(row++, col));

//   Add(new Label(UI::statusp, row++, col + 18, "Data"));
//   Add(new LField<float>(UI::statusp, row++, col, 10, "Pressure", "hPa", [ccs811]() -> float {
//     return ccs811->RT->Pressure() / 100.0;
//   }));
//   Add(new LField<float>(UI::statusp, row++, col, 10, "Temperature", "°C ", [ccs811]() -> float {
//     return ccs811->RT->Temperature();
//   }));
//   Add(new LField<float>(UI::statusp, row++, col, 10, "Humidity  ", "%", [ccs811]() -> float {
//     return ccs811->RT->Humidity();
//   }));

//   Add(new Empty(row++, col));

//   Add(new Label(UI::statusp, row++, col + 18, "Oversampling"));
//   const char *ovrsmplng_values[] = { "disabled", "1x", "2x", "4x", "8x", "16x" };

//   Add(new LField<const char*>(UI::statusp, row++, col, 10, "Humidity", "", [ccs811, ovrsmplng_values]() {
//     uint8_t t = ccs811->RT->osrs_h_2_0();
//     t = t <= 5 ? t : 5;
//     return ovrsmplng_values[t];
//   }));

//   Add(new LField<const char*>(UI::statusp, row++, col, 10, "Temperature", "", [ccs811, ovrsmplng_values]() {
//     uint8_t t = ccs811->RT->osrs_t_2_0();
//     t = t <= 5 ? t : 5;
//     return ovrsmplng_values[t];
//   }));

//   Add(new LField<const char*>(UI::statusp, row++, col, 10, "Pressure", "", [ccs811, ovrsmplng_values]() {
//     uint8_t t = ccs811->RT->osrs_p_2_0();
//     t = t <= 5 ? t : 5;
//     return ovrsmplng_values[t];
//   }));

//   Add(new Empty(row++, col));

//   Add(new LField<uint8_t>(UI::statusp, row++, col, 10, "IIR filt coef", "", [ccs811]() {
//     return (1 << ccs811->RT->filter_2_0()) - 1;
//   }));
// }

std::shared_ptr<UI> make_ccs811_raw_ui(std::shared_ptr<CCS811> &device)
{
  return make_raw_ui<CCS811, uint8_t>(device, *device->RT);
}
