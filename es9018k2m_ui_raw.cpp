// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "field_types.h"
#include "es9018k2m.h"
#include "es9018k2m_rt.h"
#include "es9018k2m_ui_raw.h"

namespace ES9018K2MUIRawFields {

template <typename AT, typename VT, typename D>
class RawField : public FieldBase {
protected:
  const Register<uint8_t, uint8_t> &reg;
  const Variable<uint8_t> *var;
  RegisterTable<AT, VT, D> &rt;

public:
  RawField(int row, const Register<AT, VT> *reg, RegisterTable<AT, VT, D> &rt) :
    FieldBase(UI::statusp, row, 1, reg->NiceName(), "", ""), reg(*reg), var(NULL), rt(rt) {
      value_width = 2;
      units_width = 0;
      attributes = A_BOLD;
  }
  RawField(int row, const Register<AT, VT> *reg, const Variable<VT> *var, RegisterTable<AT, VT, D> &rt) :
    FieldBase(UI::statusp, row, 1, var->NiceName(), "", ""), reg(*reg), var(var), rt(rt) {
      value_width = 2;
      units_width = 0;
  }
  virtual size_t Width() { return key.size() + 4; }
  virtual VT Get() {
    VT r = reg(rt.Buffer());
    return var ? (*var)(r) : r;
  }
  virtual void Update(bool full=false) {
    if (wndw) {
      key_width = key.size();
      value_width = 2;
      const char *fmt = var ? "%x" : "%02x";
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
    VT v = 0;
    size_t v_str_len = strlen(v_str);
    if (v_str_len == 0 || v_str_len > 2 || sscanf(v_str, "%02hhx", &v) != 1)
      UI::Error("invalid value '%s'", v_str);
    else if (var != NULL)
      rt.Device().Write(reg, var->Set(reg(rt.Buffer()), v));
    else
      rt.Device().Write(reg, v);
  }
};

} // ES9018K2MUIRawFields

using namespace ES9018K2MUIRawFields;

ES9018K2MUIRaw::ES9018K2MUIRaw(std::shared_ptr<ES9018K2M> &es9018k2m) : UI()
{
  devices.insert(es9018k2m.get());

  typedef RawField<uint8_t, uint8_t, ES9018K2M> RF;

  int row = 1, col = 1;
  for (auto reg : es9018k2m->RTS.Main) {
    fields.push_back(new RF(row++, reg, es9018k2m->RTS.Main));
    for (auto var : *reg)
      fields.push_back(new RF(row++, reg, var, es9018k2m->RTS.Main));
    fields.push_back(new Empty(row++, col));
  }
}

ES9018K2MUIRaw::~ES9018K2MUIRaw() {}
