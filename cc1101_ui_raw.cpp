// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "field_types.h"
#include "cc1101.h"
#include "cc1101_rt.h"
#include "cc1101_ui_raw.h"

namespace CC1101UIFields {

class RawField : public Field<uint8_t> {
protected:
  const CC1101::RegisterTable::TRegister &reg;
  const Variable<uint8_t> *var;
  CC1101::RegisterTable &rt;

public:
  RawField(int row, const CC1101::RegisterTable::TRegister *reg, CC1101::RegisterTable &rt) :
    Field<uint8_t>(UI::statusp, row, 1, reg->NiceName(), "", ""), reg(*reg), var(NULL), rt(rt) {
      value_width = 2;
      units_width = 0;
      attributes = A_BOLD;
  }
  RawField(int row, const CC1101::RegisterTable::TRegister *reg, const Variable<uint8_t> *var, CC1101::RegisterTable &rt) :
    Field<uint8_t>(UI::statusp, row, 1, var->NiceName(), "", ""), reg(*reg), var(var), rt(rt) {
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
    uint8_t v = 0;
    size_t v_str_len = strlen(v_str);
    if (v_str_len == 0 || v_str_len > 2 || sscanf(v_str, "%02hhx", &v) != 1)
      UI::Error("Invalid value '%s'", v_str);
    else if (var != NULL)
      rt.Write(reg, var->Set(rt(reg), v));
    else
      rt.Write(reg, v);
  }
};

} // CC1101UIFields

using namespace CC1101UIFields;

CC1101UIRaw::CC1101UIRaw(CC1101 &cc1101) : UI()
{
  devices.insert(&cc1101);

  int row = 1, col = 1;
  bool first = true;
  for (auto reg : *cc1101.RT) {
    if (reg->Address() == cc1101.RT->_rFIFO.Address() ||
        reg->Address() == cc1101.RT->_rPATABLE.Address())
      continue;
    fields.push_back(new RawField(row++, reg, *cc1101.RT));
    for (auto var : *reg)
      fields.push_back(new RawField(row++, reg, var, *cc1101.RT));
    fields.push_back(new Empty(row++, col));
  }

  fields.push_back(new Label(UI::statusp, row, col, "PATABLE: "));
  fields.push_back(new HexField(UI::statusp, row++, col + 9, 16, [&cc1101](void){
    uint64_t r = 0;
    for (size_t i = 0; i < 8; i++)
      r = (r << 8) | cc1101.RT->PATableBuffer[i];
    return r;
  }));
}