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

template <typename T>
class LField : public Field<T> {
protected:
  std::function<T(void)> update;

public:
  LField(WINDOW *wndw, int row, int col,
            const std::string &key,
            const std::string &units,
            std::function<T(void)> &&update) :
    Field<T>(wndw, row, col, key, "", units),
    update(update)
  {
    FieldBase::value_width = 10;
  }
  virtual T Get() { return update(); };
  virtual ~LField() = default;
};

ES9018K2MUI::ES9018K2MUI(std::shared_ptr<ES9018K2M> &es9018k2m) : UI()
{
  devices.insert(es9018k2m.get());

  int row = 1, col = 1;

  Add(new TimeField(statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  EMPTY();

  auto& RTS = es9018k2m->RTS;
  Add(new LField<double>(statusp, row++, col, "Volume 1", "dB", [RTS]() { return -0.5 * RTS.Main.Volume1(); } ));
  Add(new LField<double>(statusp, row++, col, "Volume 2", "dB", [RTS]() { return -0.5 * RTS.Main.Volume2(); } ));
  Add(new LField<int32_t>(statusp, row++, col, "Master trim", "", [RTS]() { return RTS.Main.MasterTrim(); } ));

  Add(new LField<const char*>(statusp, row++, col, "Sample freq", "kHz", [RTS]() {
    static const char *values[] = {"?", "48", "44.1", "32"};
    // uint8_t q = RTS.Professional.Status0();
    return values[2];
  }));
}

ES9018K2MUI::~ES9018K2MUI() {}
