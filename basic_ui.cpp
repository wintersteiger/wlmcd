// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "basic.h"
#include "basic_ui.h"

BasicUI::BasicUI(const Basic::State &state) :
  ::UI(),
  state(state)
{
  size_t row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  fields.push_back(new Empty(row++, col));
}

BasicUI::~BasicUI() {}

void BasicUI::Update(bool full)
{
  ::UI::Update(full);
}
