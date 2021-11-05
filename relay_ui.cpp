// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "relay_ui.h"
#include "relay_ui_fields.h"

using namespace RelayUIFields;

#define EMPTY() fields.push_back(new Empty(row++, col));

RelayUI::RelayUI(std::shared_ptr<RelayDevice> relay) : UI()
{
  devices.insert(relay);

  size_t row = 1, col = 1;

  Add(new TimeField(statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  EMPTY();

  for (size_t i=0; i < relay->Offsets().size(); i++) {
    Indicator *indicator_field = new Indicator(row, col, *relay, i);
    fields.push_back(indicator_field);
    col += indicator_field->Key().size() + 1;
  }
}

RelayUI::~RelayUI()
{
}

void RelayUI::Layout()
{
  UI::Layout();
}
