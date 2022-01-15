// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <curses.h>
#include <math.h>
#include <inttypes.h>

#include "gpio_button_field.h"
#include "field.h"
#include "rfm69.h"
#include "rfm69_rt.h"
#include "rfm69_ui.h"
#include "gpio_button.h"
#include "gpio_button_field.h"

namespace RFM69UIFields {

class RegisterField : public Field<uint8_t> {
protected:
  const typename RFM69::RegisterTable::TRegister &reg;
  const Variable<uint8_t> *var;
  const std::shared_ptr<RFM69> rfm69;
  const RFM69::RegisterTable &rt;

public:
  RegisterField(int row, const typename RFM69::RegisterTable::TRegister *reg, const std::shared_ptr<RFM69> rfm69) :
    Field<uint8_t>(UI::statusp, row, 1, reg->Name(), "", ""), reg(*reg), var(NULL), rfm69(rfm69), rt(*rfm69->RT) {
      value_width = 2;
      units_width = 0;
  }
  RegisterField(int row, const typename RFM69::RegisterTable::TRegister *reg, const Variable<uint8_t> *var, const std::shared_ptr<RFM69> rfm69) :
    Field<uint8_t>(UI::statusp, row, 1, var->Name(), "", ""), reg(*reg), var(var), rfm69(rfm69), rt(*rfm69->RT) {
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
  const RFM69::RegisterTable &rt;
public:
  IndicatorField(int r, int c, const std::string &key, const std::shared_ptr<RFM69> rfm69) :
    ::IndicatorField(UI::statusp, r, c, key), rt(*rfm69->RT) {}
  virtual bool Get() = 0;
};

template <typename T>
class ConfigField : public Field<T>
{
protected:
  const std::shared_ptr<RFM69> rfm69;
  RFM69::RegisterTable &rt;

public:
  ConfigField<T>(int row, int col, const std::string &key, const std::string &value, const std::string &units, const std::shared_ptr<RFM69> rfm69) :
    Field<T>(UI::statusp, row, col, key, value, units), rfm69(rfm69), rt(*rfm69->RT) {}
  virtual ~ConfigField<T>() {}
  virtual T Get() = 0;
  virtual void Update(bool full=false) { Field<T>::Update(full); }
  virtual bool ReadOnly() { return true; }
  virtual void Set(const char*) {}
};

class SettingField : public ConfigField<bool> {
public:
  SettingField(int r, int c, const std::string &key, const std::shared_ptr<RFM69> rfm69) :
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

#define SIND(N,NN,G) \
  class N##SInd : public IndicatorField { \
  public: \
    N##SInd(int r, int c, const std::shared_ptr<RFM69> rfm69) : \
      IndicatorField(r, c, NN, rfm69) {} \
    virtual bool Get() { G } \
  };

#define TCF(N,K,U,T,G) \
  class N##Field : public ConfigField<T> { \
  public: \
    N##Field(int r, int c, const std::shared_ptr<RFM69> rfm69) : \
      ConfigField<T>(r, c, K, "", U, rfm69) {} \
    virtual T Get() { G } \
  };

#define STG(N,K,G) \
  class N##Sttng : public SettingField { \
  public: \
    N##Sttng(int r, int c, const std::shared_ptr<RFM69> rfm69) : \
      SettingField(r, c, K, rfm69) {} \
    virtual bool Get() { G } \
  };

#define EMPTY() fields.push_back(new Empty(row++, col));

SIND(ModeReady, "RDY", { return rt.ModeReady(); });
SIND(RxReady, "RXR", { return rt.RxReady(); });
SIND(TxReady, "TXR", { return rt.TxReady(); });
SIND(PllLock, "PLL", { return rt.PllLock(); });
SIND(Rssi, "RSS", { return rt.Rssi(); });
SIND(Timeout, "T/O", { return rt.Timeout(); });
SIND(AutoMode, "AUM", { return rt.AutoMode(); });
SIND(SyncAddressMatch, "SYN", { return rt.SyncAddressMatch(); });
SIND(FifoFull, "FUL", { return rt.FifoFull(); });
SIND(FifoEmpty, "EMP", { return !rt.FifoNotEmpty(); });
SIND(FifoLevel, "LVL", { return rt.FifoLevel(); });
SIND(FifoOverrun, "OVR", { return rt.FifoOverrun(); });
SIND(PacketSent, "SNT", { return rt.PacketSent(); });
SIND(PayloadReady, "PRY", { return rt.PayloadReady(); });
SIND(CrcOk, "CRC", { return rt.CrcOk(); });

SIND(Sequencer, "SEQ", { return !rt.SequencerOff(); });
SIND(ListenOn, "LSN", {return rt.ListenOn(); });

static std::vector<const char*> modulation_map = { "FSK", "OOK", "?", "?" };
TCF(Modulation, "Modulation",  "",     const char*,  { return modulation_map[rt.ModulationType()]; });

static std::vector<const char*> modulation_shaping_map_fsk = { "None", "BT=1.0", "BT=0.5", "BT=0.3" };
static std::vector<const char*> modulation_shaping_map_ook = { "None", "f_c=BR", "f_c=2BR", "?" };
TCF(ModulationShaping, "Mod shaping",  "",     const char*,  {
  if (rt.ModulationType() == 0)
    return modulation_shaping_map_fsk[rt.ModulationShaping()];
  else if (rt.ModulationType() == 1)
    return modulation_shaping_map_ook[rt.ModulationShaping()];
  else
    return "?";
});

static std::vector<const char*> mode_map = { "Sleep", "Stdby", "FS", "TX", "RX", "?", "?", "?" };
TCF(Mode, "Mode",  "",     const char*,  {
  if (rfm69->Responsive()) {
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
  return (rfm69->F_STEP() * f_rf) / 1e6;
});

TCF(FrequencyError,  "Freq error",   "kHz",  double,  {
  int16_t fei_value = (rt.FeiMsb() << 8) | rt.FeiLsb();
  return (rfm69->F_STEP() * fei_value) / 1e3;
});

TCF(Deviation,  "Deviation",   "kHz",  double,  {
  uint64_t f_dev = (rt.Fdev_13_8() << 8) | rt.Fdev_7_0();
  return (rfm69->F_STEP() * (double)f_dev) / 1e3;
});

TCF(Bitrate,  "Bitrate",   "kBd",  double,  {
  return rfm69->rBitrate();
});

static double dcc_freq_map[] = {16, 8, 4, 2, 1, 0.5, 0.25, 0.125};
TCF(DccFreq, "DC c/o", "%", double, { return dcc_freq_map[rt.DccFreq()]; })

static uint8_t rxbw_mant_map[] = {16, 20, 24};
TCF(FilterBW,  "Fltr bandwidth",   "kHz",  double,  {
  uint8_t mant_raw = rt.RxBwMant();
  if (mant_raw == 0b11)
    return (1.0/0.0);
  auto divisor = (rxbw_mant_map[mant_raw] * pow(2, rt.RxBwExp() + 2));
  if (rt.ModulationType() == 1)
    divisor *= 2;
  auto res = rfm69->F_XOSC() / divisor / 1e3;
  return res;
});

TCF(RSSIThreshold,  "RSSI T/H",   "dBm",  double,  {
  return - rt.RssiThresh() / 2.0;
});

STG(SyncOn, "SYN", { return rt.SyncOn(); });
TCF(SyncSize, "Sync size", "B", uint8_t, {
  attributes = rt.SyncOn() > 0 ? A_NORMAL : A_DIM;
  return rt.SyncSize() + 1;
});
TCF(SyncTolerance, "Sync tolerance", "b", uint8_t, {
  attributes = rt.SyncOn() > 0 ? A_NORMAL : A_DIM;
  return rt.SyncTol();
});

TCF(PreambleSize, "# preamble", "B", uint8_t, { return rt.PreambleMsb() << 8 | rt.PreambleLsb(); });

static std::vector<const char*> format_map = { "Fixed", "Variable" };
TCF(PacketFormat, "Packet format", "", const char*,  { return format_map[rt.PacketFormat()]; });

static std::vector<const char*> dc_free_map = { "None", "Manchester", "Whitening", "?" };
TCF(DCFree, "DC free coding", "", const char*, { return dc_free_map[rt.DcFree()]; });

STG(CrcOn, "CRC", { return rt.CrcOn(); });
STG(CrcACO, "CLR", { return !rt.CrcAutoClearOff(); });

static std::vector<const char*> address_filtering_map = { "None", "Node", "Node | B/C", "?" };
TCF(AddressFiltering, "Address filter", "", const char*,  { return address_filtering_map[rt.AddressFiltering()]; });

TCF(NodeAddress, "Node addr", "", uint8_t, {
  attributes = rt.AddressFiltering() > 0 ? A_NORMAL : A_DIM;
  return rt.NodeAdrs();
});

TCF(BroadcastAddress, "Broadcast addr", "", uint8_t, {
  attributes = rt.AddressFiltering() > 0 ? A_NORMAL : A_DIM;
  return rt.BroadcastAdrs();
});

TCF(PayloadLength, "Payload length", "", uint8_t, { return rt.PayloadLength(); });

class SyncWordField : public ::HexField {
  const std::shared_ptr<RFM69> rfm69;
  uint64_t last_sync_word;
public:
  SyncWordField(int r, int c, const std::shared_ptr<RFM69> rfm69) :
    ::HexField(UI::statusp, r, c, 0, NULL), rfm69(rfm69), last_sync_word(0) {
    key = "Sync word";
    key_width = key.size();
    value_width = 16;
  }
  virtual ~SyncWordField() {}
  void Update(bool full) {
    if (wndw) {
      uint64_t sw = rfm69->rSyncWord();
      if (sw != last_sync_word || full) {
        attributes = rfm69->RT->SyncOn() > 0 ? A_NORMAL : A_DIM;
        if (active)
          attributes |= A_STANDOUT;
        char tmp2[256];
        snprintf(tmp, sizeof(tmp), "%s:  %%0%d" PRIx64, key.c_str(), value_width);
        snprintf(tmp2, sizeof(tmp2), tmp, sw);
        value = tmp2;
        if (attributes != -1) wattron(wndw, attributes);
        snprintf(tmp, sizeof(tmp), "%%- %ds", value_width);
        mvwprintw(wndw, row, col, tmp, value.c_str());
        if (attributes != -1) wattroff(wndw, attributes);
        last_sync_word = sw;
      }
    }
  }
};

STG(AutoAFC, "AFC", { return rt.AfcAutoOn(); });
STG(AutoAFCClear, "CLR", { return rt.AfcAutoclearOn(); });

TCF(DccFreqAFC, "DC c/o", "%", double, {
  attributes = rt.AfcAutoOn() > 0 ? A_NORMAL : A_DIM;
  return dcc_freq_map[rt.DccFreqAfc()];
})

static uint8_t rxbw_mant_afc_map[] = {16, 20, 24};
TCF(FilterBWAFC,  "Fltr bandwidth",   "kHz",  double,  {
  attributes = rt.AfcAutoOn() > 0 ? A_NORMAL : A_DIM;
  uint8_t mant_raw = rt.RxBwMantAfc();
  if (mant_raw == 0b11)
    return (1.0/0.0);
  auto divisor = (rxbw_mant_afc_map[mant_raw] * pow(2, rt.RxBwExpAfc() + 2));
  if (rt.ModulationType() == 1)
    divisor *= 2;
  auto res = rfm69->F_XOSC() / divisor / 1e3;
  return res;
});

STG(LowBetaAFC, "LWB", {
  attributes = rt.AfcAutoOn() > 0 ? A_NORMAL : A_DIM;
  return rt.AfcLowBetaOn();
});

TCF(LowBetaAfcOffset, "low b o/s", "kHz", double, {
  attributes = rt.AfcAutoOn() > 0 ? A_NORMAL : A_DIM;
  return (rt.LowBetaAfcOffset() * 488.0) / 1e3;
});

} // RFM69UIFields

using namespace RFM69UIFields;

RFM69UI::RFM69UI(std::shared_ptr<RFM69> rfm69, std::shared_ptr<GPIOButton> reset_button) :
  UI(),
  rfm69(rfm69),
  num_status_fields(0)
{
  devices.insert(rfm69);

  int row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  EMPTY();

  if (reset_button) {
    Add(new GPIOButtonField(UI::statusp, row++, col+10, "RESET", reset_button));
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
  Add(new Label(UI::statusp, row, col, "Misc"));
  Add(new ListenOnSInd(row, col + 6, rfm69));
  Add(new SequencerSInd(row, col + 10, rfm69));
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
  Add(new DccFreqField(row++, col, rfm69));
  Add(new FilterBWField(row++, col, rfm69));
  EMPTY();
  Add(new AutoAFCSttng(row, col, rfm69));
  Add(new AutoAFCClearSttng(row, col + 4, rfm69));
  Add(new LowBetaAFCSttng(row++, col + 8, rfm69));
  Add(new DccFreqAFCField(row++, col, rfm69));
  Add(new FilterBWAFCField(row++, col, rfm69));
  Add(new LowBetaAfcOffsetField(row++, col, rfm69));
  EMPTY();

  Add(new Label(UI::statusp, row++, col, "Modem"));
  Add(new BitrateField(row++, col, rfm69));
  Add(new ModulationField(row++, col, rfm69));
  Add(new ModulationShapingField(row++, col, rfm69));
  Add(new PreambleSizeField(row++, col, rfm69));
  Add(new DCFreeField(row++, col, rfm69));
  EMPTY();
  Add(new SyncOnSttng(row++, col, rfm69));
  Add(new Label(UI::statusp, row, col, "Sync word: "));
  Add(new SyncWordField(row++, col, rfm69));
  Add(new SyncSizeField(row++, col, rfm69));
  Add(new SyncToleranceField(row++, col, rfm69));
  EMPTY();

  row++;
  Add(new Label(UI::statusp, row++, col, "Packet control"));
  Add(new PacketFormatField(row++, col, rfm69));
  Add(new PayloadLengthField(row++, col, rfm69));
  Add(new CrcOnSttng(row, col, rfm69));
  Add(new CrcACOSttng(row++, col + 4, rfm69));
  Add(new AddressFilteringField(row++, col, rfm69));
  Add(new NodeAddressField(row++, col, rfm69));
  Add(new BroadcastAddressField(row++, col, rfm69));
  EMPTY();
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
