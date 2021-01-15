// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <math.h>
#include <inttypes.h>

#include "gpio_button_field.h"
#include "rfm69.h"
#include "rfm69_rt.h"
#include "rfm69_ui.h"
#include "gpio_button.h"
#include "gpio_button_field.h"

namespace RFM69UIFields {

class RegisterField : public Field<uint8_t> {
protected:
  const Register<uint8_t, uint8_t> &reg;
  const Variable<uint8_t> *var;
  const RFM69 &rfm69;
  const RFM69::RegisterTable &rt;

public:
  RegisterField(int row, const Register<uint8_t, uint8_t> *reg, const RFM69 &rfm69) :
    Field<uint8_t>(UI::statusp, row, 1, reg->Name(), "", ""), reg(*reg), var(NULL), rfm69(rfm69), rt(*rfm69.RT) {
      value_width = 2;
      units_width = 0;
  }
  RegisterField(int row, const Register<uint8_t, uint8_t> *reg, const Variable<uint8_t> *var, const RFM69 &rfm69) :
    Field<uint8_t>(UI::statusp, row, 1, var->Name(), "", ""), reg(*reg), var(var), rfm69(rfm69), rt(*rfm69.RT) {
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
  virtual void Set(const char *value) {}
};

class IndicatorField : public ::IndicatorField {
protected:
  const RFM69::RegisterTable &rt;
public:
  IndicatorField(int r, int c, const std::string &key, const RFM69 &rfm69) :
    ::IndicatorField(UI::statusp, r, c, key), rt(*rfm69.RT) {}
  virtual bool Get() = 0;
};

template <typename T>
class ConfigField : public Field<T>
{
protected:
  const RFM69 &rfm69;
  RFM69::RegisterTable &rt;

public:
  ConfigField<T>(int row, int col, const std::string &key, const std::string &value, const std::string &units, const RFM69 &rfm69) :
    Field<T>(UI::statusp, row, col, key, value, units), rfm69(rfm69), rt(*rfm69.RT) {}
  virtual ~ConfigField<T>() {}
  virtual T Get() = 0;
  virtual void Update(bool full=false) { Field<T>::Update(full); }
  virtual bool ReadOnly() { return true; }
  virtual void Set(const char*) {}
};

class SettingField : public ConfigField<bool> {
public:
  SettingField(int r, int c, const std::string &key, const RFM69 &rfm69) :
    ConfigField<bool>(r, c, key, "", "", rfm69) {}
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

#define SIND(N,NN,T,G) \
  class N##SInd : public IndicatorField { \
  public: \
    N##SInd(int r, int c, const RFM69 &rfm69) : \
      IndicatorField(r, c, NN, rfm69) {} \
    virtual bool Get() { G } \
  };

#define TCF(N,K,U,T,G) \
  class N##Field : public ConfigField<T> { \
  public: \
    N##Field(int r, int c, const RFM69 &rfm69) : \
      ConfigField<T>(r, c, K, "", U, rfm69) {} \
    virtual T Get() { G } \
  };

#define STG(N,K,G) \
  class N##Sttng : public SettingField { \
  public: \
    N##Sttng(int r, int c, const RFM69 &rfm69) : \
      SettingField(r, c, K, rfm69) {} \
    virtual bool Get() { G } \
  };

#define EMPTY() fields.push_back(new Empty(row++, col));

SIND(ModeReady, "RDY",  uint8_t, { return rt.ModeReady(); });
SIND(RxReady, "RXR",  uint8_t, { return rt.RxReady(); });
SIND(TxReady, "TXR",  uint8_t, { return rt.TxReady(); });
SIND(PllLock, "PLL",  uint8_t, { return rt.PllLock(); });
SIND(Rssi, "RSS",  uint8_t, { return rt.Rssi(); });
SIND(Timeout, "T/O",  uint8_t, { return rt.Timeout(); });
SIND(AutoMode, "AUM",  uint8_t, { return rt.AutoMode(); });
SIND(SyncAddressMatch, "SYN",  uint8_t, { return rt.SyncAddressMatch(); });
SIND(FifoFull, "FUL",  uint8_t, { return rt.FifoFull(); });
SIND(FifoEmpty, "EMP",  uint8_t, { return rt.FifoEmpty(); });
SIND(FifoLevel, "LVL",  uint8_t, { return rt.FifoLevel(); });
SIND(FifoOverrun, "OVR",  uint8_t, { return rt.FifoOverrun(); });
SIND(PacketSent, "SNT",  uint8_t, { return rt.PacketSent(); });
SIND(PayloadReady, "PRY",  uint8_t, { return rt.PayloadReady(); });
SIND(CrcOk, "CRC",  uint8_t, { return rt.CrcOk(); });

static std::vector<const char*> modulation_map = { "FSK", "OOK", "?", "?" };
TCF(Modulation, "Modulation",  "",     const char*,  { return modulation_map[rt.ModulationType()]; });

static std::vector<const char*> mode_map = { "Sleep", "Stdby", "FS", "TX", "RX", "?", "?", "?" };
TCF(Mode, "Mode",  "",     const char*,  {
  if (rfm69.Responsive()) {
    this->colors = -1;
    if (rt.ListenOn())
      return "LISTEN";
    else {
      auto m = rt.Mode();
      return mode_map[m];
    }
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
  return (rfm69.F_STEP() * f_rf) / 1e6;
});

TCF(FrequencyError,  "Freq error",   "kHz",  double,  {
  int16_t fei_value = (rt.FeiMsb() << 8) | rt.FeiLsb();
  return (rfm69.F_STEP() * fei_value) / 1e3;
});

TCF(Deviation,  "Deviation",   "kHz",  double,  {
  uint64_t f_dev = (rt.Fdev_13_8() << 8) | rt.Fdev_7_0();
  return (rfm69.F_STEP() * (double)f_dev) / 1e3;
});

TCF(Bitrate,  "Bitrate",   "kBd",  double,  {
  uint64_t br = (rt.BitrateMsb() << 8) | rt.BitrateLsb();
  return rfm69.F_XOSC() / (double)br / 1e3;
});

static uint8_t rxbw_mant_map[] = {16, 20, 24};
TCF(FilterBW,  "Filter B/W",   "kHz",  double,  {
  uint8_t mant_raw = rt.RxBwMant();
  if (mant_raw == 0b11)
    return (1.0/0.0);
  auto divisor = (rxbw_mant_map[mant_raw] * pow(2, rt.RxBwExp() + 2));
  // TODO: if (OOK) divisor *= 2;
  auto res = rfm69.F_XOSC() / divisor / 1e3;
  return res;
});

TCF(RSSIThreshold,  "RSSI T/H",   "dBm",  double,  {
  return - rt.RssiThresh() / 2.0;
});

STG(SyncOn, "SYN", { return rt.SyncOn(); });

} // RFM69UIFields

using namespace RFM69UIFields;

RFM69UI::RFM69UI(RFM69 &rfm69, GPIOButton *reset_button) :
  UI(),
  rfm69(rfm69),
  num_status_fields(0)
{
  devices.insert(&rfm69);

  int row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  EMPTY();

  if (reset_button) {
    Add(new GPIOButtonField(UI::statusp, row++, col+10, "RESET", *reset_button));
    row++;
  }

  Add(new ModeField(row++, col, rfm69));
  EMPTY();

  Add(new Label(UI::statusp, row, col, "Info"));
  Add(new ModeReadySInd(row, col + 6, rfm69));
  Add(new RxReadySInd(row, col + 10, rfm69));
  Add(new TxReadySInd(row, col + 14, rfm69));
  Add(new PllLockSInd(row++, col + 18, rfm69));

  Add(new Label(UI::statusp, row, col, "Trig "));
  Add(new RssiSInd(row, col + 6, rfm69));
  Add(new TimeoutSInd(row, col + 10, rfm69));
  Add(new AutoModeSInd(row, col + 14, rfm69));
  Add(new SyncAddressMatchSInd(row++, col + 18, rfm69));

  Add(new Label(UI::statusp, row, col, "FIFO"));
  Add(new FifoFullSInd(row, col + 6, rfm69));
  Add(new FifoEmptySInd(row, col + 10, rfm69));
  Add(new FifoLevelSInd(row, col + 14, rfm69));
  Add(new FifoOverrunSInd(row++, col + 18, rfm69));

  Add(new Label(UI::statusp, row, col, "P/L"));
  Add(new PacketSentSInd(row, col + 6, rfm69));
  Add(new PayloadReadySInd(row, col + 10, rfm69));
  Add(new CrcOkSInd(row, col + 14, rfm69));
  EMPTY();

  row++;
  Add(new Label(UI::statusp, row, col, "     "));
  Add(new FrequencyErrorField(row++, col, rfm69));
  Add(new RSSIField(row++, col, rfm69));
  EMPTY();

  num_status_fields = fields.size();

  row = 1, col = 30;
  Add(new Label(UI::statusp, row++, col, "Radio"));
  Add(new FrequencyField(row++, col, rfm69));
  Add(new DeviationField(row++, col, rfm69));
  Add(new RSSIThresholdField(row++, col, rfm69));
  EMPTY();

  Add(new Label(UI::statusp, row++, col, "Modem"));
  Add(new FilterBWField(row++, col, rfm69));
  Add(new BitrateField(row++, col, rfm69));
  Add(new ModulationField(row++, col, rfm69));
  Add(new SyncOnSttng(row++, col, rfm69));
  EMPTY();

  // Add(new Label(UI::statusp, row++, col, "TX"));
}

RFM69UI::~RFM69UI()
{
}

void RFM69UI::Layout() {
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
