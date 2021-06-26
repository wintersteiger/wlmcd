// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>

#include "field_types.h"
#include "raw_ui.h"
#include "es9028pro.h"
#include "es9028pro_rt.h"
#include "es9028pro_ui.h"

static bool str_eq_insensitive(const std::string& a, const std::string& b)
{
  return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                    [](char a, char b) {
                      return tolower(a) == tolower(b);
                    });
}

ES9028PROUI::ES9028PROUI(std::shared_ptr<ES9028PRO> &es9028pro) : UI()
{
  devices.insert(es9028pro.get());

  int row = 1, col = 1;

  Add(new TimeField(statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  Add(new Empty(row++, col));

  auto* RTS = es9028pro->RTS;

  auto* main = &es9028pro->RTS->Main;

  static const char *auto_inputs[] = { "None", "I2S|DSD", "I2S|S/PDIF", "I2S|S/PDIF|DSD" };
  static const char *manual_inputs[] = { "I2S", "S/PDIF", "?", "DSD" };
  Add(new LField<const char*>(statusp, row++, col, 10, "Input", "",
  [main]() {
    uint8_t auto_input = main->auto_select();
    if (auto_input != 0)
      return auto_inputs[auto_input];
    else
      return manual_inputs[main->input_select()];
  },
  [main](const char* v) {
    if (str_eq_insensitive(v, "auto")) {
      main->Write(main->_rINPUT, main->_vauto_select, 3);
      return;
    }
    else {
      for (size_t i=0; i < 4; i++) {
        if (str_eq_insensitive(v, auto_inputs[i])) {
          main->Write(main->_rINPUT, main->_vauto_select, i);
          return;
        }
        else if (str_eq_insensitive(v, manual_inputs[i])) {
          uint8_t x = main->INPUT();
          main->Write(main->_rINPUT, (x & 0xF0) | i);
          return;
        }
      }
    }
    UI::Error("invalid value '%s'", v);
  }));

  Add(new LField<double>(statusp, row++, col, 10, "Sample rate", "kHz", [es9028pro]() {
    return es9028pro->FSR() / 1e3;
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Volume"));
  Add(new LField<bool>(statusp, row++, col, 10, "Mute", "", [RTS]() {
    return RTS->Main.mute();
  }));
  Add(new LField<double>(statusp, row++, col, 10, "Master trim", "%", [RTS]() {
    return (RTS->Main.master_trim() / (double)0x7FFFFFFF) * 100.0;
  }));
  Add(new LField<double>(statusp, row++, col, 10, "Channel 1", "dB", [RTS]() { return -0.5 * RTS->Main.volume1(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Channel 2", "dB", [RTS]() { return -0.5 * RTS->Main.volume2(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Channel 3", "dB", [RTS]() { return -0.5 * RTS->Main.volume3(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Channel 4", "dB", [RTS]() { return -0.5 * RTS->Main.volume4(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Channel 5", "dB", [RTS]() { return -0.5 * RTS->Main.volume5(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Channel 6", "dB", [RTS]() { return -0.5 * RTS->Main.volume6(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Channel 7", "dB", [RTS]() { return -0.5 * RTS->Main.volume7(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Channel 8", "dB", [RTS]() { return -0.5 * RTS->Main.volume8(); } ));
  Add(new LField<double>(statusp, row++, col, 10, "Ramp rate", "dB/s", [es9028pro]() {
    return es9028pro->VolumeRampRate();
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Clock"));
  Add(new LField<double>(statusp, row++, col, 10, "Frequency", "MHz", [es9028pro]() {
    return es9028pro->CLK_FREQ() / 1e6;
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "Drive", "", [RTS]() {
    static const char *values[] = { "full", "?", "?", "?", "?", "?", "?", "?", "3/4 bias", "?", "?", "?", "1/2 bias", "?", "1/4 bias", "off" };
    return values[RTS->Main.osc_drv()];
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "Divider", "", [RTS]() {
    static const char *values[] = { "XIN", "XIN/2", "XIN/4", "XIN/8" };
    return values[RTS->Main.clk_gear()];
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Serial"));
  Add(new LField<const char*>(statusp, row++, col, 10, "Bit width", "bit", [RTS]() {
    static const char *values[] = { "16", "24", "32", "32" };
    return values[RTS->Main.serial_bits()];
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "Length", "bit", [RTS]() {
    static const char *values[] = { "16", "24", "32", "32" };
    return values[RTS->Main.serial_length()];
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "Serial mode", "", [RTS]() {
    static const char *values[] = { "I2S", "Left-just.", "Right-just.", "Right-just." };
    return values[RTS->Main.serial_mode()];
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "S/PDIF"));
  Add(new LField<const char*>(statusp, row++, col, 10, "Status word", "", [RTS]() {
    static const char *values[] = { "consumer", "pro/AES3" };
    return values[RTS->Main.user_bits()];
  }));
  Add(new LField<bool>(statusp, row++, col, 10, "Ign. data flg", "", [RTS]() {
    return RTS->Main.spdif_ig_data();
  }));
  Add(new LField<bool>(statusp, row++, col, 10, "Ign. valid flg", "", [RTS]() {
    return RTS->Main.spdif_ig_valid();
  }));
  Add(new LField<bool>(statusp, row++, col, 10, "Auto de-emph", "", [RTS]() {
    return RTS->Main.auto_deemph();
  }));
  Add(new LField<bool>(statusp, row++, col, 10, "De-emph bypass", "", [RTS]() {
    return RTS->Main.deemph_bypass();
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "De-emph filter", "kHz", [RTS]() {
    const char* values[] = { "32", "44.1", "48", "?" };
    return values[RTS->Main.deemph_sel()];
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "Input pin", "", [RTS]() {
    const char* values[] = { "DATA_CLK", "DATA1", "DATA2", "DATA3", "Data4", "Data5", "Data6", "Data7", "Data8", "GPIO1", "GPIO2", "GPIO3", "GPIO4", "?", "?", "?" };
    return values[RTS->Main.spdif_sel()];
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Automute"));
  Add(new LField<const char*>(statusp, row++, col, 10, "Config", "", [RTS]() {
    static const char *values[] = { "Normal", "Mute", "Ramp", "Mute, Ramp" };
    return values[RTS->Main.automute_config()];
  }));
  Add(new LField<double>(statusp, row++, col, 10, "Time", "sec", [es9028pro]() {
    return 2096896.0 / (es9028pro->RTS->Main.automute_time() * es9028pro->FSR());
  }));
  Add(new LField<double>(statusp, row++, col, 10, "Level", "dB", [es9028pro]() {
    return - (double) es9028pro->RTS->Main.automute_level();
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Filters"));
  Add(new LField<const char*>(statusp, row++, col, 10, "FIR interpol.", "", [RTS]() {
    const char* values[] = { "fast r/o, lin", "slow r/o, lin", "fast r/o, min", "slow r/o, min", "?", "apodizing", "hybrid", "brickwall" };
    return values[RTS->Main.filter_shape()];
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "IIR interpol.", "fs", [RTS]() {
    const char* values[] = { "1.0757", "1.1338", "1.3605", "1.5873" };
    return values[RTS->Main.iir_bw()];
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Jitter eliminator & DPLL"));
  Add(new LField<bool>(statusp, row++, col, 10, "Enabled", "", [RTS]() {
    return RTS->Main.jitterelim_en();
  }));
  Add(new LField<uint8_t>(statusp, row++, col, 10, "DPLL B/W Ser.", "/15", [RTS]() {
    return RTS->Main.dpll_bw_serial();
  }));
  Add(new LField<uint8_t>(statusp, row++, col, 10, "DPLL B/W DSD", "/15", [RTS]() {
    return RTS->Main.dpll_bw_dsd();
  }));
  Add(new LField<bool>(statusp, row++, col, 10, "Dither", "", [RTS]() {
    return RTS->Main.ns_dither_enb();
  }));
  Add(new LField<bool>(statusp, row++, col, 10, "THD comp.", "", [RTS]() {
    return RTS->Main.thd_enb();
  }));
  Add(new Empty(row++, col));

  Add(new Label(UI::statusp, row++, col + 18, "Soft-Start"));
  Add(new LField<const char*>(statusp, row++, col, 10, "Ramp to", "", [RTS]() {
    const char *values[] = { "Ground", "AVCC/2" };
    return values[RTS->Main.soft_start()];
  }));
  Add(new LField<double>(statusp, row++, col, 10, "Time", "s", [es9028pro]() {
    return es9028pro->SoftStartTime();
  }));
  Add(new LField<bool>(statusp, row++, col, 10, "Soft stop", "", [RTS]() {
    return RTS->Main.soft_stop_on_unlock();
  }));
  Add(new Empty(row++, col));
}

ES9028PROUI::~ES9028PROUI() {}

std::shared_ptr<UI> make_es9028pro_raw_ui(std::shared_ptr<ES9028PRO> &es9028pro)
{
  return make_raw_ui<ES9028PRO, uint8_t, uint8_t>(es9028pro, es9028pro->RTS->Main);
}