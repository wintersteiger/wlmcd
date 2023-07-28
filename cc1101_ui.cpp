// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <cmath>

#include "field_types.h"
#include "cc1101.h"
#include "cc1101_rt.h"
#include "cc1101_ui.h"

namespace CC1101UIFields {

template<typename T>
class StatusField : public Field<T> {
protected:
  const std::shared_ptr<CC1101> cc1101;
  CC1101::RegisterTable &rt;
public:
  StatusField<T>(int row, int col, const std::string &key, const std::string &value, const std::string &units, const std::shared_ptr<CC1101> cc1101) :
    Field<T>(UI::statusp, row, col, key, value, units), cc1101(cc1101), rt(*cc1101->RT) {}
  virtual T Get() = 0;
};

#define TSF(N,K,U,T,G) \
  class N##Field : public StatusField<T> { \
  public: \
    N##Field(int r, int c, const std::shared_ptr<CC1101> cc1101) : \
      StatusField<T>(r, c, K, "", U, cc1101) {} \
    virtual T Get() { G } \
  };

class StatusIndicator : public IndicatorField {
protected:
  CC1101::RegisterTable &rt;
public:
  StatusIndicator(int r, int c, const std::string &key, const std::shared_ptr<CC1101> cc1101) :
    IndicatorField(UI::statusp, r, c, key), rt(*cc1101->RT) {}
  virtual bool Get() = 0;
};

#define IND(N,T,G) \
  class N##StatusIndicator : public StatusIndicator { \
  public: \
    N##StatusIndicator(int r, int c, const std::shared_ptr<CC1101> cc1101) : \
      StatusIndicator(r, c, "" # N, cc1101) {} \
    virtual bool Get() { G } \
  };

template <typename T>
class ConfigField : public Field<T>
{
protected:
  CC1101::RegisterTable &rt;

public:
  ConfigField<T>(int row, int col, const std::string &key, const std::string &value, const std::string &units, std::shared_ptr<CC1101> cc1101) :
    Field<T>(UI::statusp, row, col, key, value, units), rt(*cc1101->RT) {}
  virtual ~ConfigField<T>() {}
  virtual T Get() override = 0;
  virtual void Update(bool full=false) override { Field<T>::Update(full); }
  virtual bool ReadOnly() const override { return false; }
  virtual void Set(const char*) override {}
};

#define TCF(N,K,U,T,G) \
  class N##Field : public ConfigField<T> { \
  public: \
    N##Field(int r, int c, std::shared_ptr<CC1101> cc1101) : \
      ConfigField<T>(r, c, K, "", U, cc1101) {} \
    virtual T Get() { G } \
  };

class SettingField : public ConfigField<bool> {
public:
  SettingField(int r, int c, const std::string &key, std::shared_ptr<CC1101> cc1101) :
    ConfigField<bool>(r, c, key, "", "", cc1101) {}
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

#define STG(N,K,G) \
  class N##Sttng : public SettingField { \
  public: \
    N##Sttng(int r, int c, std::shared_ptr<CC1101> cc1101) : \
      SettingField(r, c, K, cc1101) {} \
    virtual bool Get() { G } \
  };


TSF(FREQEST,  "Freq O/S E", "kHz",  uint8_t,      { return cc1101->rFOE(); });
TSF(LQI,      "LQI",        "%",    uint8_t,      { return cc1101->rLQI(); });
TSF(RSSI,     "RSSI",       "dBm",  double,       { return cc1101->rRSSI(); });
TSF(State,    "State",      "",     std::string,  {
  if (cc1101->Responsive()) {
    this->colors = -1;
    return CC1101::StateName((CC1101::State)rt.MARCSTATE());
  } else {
    this->colors = DISABLED_PAIR;
    return "* N/C *";
  };
});
TSF(WORTimer, "WOR Timer",  "ms",   uint16_t,     { return rt.WORTIME1() << 8 | rt.WORTIME0(); });
TSF(PLLCal,   "PLL cal",    "",     bytes_t,      { return { rt.VCO_VC_DAC() }; });
TSF(NumTXRX,  "#TX/#RX",    "B",    pair_uint8_t, { return std::make_pair(rt.TXBYTES(), rt.RXBYTES()); });
TSF(XOSCCal,  "XOSC cal",   "",     bytes_t,      { return mk_bytes(rt.RCCTRL1_STATUS() & 0x7F, rt.RCCTRL0_STATUS() & 0x7F); });

IND(CS,   uint8_t, { auto x = rt.PKTSTATUS() & 0x40; return x; });
IND(PQT,  uint8_t, { return rt.PKTSTATUS() & 0x20; });
IND(CCA,  uint8_t, { return rt.PKTSTATUS() & 0x10; });
IND(CRC,  uint8_t, { return rt.PKTSTATUS() & 0x80; });
IND(SFD,  uint8_t, { return rt.PKTSTATUS() & 0x08; });
IND(GDO0, uint8_t, { return rt.PKTSTATUS() & 0x01; });
IND(GDO2, uint8_t, { return rt.PKTSTATUS() & 0x04; });

IND(PLL,  uint8_t, { return rt.FSCAL1() != 0x3F; });

class GDOField : public ConfigField<uint8_t>
{
protected:
  size_t colon_inx;
public:
  GDOField(int row, int col, std::shared_ptr<CC1101> cc1101) :
    ConfigField<uint8_t>(row, col, "", "", "", cc1101), colon_inx(col+1) {}
  virtual ~GDOField() {}
  virtual uint8_t Get() { return 0; }
  virtual void Update(bool full=false);
};

static std::vector<const char*> gdo_drive_map = { "low", "high" };
TCF(GDODrive,   "Drive",       "",    const char*,  { return gdo_drive_map[(rt.IOCFG1() & 0x80) >> 7]; });

STG(ATS,        "TEMP", { return (rt.IOCFG0() & 0x80) != 0; });
TCF(Deviation,  "Deviation",  "kHz",  double,       { return rt.Device().rDeviation()/1e3; });
TCF(FilterBW,   "Filter B/W", "kHz",  double,       { return rt.Device().rFilterBW()/1e3; });

static std::vector<const char*> modulation_map = { "2-FSK", "GFSK", "?", "ASK/OOK", "4-FSK", "?", "?", "MSK" };
TCF(Modulation, "Modulation",  "",     const char*,  { return modulation_map[(rt.MDMCFG2() & 0x70) >> 4]; });

static std::vector<const char*> sync_mode_map = {
"No sync", "15/16" , "16/16", "30/32",
"CS > TH", "15/16 CS", "16/16 CS", "30/32 CS"
};
TCF(SyncMode,   "Sync Mode",  "",     const char*,  { return sync_mode_map[rt.MDMCFG2() & 0x07]; });
TCF(SyncWord,   "Sync Word",  "",     bytes_t,      { return mk_bytes(rt.SYNC1(), rt.SYNC0()); });

STG(ADCRet,     "ADCRET", { return rt.FIFOTHR() & 0x70; });

static std::vector<const char*> rx_attn_map = { "0", "6", "12", "18" };
TCF(RXAttn,     "RX Attn",    "dB",   const char*,  { return rx_attn_map[(rt.FIFOTHR() & 0x30) >> 4]; });

static std::vector<std::pair<size_t, size_t>> fifo_thr_map = { // { TX, RX }
  { 61,  4 }, { 57,  8 }, { 53, 12 }, { 49, 16 }, { 45, 20 }, { 41, 24 }, { 37, 28 }, { 33, 32 },
  { 29, 36 }, { 25, 40 }, { 21, 44 }, { 17, 48 }, { 13, 52 }, {  9, 56 }, {  5, 60 }, {  1, 64 }
};
TCF(FIFOTXRX,   "FIFO TX/RX",  "B",     const char*,  {
  auto txrx = fifo_thr_map[rt.FIFOTHR() & 0x0F];
  snprintf(tmp, sizeof(tmp), "%zd/%zd", txrx.first, txrx.second);
  return tmp;
});

TCF(PktLength,  "Pkt length",   "B",  uint8_t,      { return rt.PKTLEN(); });
TCF(PQEThresh,  "PQE Thresh",   "B",  uint8_t,      { return (uint8_t)((rt.PKTCTRL1() & 0xE0) >> 5); });

static std::vector<const char*> addr_chk_map = { "None", "Addr", "Addr/00", "Ad/00/FF" };
TCF(AddrChk,    "Addr Chk",     "",   const char*,  { return addr_chk_map[rt.PKTCTRL1() & 0x03]; });

static std::vector<const char*> pkt_fmt_map = { "FIFO", "Sync GDO", "Random TX", "Async GDO" };
TCF(PktFormat,  "Packet format","",   const char*,  { return pkt_fmt_map[(rt.PKTCTRL0() & 0x30) >> 4]; });

STG(DataWhtng,  "WHT",            { return rt.PKTCTRL0() & 0x40; });
STG(CRC,        "CRC",            { return rt.PKTCTRL0() & 0x04; });
STG(CRCAF,      "CRC-A/F",        { return rt.PKTCTRL1() & 0x08; });
STG(Append,     "APP",            { return rt.PKTCTRL1() & 0x04; });

static std::vector<const char*> pkt_mode_map = { "Fixed", "Variable", "Infinite", "Reserved" };
TCF(PktMode,    "Pkt mode",     "",   const char*,  { return pkt_mode_map[rt.PKTCTRL0() & 0x03]; });

TCF(Address,    "Address",      "",   const char*,  {
  snprintf(tmp, sizeof(tmp), "%d (%02x)", rt.ADDR(), rt.ADDR());
  return tmp;
});

TCF(Channel,    "Channel",      "",    uint8_t,     { return rt.CHANNR(); });
TCF(IFFreq,     "IF freq",      "kHz", double,      { return rt.Device().rIFFrequency()/1e3; });
TCF(FreqOS,     "Freq O/S",     "kHz", int8_t,      { return rt.FSCTRL0(); });

STG(DCBlock,    "DCB",  { return !(rt.MDMCFG2() & 0x80); });
STG(Manchester, "MAN",  { return rt.MDMCFG2() & 0x08; });
STG(FEC,        "FEC",  { return rt.MDMCFG1() & 0x80; });

static std::vector<const char*> num_pre_map = { "2", "3", "4", "6", "8", "12", "16", "24" };
TCF(NPreamble,  "# preamble",   "B",   const char*, { return num_pre_map[(rt.MDMCFG1() & 0x70) >> 4]; });

TCF(ChSpacing,  "Ch spacing",   "kHz", double,      { return rt.Device().rChannelSpacing() / 1e3; });

TCF(RXTerm,     "RX term",      "",    const char*, { return ""; });
STG(RXTermRSSI, "RSSI", { return rt.MCSM2() & 0x10; });
STG(RXTermSPQI, "S+PQI", { return rt.MCSM2() & 0x08; });

TCF(WOREvent0,  "Event 0 T/O",  "s",   double,      { return rt.Device().rEvent0(); });
TCF(RXTO,       "RX T/O",       "ms",  double,      { return rt.Device().rRXTimeout(); });

static std::vector<const char*> cca_mode_map = { "Always", "RSSI<TH", "Rcvng", "RSSI+Rcv" };
TCF(CCAMode,    "CCA mode",     "",    const char*, { return cca_mode_map[(rt.MCSM1() & 0x30) >> 4]; });

static std::vector<const char*> rxtx_off_map = { "IDLE", "FSTXON", "TX", "RX" };
TCF(RXOffMd,    "RX off md",    "",    const char*, { return rxtx_off_map[(rt.MCSM1() & 0x0C) >> 2]; });
TCF(TXOffMd,    "TX off md",    "",    const char*, { return rxtx_off_map[rt.MCSM1() & 0x03]; });

static std::vector<const char*> fs_autocal_map = { "Never", "I->*X", "*X->I", "*X-4>I" };
TCF(FSAutocal,  "FS autocal",   "",    const char*, { return fs_autocal_map[(rt.MCSM0() & 0x30) >> 4]; });

static std::vector<const char*> po_to_map = { "~2.3-2.4", "~37-30", "~149-155", "~597-620" };
TCF(POTO,       "PO T/O",       "us",  const char*, { return po_to_map[(rt.MCSM0() & 0x0C) >> 2]; });

STG(PinRdoCtrl, "PINCTRL", { return rt.MCSM0() & 0x02; });
STG(XOSCSleep,  "XOSLEEP", { return rt.MCSM0() & 0x01; });
STG(BSCSGate,   "BS/CS", { return rt.FOCCFG() & 0x20; });

TCF(FOCPreK,    "FOC pre",      "K",   uint8_t,     { return ((rt.FOCCFG() & 0x18) >> 3) + 1; });

static std::vector<const char*> foc_post_map = { "= FOC pre", "0.5" };
TCF(FOCPostK,   "FOC post",     "K",   const char*, { return foc_post_map[(rt.FOCCFG() & 0x04) >> 2]; });

static std::vector<const char*> foc_lim_map = { "None", "+-BW/8", "+-BW/4", "+-BW/2" };
TCF(FOCLimit,   "FOC limit",    "",    const char*, { return foc_lim_map[rt.FOCCFG() & 0x03]; });

TCF(BSPreKI,   "BS pre",        "K_i", uint8_t,     { return (rt.BSCFG() & 0xC0) >> 6; });
TCF(BSPreKP,   "BS pre",        "K_p", uint8_t,     { return (rt.BSCFG() & 0x30) >> 4; });

static std::vector<const char*> bs_post_ki_map = { "= BS pre", "0.5" };
TCF(BSPostKI,  "BS post",       "K_i", const char*, { return bs_post_ki_map[(rt.BSCFG() & 0x08) >> 3]; });

static std::vector<const char*> bs_post_kp_map = { "= BS pre", "0.5" };
TCF(BSPostKP,  "BS post",       "K_p", const char*, { return bs_post_kp_map[(rt.BSCFG() & 0x04) >> 2]; });

static std::vector<const char*> bs_limit_map = { "off", "3.125", "6.25", "12.5" };
TCF(BSLimit,   "BS limit",      "%",   const char*,  { return bs_limit_map[(rt.BSCFG() & 0x03)]; });

static std::vector<const char*> max_dvga_gain_map = { "all", "-1", "-2", "-3" };
TCF(MaxDVGAGain, "DVGA gain",   "",    const char*,  { return max_dvga_gain_map[(rt.AGCCTRL2() & 0xC0) >> 6]; });

static std::vector<const char*> max_lna_gain_map = { "-0.0", "-2.6", "-6.1", "-7.4", "-9.2", "-11.5", "-14.6", "-17.1" };
TCF(MaxLNAGain, "LNA gain",     "dB",  const char*,  { return max_lna_gain_map[(rt.AGCCTRL2() & 0x38) >> 3]; });

static std::vector<const char*> magn_target_map = { "24", "27", "30", "33", "36", "38", "40", "42" };
TCF(MAGNTarget, "MAGN target",  "dB",  const char*,  { return magn_target_map[rt.AGCCTRL2() & 0x07]; });

static std::vector<const char*> lna_priority_map = { "LNA2", "LNA" };
TCF(AGCLNAPrio, "LNA priority", "",    const char*,  { return lna_priority_map[(rt.AGCCTRL1() & 0x40) >> 6]; });

static std::vector<const char*> cs_rel_th_map = { "0", "+6", "+10", "+14" };
TCF(CSRelTh,    "CS rel T/H",   "dB",  const char*,  { return cs_rel_th_map[(rt.AGCCTRL1() & 0x30) >> 4]; });
TCF(CSAbsTh,    "CS abs T/H",   "dB",  int16_t,      {
  int16_t r = rt.AGCCTRL1() & 0x0F;
  if (r & 0x0008)
    r |= 0xFFF0;
  return r;
});

static std::vector<const char*> hyst_level_map = { "none", "low", "medium", "large" };
TCF(HystLevel,  "Hysteresis",   "",    const char*,  { return hyst_level_map[(rt.AGCCTRL0() & 0xC0) >> 6]; });

static std::vector<const char*> wait_time_map = { "8", "16", "24", "32" };
TCF(WaitTime,   "Wait time",    "S",   const char*,  { return wait_time_map[(rt.AGCCTRL0() & 0x30) >> 4]; });

static std::vector<const char*> agc_freeze_map = { "normal", "sync word", "man anlg", "manual" };
TCF(AGCFreeze,  "AGC freeze",   "",    const char*,  { return agc_freeze_map[(rt.AGCCTRL0() & 0x0C) >> 2]; });

static std::vector<const char*> filter_len_fsk_map = { "8", "16", "32", "64" };
static std::vector<const char*> filter_len_ask_map = { "4", "8", "12", "16" };
TCF(FilterLen,  "Filter len",   "",    const char*,  {
  int i = rt.AGCCTRL0() & 0x03;
  uint8_t modulation = (rt.MDMCFG2() & 0x70) >> 4;
  switch (modulation) {
  case 0:
  case 1:
  case 4:
  case 7:
    units = "S";
    return filter_len_fsk_map[i];
  case 3:
    units = "dB";
    return filter_len_ask_map[i];
  }
  units = "";
  return "?";
});

STG(OSCPD,      "RCPWR",  { return rt.WORCTRL() & 0x80; });

static std::vector<const char*> wor_e1_to_map = { "4", "6", "8", "12", "16", "24", "32", "48" };
TCF(Ev1TO,      "Event 1 T/O",   "cyc", const char*,   { return wor_e1_to_map[(rt.WORCTRL() & 0x70) >> 4]; });

STG(RCCal,      "RCCAL", { return rt.WORCTRL() & 0x08; });

static std::vector<const char*> wor_e0_res_map = { "1", "32", "1024", "32768" };
TCF(WORREs,     "Event 0 res",   "p",   const char*,   { return wor_e0_res_map[rt.WORCTRL() & 0x03]; });


TCF(LNACurrent, "LNA current",  "",    uint8_t,       { return rt.FREND1() >> 6; });
TCF(LNA2MixCur, "LNA2 mix cur", "",    uint8_t,       { return (rt.FREND1() && 0x30) >> 4; });
TCF(LODivBufCurRx, "RX LO cur", "",    uint8_t,       { return (rt.FREND1() && 0x0C) >> 2; });
TCF(MixerCur,   "Mixer cur",    "",    uint8_t,       { return rt.FREND1() && 0x03; });\

TCF(LODIVBufCurTx, "TX LO cur", "",    uint8_t,       { return (rt.FREND0() && 0x30) >> 4; });
TCF(PAPowerInx, "Pwr amp inx",  "",    uint8_t,       { return rt.FREND0() && 0x07; });


TCF(FSCal3_76,  "FS cal 3[7:6]", "",   uint8_t,       { return rt.FSCAL3() && 0xC0; });
TCF(FSCPCal,    "FS CP cal",    "",    uint8_t,       { return rt.FSCAL3() && 0x30; });
TCF(FSCal3,     "FS CP cur",    "",    uint8_t,       { return rt.FSCAL3() && 0x0F; });

static std::vector<const char*> fs_vco_map = { "low", "high" };
TCF(FSVCO,      "FS VCO",       "",    const char*,   { return fs_vco_map[(rt.FSCAL2() && 0x20) >> 5]; });
TCF(FSCal2,     "FS cal 2",     "",    uint8_t,       { return rt.FSCAL2() && 0x1F; });

TCF(FSCal1,     "FS cal 1",     "",    uint8_t,       { return rt.FSCAL1() && 0x3F; });
TCF(FSCal0,     "FS cal 0",     "",    uint8_t,       { return rt.FSCAL0() && 0x7F; });
TCF(FSTest,     "FS test",      "",    uint8_t,       { return rt.FSTEST(); });

TCF(RCCtrl,     "RC OSC cal",   "",    bytes_t,       { return mk_bytes(rt.RCCTRL1() & 0x7F, rt.RCCTRL0() & 0x7F); });

TCF(PTest,      "Production",   "",    uint8_t,       { return rt.PTEST(); });
TCF(AGCTest,    "AGC",          "",    uint8_t,       { return rt.AGCTEST(); });
TCF(Test,       "Various",      "",    bytes_t,       { return mk_bytes(rt.TEST2(), rt.TEST1(), rt.TEST0()); });

template<>
void ConfigField<uint8_t>::Update(bool full)
{
  Field<uint8_t>::Update(full);
};

template<>
void ConfigField<const char*>::Update(bool full) {
  Field<const char*>::Update(full);
};

template<>
void ConfigField<bool>::Update(bool full)
{
  Field<bool>::Update(full);
}

template<>
void ConfigField<double>::Update(bool full)
{
  Field<double>::Update(full);
}

template<>
void ConfigField<int8_t>::Update(bool full)
{
  Field<int8_t>::Update(full);
}

template<>
void ConfigField<uint16_t>::Update(bool full)
{
  Field<uint16_t>::Update(full);
}

template<>
void ConfigField<int16_t>::Update(bool full)
{
  Field<int16_t>::Update(full);
}

template<>
void ConfigField<bytes_t>::Update(bool full)
{
  Field<bytes_t>::Update(full);
}

void GDOField::Update(bool full)
{
  if (Field<uint8_t>::wndw) {
    int r = row, c = col, lcol = -1;
    if (full) {
      static char tmp2[256];
      snprintf(tmp, sizeof(tmp), "%%- %zds: ", key_width);
      snprintf(tmp2, sizeof(tmp2), tmp, "Status");
      mvwprintw(wndw, row, col, "%s", tmp2);
      colon_inx = c + strlen(tmp2);
    }
    size_t gc = colon_inx;
    for (size_t i = 0; i < 3; i++) {
      uint8_t iocfg = i == 0 ? rt.IOCFG0() : i == 1 ? rt.IOCFG1() : rt.IOCFG2();
      snprintf(tmp, sizeof(tmp), "%1s%02x", iocfg & 0x40 ? "~" : " ", iocfg & 0x3F);

      // uint8_t gdoi = rt.Device().rGDO(i);
      // if (gdoi != 0)
      //   lcol = COLOR_PAIR(digitalRead(gdoi) ? HIGH_PAIR : LOW_PAIR);

      if (lcol != -1) wattron(Field<uint8_t>::wndw, lcol);
      mvwprintw(Field<uint8_t>::wndw, r, gc, tmp);
      if (lcol != -1) wattroff(Field<uint8_t>::wndw, lcol);
      gc += strlen(tmp) + 1;
    }
  }
}

} // CC1101UIFields

using namespace CC1101UIFields;

CC1101UI::CC1101UI(std::shared_ptr<CC1101> cc1101) :
  UI()
{
  // const CC1101::Status &status = cc1101->StatusBuffer();

  devices.insert(cc1101);

  size_t row = 1, col = 1;

  Add(new TimeField(statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  row++;

  Add(new FREQESTField(row++, col, cc1101));
  Add(new LQIField(row++, col, cc1101));
  Add(new RSSIField(row++, col, cc1101));
  Add(new StateField(row++, col, cc1101));
  Add(new WORTimerField(row++, col, cc1101));

  Add(new CSStatusIndicator(row, col + 1, cc1101));
  Add(new PQTStatusIndicator(row, col + 4, cc1101));
  Add(new CCAStatusIndicator(row, col + 8, cc1101));
  Add(new CRCStatusIndicator(row, col + 12, cc1101));
  Add(new SFDStatusIndicator(row, col + 16, cc1101));
  Add(new PLLStatusIndicator(row, col + 20, cc1101));
  row++;

  Add(new GDO0StatusIndicator(row, col + 4, cc1101));
  Add(new GDO2StatusIndicator(row, col + 11, cc1101));
  row++;

  Add(new PLLCalField(row++, col, cc1101));
  Add(new NumTXRXField(row++, col, cc1101));
  Add(new XOSCCalField(row++, col, cc1101));

  row = 1;
  col += 26;
  Add(new Label(UI::statusp, row++, col, "GDO"));
  Add(new GDOField(row++, col, cc1101));
  Add(new GDODriveField(row++, col, cc1101));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col, "Radio"));
  Add(new LField<double>(UI::statusp, row++, col, 8, "Frequency", "MHz",
    [cc1101](){ return cc1101->rFrequency()/1e6; },
    [cc1101](const char *v) { cc1101->wFrequency(atof(v)); },
    [cc1101](){ cc1101->dec_frequency(); },
    [cc1101](){ cc1101->inc_frequency(); }));
  Add(new DeviationField(row++, col, cc1101));
  Add(new ModulationField(row++, col, cc1101));
  Add(new FOCPreKField(row++, col, cc1101));
  Add(new FOCPostKField(row++, col, cc1101));
  Add(new FOCLimitField(row++, col, cc1101));
  Add(new BSCSGateSttng(row++, col, cc1101));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col, "Modem"));
  Add(new FilterBWField(row++, col, cc1101));

  Add(new LField<double>(UI::statusp, row++, col, 8, "Data rate", "kBd",
    [cc1101](){ return cc1101->rDataRate()/1e3; },
    [cc1101](const char *v){ cc1101->wDatarate(atof(v)); },
    [cc1101](){ cc1101->dec_datarate(); },
    [cc1101](){ cc1101->inc_datarate(); }
  ));

  Add(new SyncModeField(row++, col, cc1101));
  Add(new SyncWordField(row++, col, cc1101));
  Add(new NPreambleField(row++, col, cc1101));
  Add(new ChSpacingField(row++, col, cc1101));
  Add(new DCBlockSttng(row, col, cc1101));
  Add(new ManchesterSttng(row, col + 4, cc1101));
  Add(new FECSttng(row++, col + 8, cc1101));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col, "Packet control"));
  Add(new PktModeField(row++, col, cc1101));
  Add(new PktLengthField(row++, col, cc1101));
  Add(new PktFormatField(row++, col, cc1101));
  Add(new PQEThreshField(row++, col, cc1101));
  Add(new AddrChkField(row++, col, cc1101));
  Add(new DataWhtngSttng(row, col, cc1101));
  Add(new CRCSttng(row, col + 4, cc1101));
  Add(new CRCAFSttng(row, col + 8, cc1101));
  Add(new AppendSttng(row++, col + 16, cc1101));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col, "Miscellaneous"));
  Add(new ATSSttng(row, col, cc1101));
  Add(new ADCRetSttng(row, col + 5, cc1101));
  Add(new PinRdoCtrlSttng(row, col + 12, cc1101));
  Add(new XOSCSleepSttng(row++, col + 20, cc1101));
  Add(new RXAttnField(row++, col, cc1101));
  Add(new FIFOTXRXField(row++, col, cc1101));

  Add(new AddressField(row++, col, cc1101));

  Add(new ChannelField(row++, col, cc1101));
  Add(new IFFreqField(row++, col, cc1101));
  Add(new FreqOSField(row++, col, cc1101));
  Add(new RXTermField(row, col, cc1101));
  Add(new RXTermRSSISttng(row, col + 13, cc1101));
  Add(new RXTermSPQISttng(row++, col + 18, cc1101));

  Add(new RXTOField(row++, col, cc1101));
  Add(new CCAModeField(row++, col, cc1101));
  Add(new RXOffMdField(row++, col, cc1101));
  Add(new TXOffMdField(row++, col, cc1101));
  Add(new FSAutocalField(row++, col, cc1101));
  Add(new POTOField(row++, col, cc1101));

  Add(new LNACurrentField(row++, col, cc1101));
  Add(new LNA2MixCurField(row++, col, cc1101));
  Add(new LODivBufCurRxField(row++, col, cc1101));
  Add(new MixerCurField(row++, col, cc1101));
  Add(new LODIVBufCurTxField(row++, col, cc1101));
  Add(new PAPowerInxField(row++, col, cc1101));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col, "Bit synchronization"));
  Add(new BSPreKIField(row++, col, cc1101));
  Add(new BSPreKPField(row++, col, cc1101));
  Add(new BSPostKIField(row++, col, cc1101));
  Add(new BSPostKPField(row++, col, cc1101));
  Add(new BSLimitField(row++, col, cc1101));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col, "Automatic gain control"));
  Add(new MaxDVGAGainField(row++, col, cc1101));
  Add(new MaxLNAGainField(row++, col, cc1101));
  Add(new MAGNTargetField(row++, col, cc1101));
  Add(new AGCLNAPrioField(row++, col, cc1101));
  Add(new CSRelThField(row++, col, cc1101));
  Add(new CSAbsThField(row++, col, cc1101));
  Add(new HystLevelField(row++, col, cc1101));
  Add(new WaitTimeField(row++, col, cc1101));
  Add(new AGCFreezeField(row++, col, cc1101));
  Add(new FilterLenField(row++, col, cc1101));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col, "Wake on radio"));
  Add(new WOREvent0Field(row++, col, cc1101));
  Add(new Ev1TOField(row++, col, cc1101));
  Add(new WORREsField(row++, col, cc1101));
  Add(new OSCPDSttng(row, col, cc1101));
  Add(new RCCalSttng(row++, col + 6, cc1101));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col, "Frequency synthesizer"));
  Add(new FSCal3_76Field(row++, col, cc1101));
  Add(new FSCPCalField(row++, col, cc1101));
  Add(new FSCal3Field(row++, col, cc1101));
  Add(new FSVCOField(row++, col, cc1101));
  Add(new FSCal2Field(row++, col, cc1101));
  Add(new FSCal1Field(row++, col, cc1101));
  Add(new FSCal0Field(row++, col, cc1101));
  Add(new FSTestField(row++, col, cc1101));
  Add(new RCCtrlField(row++, col, cc1101));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col, "Test registers"));
  Add(new PTestField(row++, col, cc1101));
  Add(new AGCTestField(row++, col, cc1101));
  Add(new TestField(row++, col, cc1101));
}

void CC1101UI::Layout()
{
  SkippingLayout(18);
};
