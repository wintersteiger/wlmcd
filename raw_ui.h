// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RAW_UI_H_
#define _RAW_UI_H_

#include <memory>
#include <cstring>

#include "field.h"
#include "register.h"
#include "register_table.h"
#include "ui.h"

template <typename VT>
struct ValueParser {
  virtual bool Parse(const char *v_str, VT &value);
};

template <typename VT>
struct ValueFormatter {
  virtual void Format(const VT& value, std::string &formatted);
};

template <typename TT, typename VT, typename D>
class RawField : public FieldBase, ValueParser<VT>, ValueFormatter<VT> {
protected:
  const typename TT::TRegister &reg;
  const Variable<VT> *var;
  TT &rt;

  using ValueParser<VT>::Parse;
  using ValueFormatter<VT>::Format;

public:
  RawField(int row, const typename TT::TRegister *reg, TT &rt) :
    FieldBase(UI::statusp, row, 1, reg->NiceName(), "", ""), reg(*reg), var(NULL), rt(rt) {
      value_width = 2 * reg->value_size();
      units_width = 0;
      attributes = A_BOLD;
  }
  RawField(int row, const typename TT::TRegister *reg, const Variable<VT> *var, TT &rt) :
    FieldBase(UI::statusp, row, 1, var->NiceName(), "", ""), reg(*reg), var(var), rt(rt) {
      value_width = 2 * reg->value_size();
      units_width = 0;
  }
  virtual size_t Width() override { return key.size() + value.size() + 2; }
  virtual VT Get() {
    VT r = rt(reg);
    return var ? (*var)(r) : r;
  }
  virtual void Update(bool full=false) override {
    if (wndw) {
      key_width = key.size();
      Format(Get(), value);
      value_width = value.size();
      FieldBase::Update(full);
    }
  }
  virtual void Active(bool active) override {
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
  virtual std::string Describe() const override {
    std::string description;
    if (var) {
      const std::string& name = var->Name();
      const std::string& nice_name = var->NiceName();
      if (name != nice_name)
        description += nice_name + " (" + name + ")";
      else
        description += name;
      description += std::string(":\n\n");
      description += var->Description();
    }
    else {
      const std::string& name = reg.Name();
      const std::string& nice_name = reg.NiceName();
      if (name != nice_name)
        description += nice_name + " (" + name + ")";
      else
        description += name;
      description += std::string(":\n\n");
      description += reg.Description();

      if (reg.begin() != reg.end()) {
        description += "\n\nVariables:";
        for (auto var : reg) {
          description += std::string("\n");
          const std::string& v_name = var->Name();
          const std::string& v_nice_name = var->NiceName();
          if (v_name != v_nice_name)
            description += v_nice_name + " (" + v_name + ")";
          else
            description += v_name;
        }
      }
    }
    return description;
  }
  virtual bool Activateable() const override { return true; }
  virtual bool ReadOnly() const override { return !(var ? var->Writeable() : reg.Writeable()); }
  virtual void Set(const char *v_str) override
  {
    VT v;
    size_t v_str_len = strlen(v_str);
    if (v_str_len == 0 || v_str_len > 2 || !Parse(v_str, v))
      UI::Error("Invalid value '%s'", v_str);
    else if (var != NULL)
      rt.Device().Write(reg, var->Set(rt(reg), v));
    else
      rt.Device().Write(reg, v);
  }
};

template <typename D, typename AT, typename VT>
std::shared_ptr<UI> make_raw_ui(std::shared_ptr<D> &device, RegisterTable<AT, VT, D> &rt)
{
  auto ui = std::make_shared<UI>();

  ui->Add(device);

  typedef RawField<RegisterTable<AT, VT, D>, VT, D> RF;

  int row = 1, col = 1;
  for (auto reg : rt) {
    if (reg->Readable()) {
      ui->Add(new RF(row++, reg, rt));
      for (auto var : *reg)
        ui->Add(new RF(row++, reg, var, rt));
      ui->Add(new Empty(row++, col));
    }
  }

  return ui;
}

template <typename D, typename AT, typename VT>
std::shared_ptr<UI> make_raw_ui(std::shared_ptr<D> &device, RegisterTableSparse<AT, VT, D> &rt)
{
  auto ui = std::make_shared<UI>();

  ui->Add(device);

  typedef RawField<RegisterTableSparse<AT, VT, D>, VT, D> RF;

  int row = 1, col = 1;
  for (auto reg : rt) {
    if (reg->Readable()) {
      ui->Add(new RF(row++, reg, rt));
      for (auto var : *reg)
        ui->Add(new RF(row++, reg, var, rt));
      ui->Add(new Empty(row++, col));
    }
  }

  return ui;
}

template <typename D, typename AT>
std::shared_ptr<UI> make_raw_ui(std::shared_ptr<D> &device, RegisterTableSparseVar<AT, D> &rt)
{
  auto ui = std::make_shared<UI>();

  ui->Add(device);

  typedef RawField<RegisterTableSparseVar<AT, D>, std::vector<uint8_t>, D> RF;

  int row = 1, col = 1;
  for (auto reg : rt) {
    if (reg->Readable()) {
      ui->Add(new RF(row++, reg, rt));
      for (auto var : *reg)
        ui->Add(new RF(row++, reg, var, rt));
      ui->Add(new Empty(row++, col));
    }
  }

  return ui;
}

#endif // _RAW_UI_H_