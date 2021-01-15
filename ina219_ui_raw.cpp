// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "field_types.h"
#include "ina219.h"
#include "ina219_rt.h"
#include "ina219_ui_raw.h"

namespace INA219UIFields {

class RawField : public Field<uint16_t> {
protected:
  const Register<uint8_t, uint16_t> &reg;
  const Variable<uint16_t> *var;
  INA219::RegisterTable &rt;

public:
  RawField(int row, const Register<uint8_t, uint16_t> *reg, INA219::RegisterTable &rt) :
    Field<uint16_t>(UI::statusp, row, 1, reg->NiceName(), "", ""), reg(*reg), var(NULL), rt(rt) {
      value_width = 2;
      units_width = 0;
      attributes = A_BOLD;
  }
  RawField(int row, const Register<uint8_t, uint16_t> *reg, const Variable<uint16_t> *var, INA219::RegisterTable &rt) :
    Field<uint16_t>(UI::statusp, row, 1, var->NiceName(), "", ""), reg(*reg), var(var), rt(rt) {
      value_width = 2;
      units_width = 0;
  }
  virtual size_t Width() { return key.size() + 4; }
  virtual uint16_t Get() {
    uint16_t r = reg(rt.Buffer());
    return var ? (*var)(r) : r;
  }
  virtual void Update(bool full=false) {
    if (wndw) {
      key_width = key.size();
      value_width = 2;
      const char *fmt = var ? "%x" : "%04x";
      snprintf(tmp, sizeof(tmp), fmt, Get());
      value = tmp;
      FieldBase::Update(full);
    }
  }
  virtual void Active(bool active) {
    this->active = active;
    if (active) {
      const char* name = var ? var->NiceName().c_str() : reg.NiceName().c_str();
      const char* desc = var ? var->Description().c_str() : reg.Description().c_str();
      char fmt[UI::ScreenWidth() + 1 - 3];
      size_t total_width = 6 + strlen(name) + strlen(desc) + 1;
      if (total_width <= UI::ScreenWidth())
        snprintf(fmt, sizeof(fmt), "%%s: %%.%ds...", UI::ScreenWidth() - 6);
      else
        snprintf(fmt, sizeof(fmt), "%%s: %%.%ds", UI::ScreenWidth() - 3);
      UI::Info(fmt, name, desc);
    }
  }
  virtual std::string Describe() const {
    std::string description;
    if (var) {
      std::string name = var->Name();
      std::string nice_name = var->NiceName();
      description += nice_name;
      if (name != nice_name)
        description += std::string(" (") + name + ")";
      description += std::string(":\n\n");
      description += var->Description();
    }
    else {
      std::string name = reg.Name();
      std::string nice_name = reg.NiceName();
      description += nice_name;
      if (name != nice_name)
        description += std::string(" (") + name + ")";
      description += std::string(":\n\n");
      description += reg.Description();

      if (reg.begin() != reg.end()) {
        description += "\n\nVariables:";
        for (auto var : reg) {
          description += std::string("\n");
          description += var->Name();
        }
      }
    }
    return description;
  }
  virtual bool Activateable() const { return true; }
  virtual bool ReadOnly() { return !(var ? var->Writeable() : reg.Writeable()); }
  virtual void Set(const char *v_str) {
    uint16_t v = 0;
    size_t v_str_len = strlen(v_str);
    if (v_str_len == 0 || v_str_len > 2 || sscanf(v_str, "%04hx", &v) != 1)
      UI::Error("invalid value '%s'", v_str);
    else if (var != NULL)
      rt.Write(reg, var->Set(reg(rt.Buffer()), v));
    else
      rt.Write(reg, v);
  }
};

} // INA219UIFields

using namespace INA219UIFields;

INA219UIRaw::INA219UIRaw(INA219 &ina219) : UI()
{
  devices.insert(&ina219);

  int row = 1, col = 1;
  bool first = true;
  for (auto reg : ina219.RT) {
    fields.push_back(new RawField(row++, reg, ina219.RT));
    for (auto var : *reg)
      fields.push_back(new RawField(row++, reg, var, ina219.RT));
    fields.push_back(new Empty(row++, col));
  }
}

INA219UIRaw::~INA219UIRaw() {}
