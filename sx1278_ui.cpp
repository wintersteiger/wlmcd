// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <math.h>
#include <inttypes.h>

#include "sx1278.h"
#include "sx1278_rt.h"
#include "sx1278_ui.h"

namespace SX1278UIFields {

class RegisterField : public Field<uint8_t> {
protected:
  const SX1278::NormalRegisterTable::TRegister &reg;
  const Variable<uint8_t> *var;
  const std::shared_ptr<SX1278> sx1278;
  const SX1278::NormalRegisterTable &rt;

public:
  RegisterField(int row, const SX1278::NormalRegisterTable::TRegister *reg, const std::shared_ptr<SX1278> sx1278) :
    Field<uint8_t>(UI::statusp, row, 1, reg->Name(), "", ""), reg(*reg), var(NULL), sx1278(sx1278), rt(sx1278->Normal) {
      value_width = 2;
      units_width = 0;
  }
  RegisterField(int row, const SX1278::NormalRegisterTable::TRegister *reg, const Variable<uint8_t> *var, const std::shared_ptr<SX1278> sx1278) :
    Field<uint8_t>(UI::statusp, row, 1, var->Name(), "", ""), reg(*reg), var(var), sx1278(sx1278), rt(sx1278->Normal) {
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
  virtual void Set(const char *value) {}
};

class IndicatorField : public ::IndicatorField {
protected:
  const SX1278::NormalRegisterTable &rt;
public:
  IndicatorField(int r, int c, const std::string &key, const std::shared_ptr<SX1278> sx1278) :
    ::IndicatorField(UI::statusp, r, c, key), rt(sx1278->Normal) {}
  virtual bool Get() = 0;
};

template <typename T>
class ConfigField : public Field<T>
{
protected:
  const std::shared_ptr<SX1278> sx1278;
  const SX1278::NormalRegisterTable &rt;

public:
  ConfigField<T>(int row, int col, const std::string &key, const std::string &value, const std::string &units, const std::shared_ptr<SX1278> sx1278) :
    Field<T>(UI::statusp, row, col, key, value, units), sx1278(sx1278), rt(sx1278->Normal) {}
  virtual ~ConfigField<T>() {}
  virtual T Get() = 0;
  virtual void Update(bool full=false) { Field<T>::Update(full); }
  virtual bool ReadOnly() { return true; }
  virtual void Set(const char*) {}
};

class SettingField : public ConfigField<bool> {
public:
  SettingField(int r, int c, const std::string &key, const std::shared_ptr<SX1278> sx1278) :
    ConfigField<bool>(r, c, key, "", "", sx1278) {}
  virtual size_t Width() { return key.size(); }
  virtual bool Get() = 0;
  virtual void Update(bool full=false) {
    colors = Get() ? ENABLED_PAIR : DISABLED_PAIR;
    if (wndw) {
      if (active) wattron(wndw, A_STANDOUT);
      if (colors != -1) wattron(wndw, COLOR_PAIR(colors));
      mvwprintw(wndw, row, col, "%s", key.c_str());
      if (colors != -1) wattroff(wndw, COLOR_PAIR(colors));
      if (active) wattroff(wndw, A_STANDOUT);
    }
  }
};

class NameField : public Label
{
protected:
  const std::shared_ptr<SX1278> sx1278;

public:
  NameField(int row, int col, const std::shared_ptr<SX1278> sx1278) :
    Label(UI::statusp, row, col, ""), sx1278(sx1278) {}
  virtual ~NameField() {}
  virtual void Update(bool full=false) {
    key = sx1278->Normal.LongRangeMode() ? "SX1278 LoRa" : "SX1278";
    Label::Update(full);
  }
};

#define SIND(N,NN,T,G) \
  class N##SInd : public IndicatorField { \
  public: \
    N##SInd(int r, int c, const std::shared_ptr<SX1278> sx1278) : \
      IndicatorField(r, c, NN, sx1278) {} \
    virtual bool Get() { G } \
  };

#define TCF(N,K,U,T,G) \
  class N##Field : public ConfigField<T> { \
  public: \
    N##Field(int r, int c, const std::shared_ptr<SX1278> sx1278) : \
      ConfigField<T>(r, c, K, "", U, sx1278) {} \
    virtual T Get() { G } \
  };

#define STG(N,K,G) \
  class N##Sttng : public SettingField { \
  public: \
    N##Sttng(int r, int c, const std::shared_ptr<SX1278> sx1278) : \
      SettingField(r, c, K, sx1278) {} \
    virtual bool Get() { G } \
  };

#define EMPTY() fields.push_back(new Empty(row++, col));

SIND(ModeReady, "RDY",  uint8_t, { return rt.ModeReady(); });
SIND(RxReady, "RXR",  uint8_t, { return rt.RxReady(); });
SIND(TxReady, "TXR",  uint8_t, { return rt.TxReady(); });
SIND(PllLock, "PLL",  uint8_t, { return rt.PllLock(); });
SIND(Rssi, "RSS",  uint8_t, { return rt.Rssi(); });
SIND(Timeout, "T/O",  uint8_t, { return rt.Timeout(); });
SIND(PreambleDetect, "PRE",  uint8_t, { return rt.PreambleDetect_(); });
SIND(SyncAddressMatch, "SYN",  uint8_t, { return rt.SyncAddressMatch(); });
SIND(FifoFull, "FUL",  uint8_t, { return rt.FifoFull(); });
SIND(FifoEmpty, "EMP",  uint8_t, { return rt.FifoEmpty(); });
SIND(FifoLevel, "LVL",  uint8_t, { return rt.FifoLevel(); });
SIND(FifoOverrun, "OVR",  uint8_t, { return rt.FifoOverrun(); });
SIND(PacketSent, "SNT",  uint8_t, { return rt.PacketSent(); });
SIND(PayloadReady, "PRY",  uint8_t, { return rt.PayloadReady(); });
SIND(CrcOk, "CRC",  uint8_t, { return rt.CrcOk(); });
SIND(LowBat, "BAT",  uint8_t, { return rt.LowBat_(); });

static std::vector<const char*> modulation_map = { "FSK", "OOK", "?", "?" };
TCF(Modulation, "Modulation",  "",     const char*,  { return modulation_map[rt.ModulationType()]; });

static std::vector<const char*> mode_map = { "Sleep", "Stdby", "FS TX", "TX", "FS RX", "RX", "?", "?" };
TCF(Mode, "Mode",  "",     const char*,  {
  if (sx1278->Responsive()) {
    this->colors = -1;
    return mode_map[rt.Mode()];
  } else {
    this->colors = DISABLED_PAIR;
    return "* N/C *";
  };
});

TCF(RSSI,  "RSSI",   "dBm",  double,  {
  return - (rt.RssiValue() / 2.0);
});

TCF(Frequency,  "Frequency",   "MHz",  double,      {
  double f_rf = (double) ((rt.Frf_23_16() << 16) | (rt.Frf_15_8() << 8) | rt.Frf_7_0());
  return (sx1278->F_STEP() * f_rf) / 1e6;
});

TCF(FrequencyError,  "Freq error",   "kHz",  double,  {
  double f_ei = (double) ((rt.FeiValue_15_8() << 8) | rt.FeiValue_7_0());
  return (sx1278->F_STEP() * f_ei) / 1e3;
});

TCF(Deviation,  "Deviation",   "kHz",  double,  {
  uint64_t f_dev = (rt.Fdev_13_8() << 8) | rt.Fdev_7_0();
  return (sx1278->F_STEP() * (double)f_dev) / 1e3;
});

TCF(Bitrate,  "Bitrate",   "kBd",  double,  {
  uint64_t br = (rt.BitRate_15_8() << 8) | rt.BitRate_7_0();
  return (sx1278->F_XOSC() / (br + (rt.BitRateFrac() / 16.0))) / 1e3;
});

static uint8_t rxbw_mant_map[] = {16, 20, 24};
TCF(FilterBW,  "Filter B/W",   "kHz",  double,  {
  uint8_t mant_raw = rt.RxBwMant();
  if (mant_raw == 0b11)
    return (1.0/0.0);
  return (sx1278->F_XOSC() / (rxbw_mant_map[mant_raw] * pow(2, rt.RxBwExp() + 2))) / 1e3;
});

TCF(RSSIThreshold,  "RSSI T/H",   "dBm",  double,  {
  return - rt.RssiThreshold() / 2.0;
});

STG(SyncDetect, "SYN", { return rt.SyncOn(); });
TCF(SyncSize,  "Sync size",   "B",  uint8_t,  { return rt.DcFreIoHomeOne() ? rt.SyncSize() : rt.SyncSize() + 1; });

class SyncWordField : public ConfigField<std::string> {
public:
  SyncWordField(int r, int c, const std::shared_ptr<SX1278> sx1278) :
    ConfigField<std::string>(r, c, "Sync word", "", "", sx1278) {
      key_width = 10; value_width = 16; units_width = 0;
    }
  virtual std::string Get() {
    size_t len = rt.DcFreIoHomeOne() ? rt.SyncSize() : rt.SyncSize() + 1;
    value_width = 8;
    if (len > 4) value_width = 2*len;
    key_width = 14;
    if (len > 6)
       key_width -= value_width - 12;
    std::vector<uint8_t> svbuf = {  rt.SyncValue1(), rt.SyncValue2(), rt.SyncValue3(), rt.SyncValue4(),
                                    rt.SyncValue5(), rt.SyncValue6(), rt.SyncValue7(), rt.SyncValue8() };
    uint64_t sw = 0;
    for (size_t i = 0; i < len; i++)
      sw = (sw << 8) | svbuf[i];
    char sws[2*len+1];
    snprintf(sws, 2*len+1, "%" PRIx64, sw);
    return sws;
  }
};

static const char* paselect_map[] = { "RFO", "BOOST" };
TCF(PASelect,  "P/A pin",   "",  const char*,  {
  return paselect_map[rt.PaSelect()];
});

TCF(MaxPower,  "Max power",   "dBm",  double,  {
  return 10.8 + 0.6*(double)rt.MaxPower();
});

TCF(OutputPower,  "Output power",   "dBm",  double,  {
  double pmax = rt.PaSelect() ? 17 : 10.8 + 0.6*(double)rt.MaxPower();
  double op = (double)rt.OutputPower();
  return pmax - (15 - op);
});

} // SX1278UIFields

using namespace SX1278UIFields;

SX1278UI::SX1278UI(std::shared_ptr<SX1278> sx1278) :
  UI(),
  sx1278(sx1278),
  num_status_fields(0)
{
  devices.insert(sx1278);

  int row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new NameField(row++, col + 18, sx1278));
  EMPTY();

  Add(new ModeField(row++, col, sx1278));
  EMPTY();

  Add(new Label(UI::statusp, row, col, "Info"));
  Add(new ModeReadySInd(row, col + 6, sx1278));
  Add(new RxReadySInd(row, col + 10, sx1278));
  Add(new TxReadySInd(row, col + 14, sx1278));
  Add(new PllLockSInd(row++, col + 18, sx1278));

  Add(new Label(UI::statusp, row, col, "Trig "));
  Add(new RssiSInd(row, col + 6, sx1278));
  Add(new TimeoutSInd(row, col + 10, sx1278));
  Add(new PreambleDetectSInd(row, col + 14, sx1278));
  Add(new SyncAddressMatchSInd(row++, col + 18, sx1278));

  Add(new Label(UI::statusp, row, col, "FIFO"));
  Add(new FifoFullSInd(row, col + 6, sx1278));
  Add(new FifoEmptySInd(row, col + 10, sx1278));
  Add(new FifoLevelSInd(row, col + 14, sx1278));
  Add(new FifoOverrunSInd(row++, col + 18, sx1278));

  Add(new Label(UI::statusp, row, col, "P/L"));
  Add(new PacketSentSInd(row, col + 6, sx1278));
  Add(new PayloadReadySInd(row, col + 10, sx1278));
  Add(new CrcOkSInd(row, col + 14, sx1278));
  EMPTY();

  Add(new Label(UI::statusp, row, col, "     "));
  Add(new LowBatSInd(row++, col + 6, sx1278));
  Add(new FrequencyErrorField(row++, col, sx1278));
  Add(new RSSIField(row++, col, sx1278));
  EMPTY();

  num_status_fields = fields.size();

  row = 1, col = 30;
  Add(new Label(UI::statusp, row++, col, "Radio"));
  Add(new FrequencyField(row++, col, sx1278));
  Add(new DeviationField(row++, col, sx1278));
  Add(new RSSIThresholdField(row++, col, sx1278));
  EMPTY();

  Add(new Label(UI::statusp, row++, col, "Modem"));
  Add(new FilterBWField(row++, col, sx1278));
  Add(new BitrateField(row++, col, sx1278));
  Add(new ModulationField(row++, col, sx1278));
  // Add(new SyncSizeField(row++, col, sx1278));
  Add(new SyncWordField(row++, col, sx1278));
  Add(new SyncDetectSttng(row++, col, sx1278));
  EMPTY();

  Add(new Label(UI::statusp, row++, col, "TX"));
  Add(new PASelectField(row++, col, sx1278));
  Add(new MaxPowerField(row++, col, sx1278));
  Add(new OutputPowerField(row++, col, sx1278));
}

SX1278UI::~SX1278UI()
{
}

void SX1278UI::Layout() {
  size_t h, w, widest = 0;
  size_t r = 1, c = 30;
  getmaxyx(statusp, h, w);

  // Skip the status fields
  int last_r = -1;
  int last_c = -1;
  for (size_t i=num_status_fields; i < fields.size(); i++) {
    FieldBase *f = fields[i];
    int this_c = c;

    if (f->Row() == last_r) {
      this_c += f->Col() - last_c;
      r--;
    }
    else {
      last_c = f->Col();
      last_r = f->Row();
    }

    f->Move(r++, this_c);

    size_t w = f->Width();
    if (w > widest)
      widest = w;

    if (r >= h) {
      r = 1;
      c += widest + 1;
      widest = 0;
    }
  }
};
