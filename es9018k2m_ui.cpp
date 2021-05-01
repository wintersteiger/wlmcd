// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "field_types.h"
#include "es9018k2m.h"
#include "es9018k2m_rt.h"
#include "es9018k2m_ui.h"

namespace ES9018K2MUIFields {


} // ES9018K2MUIFields

using namespace ES9018K2MUIFields;

#define EMPTY() Add(new Empty(row++, col))

ES9018K2MUI::ES9018K2MUI(ES9018K2M &es9018k2m) : UI()
{
  devices.insert(&es9018k2m);

  int row = 1, col = 1;

  Add(new TimeField(statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  EMPTY();
}

ES9018K2MUI::~ES9018K2MUI() {}
