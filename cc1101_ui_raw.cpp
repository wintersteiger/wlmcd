// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "field_types.h"
#include "raw_ui.h"

#include "cc1101.h"
#include "cc1101_rt.h"

std::shared_ptr<UI> make_cc1101_raw_ui(std::shared_ptr<CC1101> &cc1101)
{
  auto ui = std::make_shared<UI>();

  ui->Add(std::static_pointer_cast<DeviceBase>(cc1101));

  typedef RawField<RegisterTable<uint8_t, uint8_t, CC1101>, uint8_t, CC1101> RF;

  int row = 1, col = 1;
  for (auto reg : *cc1101->RT) {
   if (reg->Address() != cc1101->RT->_rFIFO.Address() ||
       reg->Address() != cc1101->RT->_rPATABLE.Address()) {
      ui->Add(new RF(row++, reg, *cc1101->RT));
      for (auto var : *reg)
        ui->Add(new RF(row++, reg, var, *cc1101->RT));
      ui->Add(new Empty(row++, col));
    }
  }

  ui->Add(new Label(UI::statusp, row, col, "PATABLE: "));
  ui->Add(new HexField(UI::statusp, row++, col + 9, 16, [&cc1101](void){
    uint64_t r = 0;
    for (size_t i = 0; i < 8; i++)
      r = (r << 8) | cc1101->RT->PATableBuffer[i];
    return r;
  }));

  return ui;
}