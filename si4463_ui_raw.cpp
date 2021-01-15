// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "field_types.h"
#include "si4463.h"
#include "si4463_rt.h"
#include "si4463_ui_raw.h"

template <typename AT>
class SI4463RawField : public Field<uint8_t> {
protected:
  const Register<AT, uint8_t> &reg;
  const Variable<uint8_t> *var;
  RegisterTable<AT, uint8_t, SI4463> *rt;

public:
  SI4463RawField(int row, const Register<AT, uint8_t> *reg, RegisterTable<AT, uint8_t, SI4463> *rt) :
    Field<uint8_t>(UI::statusp, row, 1, reg->Name(), "", ""), reg(*reg), var(NULL), rt(rt) {
      value_width = 2;
      units_width = 0;
      attributes = A_BOLD;
  }
  SI4463RawField(int row, const Register<AT, uint8_t> *reg, const Variable<uint8_t> *var, RegisterTable<AT, uint8_t, SI4463> *rt) :
    Field<uint8_t>(UI::statusp, row, 1, var->Name(), "", ""), reg(*reg), var(var), rt(rt) {
      value_width = 2;
      units_width = 0;
  }
  virtual size_t Width() { return key.size() + 4; }
  virtual uint8_t Get() {
    uint8_t r = reg(rt->Buffer());
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
  virtual bool ReadOnly() { return true; }
};

//   virtual bool ReadOnly() { return false; }
//   virtual void Set(const char *v_str) {
//     uint8_t v = 0;
//     size_t v_str_len = strlen(v_str);
//     if (v_str_len == 0 || v_str_len > 2 || sscanf(v_str, "%02hhx", &v) != 1)
//       UI::Error("invalid value '%s'", v_str);
//     else if (var != NULL)
//       rt->Write(reg, var->Set(reg(rt->Buffer()), v));
//     else
//       rt->Write(reg, v);
//   }

#define EMPTY() fields.push_back(new Empty(row++, col));

#define ADD(AT,REG,RT) fields.push_back(new SI4463RawField<AT>(row++, REG, RT));
#define ADDV(AT,REG,VAR,RT) fields.push_back(new SI4463RawField<AT>(row++, REG, VAR, RT));

SI4463UIRaw::SI4463UIRaw(SI4463 &si4463) :
  UI(),
  si4463(si4463)
{
  devices.insert(&si4463);
  int row = 1, col = 1;

  typedef RegisterTable<uint8_t, uint8_t, SI4463> RTT;
  RTT *pirt = &si4463.RTS.PartInfo;
  RTT *firt = &si4463.RTS.FuncInfo;
  RTT *gpiort = &si4463.RTS.GPIO;
  RTT *fifort = &si4463.RTS.FIFO;
  RTT *statert = &si4463.RTS.DeviceState;
  RTT *intsrt = &si4463.RTS.Interrupts;
  RTT *pktinfort = &si4463.RTS.PacketInfo;
  RTT *mdmstsrt = &si4463.RTS.ModemStatus;
  RTT *adcrt = &si4463.RTS.ADC;

  for (RTT *rt : {pirt, firt, gpiort, fifort, statert, intsrt, pktinfort, mdmstsrt, adcrt}) {
    for (auto &reg : *rt) {
      ADD(uint8_t, reg, rt);
      for (auto var : *reg)
        ADDV(uint8_t, reg, var, rt);
    }
    EMPTY();
  }

  for (auto &reg : si4463.RTS.Property) {
    ADD(uint16_t, reg, &si4463.RTS.Property);
    for (auto var : *reg)
      ADDV(uint16_t, reg, var, &si4463.RTS.Property);
    EMPTY();
  }
}

SI4463UIRaw::~SI4463UIRaw() {}
