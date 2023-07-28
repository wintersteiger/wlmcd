// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <memory>

#include "serialization.h"
#include "field_types.h"
#include "raw_ui.h"
#include "s2lp.h"
#include "s2lp_rt.h"
#include "gpio_button.h"
#include "gpio_button_field.h"

#include "s2lp_ui.h"

#define EMPTY() fields.push_back(new Empty(row++, col));

S2LPUI::S2LPUI(std::shared_ptr<S2LP> s2lp, const uint32_t &irqs)
{
  WINDOW *w = UI::statusp;
  int row = 1, col = 1;

  devices.insert(s2lp);


  Add(new TimeField(w, row, col));
  Add(new Label(w, row++, col + 18, s2lp->Name()));
  EMPTY();

  S2LP::RegisterTable *rt = s2lp->RT;
  const uint8_t* sb = s2lp->StatusBytes();
  auto& sp = s2lp;

  Add(new LWarningIndicator(w, row, col, "TXFIFO", [sb](){ return sb[0] & 0x04; }));
  Add(new LWarningIndicator(w, row, col + 7, "RXFIFO", [sb](){ return (sb[0] & 0x02) == 0; }));
  Add(new LWarningIndicator(w, row, col + 14, "ERR", [sb](){ return sb[0] & 0x01; }));
  Add(new LEnabledIndicator(w, row++, col + 18, "XO", [sb](){ return sb[1] & 0x01; }));
  EMPTY();

  Add(new LField<const char*>(w, row++, col, 8, "State", "",
    [sp, sb](){
      if (sp->Responsive()) {
        switch (sb[1] >> 1) {
          case 0x02: return "Standby";
          case 0x01: return "Sleep A";
          case 0x03: return "Sleep B";
          case 0x00: return "Ready";
          case 0x14: return "Lock ST";
          case 0x0C: return "Lock on";
          case 0x30: return "RX";
          case 0x5C: return "TX";
          case 0x50: return "Synth setup";
          case 0x7C: return "Wait sleep";
          default: return "?";
        }
      } else {
        return "* N/C *";
      };
  }));
  Add(new LField<uint8_t>(w, row++, col, 8, "Antenna", "", [sb](){ return sb[0] & 0x08; }));
  Add(new LField<std::string>(w, row++, col, 8, "FIFO TX/RX",  "B", [rt](){
    auto tx = rt->NELEM_TXFIFO();
    auto rx = rt->NELEM_RXFIFO();
    return std::to_string(tx) + "/" + std::to_string(rx);
  }));
  EMPTY();

  Add(new Label(w, row, col, "RX:"));
  Add(new LEnabledIndicator(w, row, col + 4, "DAT", [&irqs](){ return irqs & 1 << 0; }));
  Add(new LEnabledIndicator(w, row, col + 8, "DIS", [&irqs](){ return irqs & 1 << 1; }));
  Add(new LEnabledIndicator(w, row, col + 12, "U/F", [&irqs](){ return irqs & 1 << 6; }));
  Add(new LEnabledIndicator(w, row, col + 16, "A/F", [&irqs](){ return irqs & 1 << 9; }));
  Add(new LEnabledIndicator(w, row++, col + 20, "A/E", [&irqs](){ return irqs & 1 << 10; }));

  Add(new Label(w, row, col, "TX:"));
  Add(new LEnabledIndicator(w, row, col + 4, "DAT", [&irqs](){ return irqs & 1 << 2; }));
  Add(new LEnabledIndicator(w, row, col + 8, "MAX", [&irqs](){ return irqs & 1 << 3; }));
  Add(new LEnabledIndicator(w, row, col + 12, "O/F", [&irqs](){ return irqs & 1 << 5; }));
  Add(new LEnabledIndicator(w, row, col + 16, "A/F", [&irqs](){ return irqs & 1 << 7; }));
  Add(new LEnabledIndicator(w, row++, col + 20, "A/F", [&irqs](){ return irqs & 1 << 8; }));

  Add(new Label(w, row, col, "SG:"));
  Add(new LEnabledIndicator(w, row, col + 4, "BCK", [&irqs](){ return irqs & 1 << 11; }));
  Add(new LEnabledIndicator(w, row, col + 8, "PRE", [&irqs](){ return irqs & 1 << 12; }));
  Add(new LEnabledIndicator(w, row, col + 12, "SYN", [&irqs](){ return irqs & 1 << 13; }));
  Add(new LEnabledIndicator(w, row++, col + 16, "CS", [&irqs](){ return irqs & 1 << 14; }));

  Add(new Label(w, row, col, "ST:"));
  Add(new LEnabledIndicator(w, row, col + 4, "WKE", [&irqs](){ return irqs & 1 << 15; }));
  Add(new LEnabledIndicator(w, row, col + 8, "RDY", [&irqs](){ return irqs & 1 << 16; }));
  Add(new LEnabledIndicator(w, row, col + 12, "SBY", [&irqs](){ return irqs & 1 << 17; }));
  Add(new LEnabledIndicator(w, row++, col + 16, "BAT", [&irqs](){ return irqs & 1 << 18; }));
  Add(new LEnabledIndicator(w, row, col + 4, "POR", [&irqs](){ return irqs & 1 << 19; }));
  Add(new LEnabledIndicator(w, row, col + 8, "BWN", [&irqs](){ return irqs & 1 << 20; }));
  Add(new LEnabledIndicator(w, row++, col + 12, "LCK", [&irqs](){ return irqs & 1 << 21; }));

  Add(new LEnabledIndicator(w, row, col + 4, "CRC", [&irqs](){ return irqs & 1 << 4; }));
  Add(new LEnabledIndicator(w, row, col + 8, "TMR", [&irqs](){ return irqs & 1 << 29; }));
  Add(new LEnabledIndicator(w, row++, col + 12, "AES", [&irqs](){ return irqs & 1 << 30; }));

  row = 0; col += 26;
  static const char *gpio_modes[] = { "analog", "digital in", "dig out low", "dig out high" };
  static const char *gpio_confs[] = {
    "nIRQ", "nPOR", "w/u timer", "low bat", "TX clock", "enter TX", "TX FIFO low", "TX FIFO high", "RX data",
    "RX clock", "enter RX", "RX FIFO high", "RX FIFO low", "antenna div", "preamble", "sync word", "RSSI above",
    "MCU clock", "TX/RX ind", "VDD", "GND", "ext SMPS", "slp/stdby", "not slp/stdby", "lock", "wait lck high",
    "wait lck tmr", "wait READY2", "wait PM tmr", "wait VCO cal", "SYNTH", "?" };
  Add(new Label(w, row++, col, "GPIO"));
  Add(new LField<const char*>(w, row++, col, 13, "GPIO0 Mode", "", [rt](){ return gpio_modes[rt->GPIO0_MODE()]; }));
  Add(new LField<const char*>(w, row++, col, 13, "GPIO0 Config", "", [rt](){ return gpio_confs[rt->GPIO0_SELECT()]; }));
  Add(new LField<const char*>(w, row++, col, 13, "GPIO1 Mode", "", [rt](){ return gpio_modes[rt->GPIO1_MODE()]; }));
  Add(new LField<const char*>(w, row++, col, 13, "GPIO1 Config", "", [rt](){ return gpio_confs[rt->GPIO1_SELECT()]; }));
  Add(new LField<const char*>(w, row++, col, 13, "GPIO2 Mode", "", [rt](){ return gpio_modes[rt->GPIO2_MODE()]; }));
  Add(new LField<const char*>(w, row++, col, 13, "GPIO2 Config", "", [rt](){ return gpio_confs[rt->GPIO2_SELECT()]; }));
  Add(new LField<const char*>(w, row++, col, 13, "GPIO3 Mode", "", [rt](){ return gpio_modes[rt->GPIO3_MODE()]; }));
  Add(new LField<const char*>(w, row++, col, 13, "GPIO3 Config", "", [rt](){ return gpio_confs[rt->GPIO3_SELECT()]; }));
  EMPTY();

  Add(new Label(w, row++, col, "Radio"));
  Add(new LField<double>(w, row++, col, 8, "Frequency", "MHz",
    [sp](){ return sp->rFrequency() / 1e6; },
    [sp](const char *v){ return sp->wFrequency(atof(v)); }
  ));
  Add(new LField<double>(w, row++, col, 8, "Deviation", "kHz", [sp](){ return sp->rDeviation() / 1e3; }));
  Add(new LField<double>(w, row++, col, 8, "Filter B/W", "kHz", [sp](){ return sp->rFilterBandwidth(); }));
  Add(new LField<double>(w, row++, col, 8, "RSSI T/H", "dBm", [sp](){ return sp->rRSSIThreshold() - 146.0; }));
  EMPTY();

  Add(new Label(w, row++, col, "Modem"));
  Add(new LField<const char*>(w, row++, col, 8, "Modulation", "", [rt](){
    static const char *values[] = { "2-FSK", "GFSK", "OOK", "MSK" };
    auto mi = rt->MOD_TYPE();
    if (mi == 2 && rt->PA_RAMP_EN()) return "ASK";
    else return values[mi];
  }));
  Add(new LField<double>(w, row++, col, 8, "Data rate", "kBd",
    [sp](){ return sp->rDatarate() / 1e3; },
    [sp](const char *v){ return sp->wDatarate(atof(v)); }));
  EMPTY();

  Add(new Label(w, row++, col, "Packet control"));
  Add(new LField<const char*>(w, row++, col, 8, "Pkt format", "", [rt](){
    static const char *values[] = { "basic", "802.15.4g", "UART OTA", "STack" };
    return values[rt->PCKT_FRMT()]; }));
  Add(new LField<const char*>(w, row++, col, 8, "Pkt mode", "", [rt](){
    static const char *values[] = { "fixed", "variable" };
    return values[rt->FIX_VAR_LEN()]; }));
  Add(new LField<uint8_t>(w, row++, col, 8, "# preamble", "b", [rt](){ return 2 * (rt->PREAMBLE_LEN_9_8() << 8) | rt->PREAMBLE_LEN_7_0(); }));
  Add(new LField<uint8_t>(w, row++, col, 8, "# sync", "b", [rt](){ return rt->SYNC_LEN(); }));
  Add(new LField<std::string>(w, row++, col, 8, "sync word", "",
    [rt](){
      std::array<uint8_t, 4> sw = { rt->SYNC1(), rt->SYNC2(), rt->SYNC3(), rt->SYNC4() };
      return to_hex(sw);
  }));
  Add(new LField<const char*>(w, row++, col, 8, "CRC mode", "", [rt](){
    static const char *values[] = { "None", "0x07", "0x8005", "0x1021", "0x864CBF", "?" };
    return values[rt->CRC_MODE()]; }));
  Add(new LField<uint16_t>(w, row++, col, 8, "Pkt length", "b", [rt](){ return rt->PCKTLEN1() << 8 | rt->PCKTLEN0(); }));

  Add(new LField<const char*>(w, row++, col, 8, "RX mode", "", [rt](){
    static const char *values[] = { "normal", "dct FIFO", "dct GPIO", "?" };
    return values[rt->RX_MODE()]; }));
  Add(new LField<const char*>(w, row++, col, 8, "TX source", "", [rt](){
    static const char *values[] = { "normal", "dct FIFO", "dct GPIO", "PN9" };
    return values[rt->TXSOURCE()]; }));

  Add(new LEnabledIndicator(w, row, col, "FEC", [rt](){ return rt->FEC_EN(); }));
  Add(new LEnabledIndicator(w, row++, col + 4, "WHT", [rt](){ return rt->WHIT_EN(); }));
  EMPTY();

  Add(new Label(w, row++, col, "Quality"));
  Add(new LField<uint32_t>(w, row++, col, 8, "SQI T/H", "b", [rt](){
    return rt->SYNC_LEN() - 2 * rt->SQI_TH(); }));
  Add(new LField<uint32_t>(w, row++, col, 8, "PQI T/H", "", [rt](){
    return 4 * rt->PQI_TH(); }));
  Add(new LEnabledIndicator(w, row++, col + 4, "SQI", [rt](){ return rt->SQI_EN(); }));
}

S2LPUI::~S2LPUI() {}

void S2LPUI::Layout()
{
  SkippingLayout(40);
};


std::shared_ptr<UI> make_s2lp_raw_ui(std::shared_ptr<S2LP> &device, std::shared_ptr<GPIOButton> reset_button)
{
  auto ui = make_raw_ui<S2LP, uint8_t, uint8_t>(device, *device->RT);

  if (reset_button)
    ui->Add(new GPIOButtonField(UI::statusp, 0, 0, "RESET", reset_button));

  return ui;
}
