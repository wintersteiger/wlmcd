// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "field_types.h"
#include "raw_ui.h"
#include "es9018k2m.h"
#include "es9018k2m_rt.h"
#include "es9018k2m_ui.h"

ES9018K2MUI::ES9018K2MUI(std::shared_ptr<ES9018K2M> &es9018k2m) : UI()
{
  devices.insert(es9018k2m);

  int row = 1, col = 1;

  Add(new TimeField(statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  Add(new Empty(row++, col));

  auto* RTS = es9018k2m->RTS;
  Add(new LField<double>(statusp, row++, col, 10, "Volume 1", "dB", [RTS]() { return -0.5 * RTS->Main.Volume1(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Volume 2", "dB", [RTS]() { return -0.5 * RTS->Main.Volume2(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Master trim", "%", [RTS]() {
    return (RTS->Main.MasterTrim() / (double)0x7FFFFFFF) * 100.0;
  }));
  Add(new Empty(row++, col));

  Add(new LField<const char*>(statusp, row++, col, 10, "Input", "", [RTS]() {
    static const char *values[] = { "I2S", "S/PDIF", "?", "DSD" };
    return values[RTS->Main.INPUT_SELECT()];
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "Auto input", "", [RTS]() {
    static const char *values[] = { "None", "I2S|DSD", "I2S|S/PDIF", "I2S|S/PDIF|DSD" };
    return values[RTS->Main.AUTO_INPUT_SELECT()];
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "I2S mode", "", [RTS]() {
    static const char *values[] = { "I2S", "Left-just." };
    return values[RTS->Main.I2S_MODE() & 0x01];
  }));

  Add(new LField<const char*>(statusp, row++, col, 10, "Bit width", "bit", [RTS]() {
    static const char *values[] = { "16", "24", "32", "32" };
    return values[RTS->Main.I2S_LENGTH()];
  }));

  Add(new LField<double>(statusp, row++, col, 10, "Sample rate", "kHz", [es9018k2m]() {
    return es9018k2m->SampleRate() / 1e3;
  }));
}

ES9018K2MUI::~ES9018K2MUI() {}

std::shared_ptr<UI> make_es9018k2m_raw_ui(std::shared_ptr<ES9018K2M> &es9018k2m)
{
  return make_raw_ui<ES9018K2M, uint8_t, uint8_t>(es9018k2m, es9018k2m->RTS->Main);
}