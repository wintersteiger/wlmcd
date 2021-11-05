// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <sstream>
#include <iomanip>

#include "raw_ui.h"
#include "ccs811.h"
#include "ccs811_rt.h"
#include "field.h"

#include "ccs811_ui.h"

CCS811UI::CCS811UI(std::shared_ptr<CCS811> ccs811)
{
  devices.insert(ccs811);

  size_t row = 1, col = 1;

  std::stringstream namess;
  namess << Name() << " (0x" << std::hex << std::setw(2) << std::setfill('0') << (int)ccs811->DeviceAddress() << ")";

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, namess.str()));
  Add(new Empty(row++, col));

  Add(new LEnabledIndicator(UI::statusp, row, col, 0, "APP", [ccs811](){
     auto val = ccs811->RT->FW_MODE();
     return val[0] == 1;
    }));

  Add(new LEnabledIndicator(UI::statusp, row++, col + 5, 0, "VLD", [ccs811](){
    auto val = ccs811->RT->APP_VALID();
    return val[0] == 1;
  }));

  Add(new LEnabledIndicator(UI::statusp, row, col, 0, "RDY", [ccs811](){
     auto val = ccs811->RT->DATA_READY();
     return val[0] == 1;
    }));

  Add(new LWarningIndicator(UI::statusp, row++, col + 5, 0, "ERR", [ccs811](){
    return ccs811->RT->ERROR()[0] != 0;
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Measurement mode"));
  Add(new LField<const char*>(UI::statusp, row++, col, 8, "Drive Mode", "", [ccs811](){
    static const char* vals[] = {"idle", "cnst 1s", "pulse", "low pwr", "cnst 250ms"};
    auto m = ccs811->RT->DRIVE_MODE()[0];
    return m > 4 ? "???" : vals[m];
  }));
  Add(new LSwitch(UI::statusp, row, col, "RDY",
    [ccs811](){ return ccs811->RT->INT_DATARDY()[0]; },
    [ccs811](bool v) { ccs811->RT->Write(ccs811->RT->_rMEAS_MODE, ccs811->RT->_vINT_DATARDY, {v}); }));
  Add(new LSwitch(UI::statusp, row++, col + 4, "THR",
    [ccs811](){ return ccs811->RT->INT_THRESH()[0]; },
    [ccs811](bool v) { ccs811->RT->Write(ccs811->RT->_rMEAS_MODE, ccs811->RT->_vINT_THRESH, {v}); }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Results"));
  Add(new LField<uint16_t>(UI::statusp, row++, col, 8, "eCO2", "", [ccs811](){
    auto val = ccs811->RT->ECO2();
    return val[6] << 8 | val[7];
  }));
  Add(new LField<uint16_t>(UI::statusp, row++, col, 8, "TVOC", "", [ccs811](){
    auto val = ccs811->RT->TVOC();
    return val[6] << 8 | val[7];
  }));
  Add(new LField<uint8_t>(UI::statusp, row++, col, 8, "Status", "", [ccs811](){
    return ccs811->RT->STATUS_()[0];
  }));
  Add(new LField<uint8_t>(UI::statusp, row++, col, 8, "Error ID", "", [ccs811](){
    return ccs811->RT->ERROR_ID_()[0];
  }));
  Add(new LField<uint16_t>(UI::statusp, row++, col, 8, "Raw data", "", [ccs811](){
    auto val = ccs811->RT->RAW_DATA_();
    return val[6] << 8 | val[7];
  }));

  Add(new LField<uint8_t>(UI::statusp, row++, col, 8, "Raw current", "", [ccs811](){
    return ccs811->RT->CURRENT()[0];
  }));
  Add(new LField<uint16_t>(UI::statusp, row++, col, 8, "Raw ADC[9:0]", "", [ccs811](){
    auto val = ccs811->RT->RAW_ADC();
    return val[0] << 8 | val[1];
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Error ID"));

  Add(new LEnabledIndicator(UI::statusp, row, col, 0, "WRG", [ccs811](){
    return ccs811->RT->ERROR_ID()[0] & 0x01;
  }));

  Add(new LEnabledIndicator(UI::statusp, row, col + 4, 0, "RRG", [ccs811](){
    return ccs811->RT->ERROR_ID()[0] & 0x02;
  }));

  Add(new LEnabledIndicator(UI::statusp, row++, col + 8, 0, "MMD", [ccs811](){
    return ccs811->RT->ERROR_ID()[0] & 0x04;
  }));

  Add(new LEnabledIndicator(UI::statusp, row, col, 0, "MXR", [ccs811](){
    return ccs811->RT->ERROR_ID()[0] & 0x08;
  }));

  Add(new LEnabledIndicator(UI::statusp, row, col + 4, 0, "HTR", [ccs811](){
    return ccs811->RT->ERROR_ID()[0] & 0x10;
  }));

  Add(new LEnabledIndicator(UI::statusp, row, col + 8, 0, "HSY", [ccs811](){
    return ccs811->RT->ERROR_ID()[0] & 0x20;
  }));

  // Add(new Label(UI::statusp, row++, col + 18, "Data"));
  // Add(new LField<float>(UI::statusp, row++, col, 10, "Pressure", "hPa", [ccs811]() -> float {
  //   return ccs811->RT->Pressure() / 100.0;
  // }));
  // Add(new LField<float>(UI::statusp, row++, col, 10, "Temperature", "°C ", [ccs811]() -> float {
  //   return ccs811->RT->Temperature();
  // }));
  // Add(new LField<float>(UI::statusp, row++, col, 10, "Humidity  ", "%", [ccs811]() -> float {
  //   return ccs811->RT->Humidity();
  // }));
}

std::shared_ptr<UI> make_ccs811_raw_ui(std::shared_ptr<CCS811> &device)
{
  return make_raw_ui<CCS811, uint8_t>(device, *device->RT);
}
