// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include <wiringPi.h>

#include "rfm69h.h"
#include "rfm69h_rt.h"
#include "field_types.h"
#include "rfm69h_ui_raw.h"

class RFM69HRawField : public Field<uint8_t> {
protected:
  const Register<uint8_t, uint8_t> &reg;
  const Variable<uint8_t> *var;
  RFM69H::RegisterTable &rt;

public:
  RFM69HRawField(int row, const Register<uint8_t, uint8_t> *reg, RFM69H::RegisterTable &rt) :
    Field<uint8_t>(UI::statusp, row, 1, reg->Name(), "", ""), reg(*reg), var(NULL), rt(rt) {
      value_width = 2;
      units_width = 0;
      attributes = A_BOLD;
  }
  RFM69HRawField(int row, const Register<uint8_t, uint8_t> *reg, const Variable<uint8_t> *var, RFM69H::RegisterTable &rt) :
    Field<uint8_t>(UI::statusp, row, 1, var->Name(), "", ""), reg(*reg), var(var), rt(rt) {
      value_width = 2;
      units_width = 0;
  }
  virtual size_t Width() { return key.size() + 4; }
  virtual uint8_t Get() {
    uint8_t r = reg(rt.Buffer());
    return var ? (*var)(r) : r;
  }
  virtual void Update(bool full=false) {
    if (wndw) {
      key_width = key.size();
      value_width = 2;
      snprintf(tmp, sizeof(tmp), "%02x", Get());
      value = tmp;
      FieldBase::Update(full);
    }
  }
  virtual bool Activateable() const { return true; }
  virtual bool ReadOnly() { return false; }
  virtual void Set(const char *v_str) {
    uint8_t v = 0;
    size_t v_str_len = strlen(v_str);
    if (v_str_len == 0 || v_str_len > 2 || sscanf(v_str, "%02hhx", &v) != 1)
      UI::Error("invalid value '%s'", v_str);
    else if (var != NULL)
      rt.Write(reg, var->Set(reg(rt.Buffer()), v));
    else
      rt.Write(reg, v);
  }
  virtual std::string Describe() const {
    std::string desc;
    if (var) {
      desc = reg.Name() + "::" + var->Name();
      if (var->Name() != var->NiceName())
        desc += " (" + var->NiceName() + ")";
      desc += "\n\n";
      desc += var->Description();
    }
    else {
      desc = reg.Name();
      if (reg.Name() != reg.NiceName())
        desc += " (" + reg.NiceName() + ")";
      desc += "\n\n";
      desc += reg.Description();
    }
    return desc;
  }
};

class Responsiveness : public IndicatorField {
public:
  const RFM69H &rfm69h;
  Responsiveness(int r, int c, const RFM69H &rfm69h)
      : IndicatorField(UI::statusp, row, col, "R"), rfm69h(rfm69h) {}
  virtual ~Responsiveness() {}
  virtual bool Get() { return rfm69h.Responsive(); }
};

#define EMPTY() fields.push_back(new Empty(row++, col));

RFM69HUIRaw::RFM69HUIRaw(RFM69H &rfm69h) :
  UI(),
  rfm69h(rfm69h)
{
  devices.insert(&rfm69h);

  int row = 1, col = 1;

  fields.push_back(new Responsiveness(row++, col, rfm69h));
  EMPTY();

  bool first = true;
  for (auto reg : rfm69h.RT) {
    if (first) { first = false; /* Skip FIFO */ continue; }
    fields.push_back(new RFM69HRawField(row++, reg, rfm69h.RT));
    for (auto var : *reg)
      fields.push_back(new RFM69HRawField(row++, reg, var, rfm69h.RT));
    fields.push_back(new Empty(row++, col));
  }
}

RFM69HUIRaw::~RFM69HUIRaw() {}
