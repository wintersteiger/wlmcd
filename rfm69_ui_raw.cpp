// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include <wiringPi.h>

#include "rfm69.h"
#include "rfm69_rt.h"
#include "field_types.h"
#include "rfm69_ui_raw.h"

class RFM69RawField : public Field<uint8_t> {
protected:
  const Register<uint8_t, uint8_t> &reg;
  const Variable<uint8_t> *var;
  RFM69::RegisterTable &rt;

public:
  RFM69RawField(int row, const Register<uint8_t, uint8_t> *reg, RFM69::RegisterTable &rt) :
    Field<uint8_t>(UI::statusp, row, 1, reg->Name(), "", ""), reg(*reg), var(NULL), rt(rt) {
      value_width = 2;
      units_width = 0;
      attributes = A_BOLD;
  }
  RFM69RawField(int row, const Register<uint8_t, uint8_t> *reg, const Variable<uint8_t> *var, RFM69::RegisterTable &rt) :
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
  const RFM69 &rfm69;
  Responsiveness(int r, int c, const RFM69 &rfm69)
      : IndicatorField(UI::statusp, row, col, "R"), rfm69(rfm69) {}
  virtual ~Responsiveness() {}
  virtual bool Get() { return rfm69.Responsive(); }
};

#define EMPTY() fields.push_back(new Empty(row++, col));

RFM69UIRaw::RFM69UIRaw(RFM69 &rfm69) :
  UI(),
  rfm69(rfm69)
{
  devices.insert(&rfm69);

  int row = 1, col = 1;

  fields.push_back(new Responsiveness(row++, col, rfm69));
  EMPTY();

  bool first = true;
  for (auto reg : rfm69.RT) {
    if (first) { first = false; /* Skip FIFO */ continue; }
    fields.push_back(new RFM69RawField(row++, reg, rfm69.RT));
    for (auto var : *reg)
      fields.push_back(new RFM69RawField(row++, reg, var, rfm69.RT));
    fields.push_back(new Empty(row++, col));
  }
}

RFM69UIRaw::~RFM69UIRaw() {}
