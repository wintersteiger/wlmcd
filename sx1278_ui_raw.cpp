// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "sx1278.h"
#include "sx1278_rt.h"
#include "field_types.h"
#include "sx1278_ui_raw.h"

class SX1278RawField : public Field<uint8_t> {
protected:
  const SX1278::NormalRegisterTable::TRegister &reg;
  const Variable<uint8_t> *var;
  SX1278::NormalRegisterTable &rt;

public:
  SX1278RawField(int row, const SX1278::NormalRegisterTable::TRegister *reg, SX1278::NormalRegisterTable &rt) :
    Field<uint8_t>(UI::statusp, row, 1, reg->Name(), "", ""), reg(*reg), var(NULL), rt(rt) {
      value_width = 2;
      units_width = 0;
      attributes = A_BOLD;
  }
  SX1278RawField(int row, const SX1278::NormalRegisterTable::TRegister *reg, const Variable<uint8_t> *var, SX1278::NormalRegisterTable &rt) :
    Field<uint8_t>(UI::statusp, row, 1, var->Name(), "", ""), reg(*reg), var(var), rt(rt) {
      value_width = 2;
      units_width = 0;
  }
  virtual size_t Width() { return key.size() + 4; }
  virtual uint8_t Get() {
    uint8_t r = rt(reg);
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
      UI::Error("Invalid value '%s'", v_str);
    else if (var != NULL)
      rt.Write(reg, var->Set(rt(reg), v));
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

#define EMPTY() fields.push_back(new Empty(row++, col));

SX1278UIRaw::SX1278UIRaw(SX1278 &sx1278) :
  UI(),
  sx1278(sx1278)
{
  devices.insert(&sx1278);

  int row = 1, col = 1;
  bool first = true;
  for (auto reg : sx1278.Normal) {
    if (first) { first = false; continue; }
    fields.push_back(new SX1278RawField(row++, reg, sx1278.Normal));
    for (auto var : *reg)
      fields.push_back(new SX1278RawField(row++, reg, var, sx1278.Normal));
    fields.push_back(new Empty(row++, col));
  }
}

SX1278UIRaw::~SX1278UIRaw() {}
