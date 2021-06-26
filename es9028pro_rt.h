// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ES9028PRO_RT_H_
#define _ES9028PRO_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE_SET(ES9028PRO, ES9028PRO, RegisterTableSet,

  REGISTER_TABLE_SET_TABLE_W(ES9028PRO, Main, uint8_t, uint8_t,
    REG(SYSREG, "System Registers", 0x00, RW,                                 "System Registers",
      VAR(SYSREG, osc_drv, "osc_drv", 0xF0, RW,                               "");
      VAR(SYSREG, clk_gear, "clk_gear", 0x0C, RW,                             "");
      VAR(SYSREG, soft_reset, "soft_reset", 0x01, RW,                         "");
    );
    REG(INPUT, "Input Selection", 0x01, RW,                                   "Input Selection",
      VAR(INPUT, user_bits, "user_bits", 0x80, RW,                            "");
      VAR(INPUT, spdif_ig_data, "spdif_ig_data", 0x40, RW,                    "");
      VAR(INPUT, spdif_ig_valid, "spdif_ig_valid", 0x20, RW,                  "");
      VAR(INPUT, auto_select, "auto_select", 0x0C, RW,                        "");
      VAR(INPUT, input_select, "input_select", 0x03, RW,                      "");
    );
    REG(SCAE, "SCAE", 0x02, RW,                                               "Serial Data Configuration and Automute Enable",
      VAR(SCAE, automute_config, "automute_config", 0xC0, RW,                 "");
      VAR(SCAE, serial_bits, "serial_bits", 0x30, RW,                         "");
      VAR(SCAE, serial_length, "serial_length", 0x0C, RW,                     "");
      VAR(SCAE, serial_mode, "serial_mode", 0x03, RW,                         "");
    );
    // 0x03 reserved
    REG(AMTIME, "AMTIME", 0x04, RW,                                           "Automute Time",
      VAR(AMTIME, automute_time, "automute_time", 0xFF, RW,                   "");
    );
    REG(AMLEVEL, "AMLEVEL", 0x05, RW,                                         "Automute Level",
      VAR(AMLEVEL, automute_level, "automute_level", 0xFF, RW,                "");
    );
    REG(DEEMPHVOL, "DEEMPHVOL", 0x06, RW,                                     "De-emphasis Filter & Volume RampRate",
      VAR(DEEMPHVOL, auto_deemph, "auto_deemph", 0x80, RW,                    "");
      VAR(DEEMPHVOL, deemph_bypass, "deemph_bypass", 0x40, RW,                "");
      VAR(DEEMPHVOL, deemph_sel, "deemph_sel", 0x30, RW,                      "");
      VAR(DEEMPHVOL, volume_rate, "volume_rate", 0x07, RW,                    "");
    );
    REG(FBWSMTE, "FBWSMTE", 0x07, RW,                                         "Filter Bandwidth and System Mute",
      VAR(FBWSMTE, filter_shape, "filter_shape", 0xE0, RW,                    "");
      VAR(FBWSMTE, iir_bw, "iir_bw", 0x06, RW,                                "");
      VAR(FBWSMTE, mute, "mute", 0x01, RW,                                    "");
    );
    REG(GPIO12, "GPIO12", 0x08, RW,                                           "GPIO1-2 Configuration",
      VAR(GPIO12, gpio2_cfg, "gpio2_cfg", 0xF0, RW,                           "");
      VAR(GPIO12, gpio1_cfg, "gpio1_cfg", 0x0F, RW,                           "");
    );
    REG(GPIO34, "GPIO34", 0x09, RW,                                           "GPIO3-4 Configuration",
      VAR(GPIO34, gpio4_cfg, "gpio4_cfg", 0xF0, RW,                           "");
      VAR(GPIO34, gpio3_cfg, "gpio3_cfg", 0x0F, RW,                           "");
    );
    REG(MSTRSYNC, "MSTRSYNC", 0x0A, RW,                                       "Master Mode and Sync Configuration",
      VAR(MSTRSYNC, master_mode, "master_mode", 0x80, RW,                     "");
      VAR(MSTRSYNC, master_div, "master_div", 0x60, RW,                       "");
      VAR(MSTRSYNC, _128fs_mode, "128fs_mode", 0x10, RW,                      "");
      VAR(MSTRSYNC, lock_speed, "lock_speed", 0x0F, RW,                       "");
    );
    REG(SPDIFMUXGPIO, "SPDIFMUXGPIO", 0x0B, RW,                               "SPDIF Mux and GPIO Inversion",
      VAR(SPDIFMUXGPIO, spdif_sel, "spdif_sel", 0xF0, RW,                     "");
      VAR(SPDIFMUXGPIO, invert_gpio, "invert_gpio", 0x0F, RW,                 "");
    );
    REG(JTTRELMDPLL1, "JTTRELMDPLL1", 0x0C, RW,                               "Jitter Eliminator and DPLL Bandwidth",
      VAR(JTTRELMDPLL1, dpll_bw_serial, "dpll_bw_serial", 0xF0, RW,           "");
      VAR(JTTRELMDPLL1, dpll_bw_dsd, "dpll_bw_dsd", 0x0F, RW,                 "");
    );
    REG(JTTRELMDPLL2, "JTTRELMDPLL2", 0x0D, RW,                               "Jitter Eliminator / DPLL Configuration & THD Bypass",
      VAR(JTTRELMDPLL2, ns_dither_enb, "ns_dither_enb", 0x80, RW,             "");
      VAR(JTTRELMDPLL2, thd_enb, "thd_enb", 0x40, RW,                         "");
      VAR(JTTRELMDPLL2, jitterelim_en, "jitterelim_en", 0x20, RW,             "");
    );
    REG(SOFTSTART, "SOFTSTART", 0x0E, RW,                                     "Soft-Start Configuration",
      VAR(SOFTSTART, soft_start, "soft_start", 0x80, RW,                      "");
      VAR(SOFTSTART, soft_stop_on_unlock, "soft_stop_on_unlock", 0x40, RW,    "");
      VAR(SOFTSTART, soft_start_time, "soft_start_time", 0x1F, RW,            "");
    );
    REG(GPIOINPUT, "GPIOINPUT", 0x0F, RW,                                     "GPIO Input Selection & Volume Configuration",
      VAR(GPIOINPUT, gpio_sel2, "gpio_sel2", 0xC0, RW,                        "");
      VAR(GPIOINPUT, gpio_sel1, "gpio_sel1", 0x30, RW,                        "");
      VAR(GPIOINPUT, stereo_mode, "stereo_mode", 0x04, RW,                    "");
      VAR(GPIOINPUT, ch1_vol, "ch1_vol", 0x02, RW,                            "");
      VAR(GPIOINPUT, latch_vol, "latch_vol", 0x01, RW,                        "");
    );

    #define EIGHT(X) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8)
    #define VOLUME(X) \
      REG(VOLUME##X, "VOLUME"#X, 0x10 + (X) - 1, RW, "Volume "#X, \
        VAR(VOLUME##X, volume##X, "volume" #X, 0xFF, RW, ""));
    EIGHT(VOLUME);

    REG(MSTRTRIM_7_0, "MSTRTRIM[7:0]", 0x18, RW,                              "Master Trim 0 [7:0]",);
    REG(MSTRTRIM_15_8, "MSTRTRIM[15:8]", 0x19, RW,                            "Master Trim 1 [15:8]",);
    REG(MSTRTRIM_23_16, "MSTRTRIM[23:16]", 0x1A, RW,                          "Master Trim 2 [23:16]",);
    REG(MSTRTRIM_31_24, "MSTRTRIM[31:24]", 0x1B, RW,                          "Master Trim 3 [31:24]",);
    int32_t master_trim() const { return (MSTRTRIM_31_24() << 24) | (MSTRTRIM_23_16()) << 16 | (MSTRTRIM_15_8() << 8) | MSTRTRIM_7_0(); }

    REG(THDCOMP_C2_7_0, "THDCOMP_C2[7:0]", 0x1C, RW,                          "THD Compensation C2 [7:0]",);
    REG(THDCOMP_C2_15_8, "THDCOMP_C2[15:8]", 0x1D, RW,                        "THD Compensation C2 [15:8]",);
    int16_t thd_comp_c2() const { return THDCOMP_C2_15_8() << 8 | THDCOMP_C2_15_8(); }

    REG(THDCOMP_C3_7_0, "THDCOMP_C3[7:0]", 0x1E, RW,                          "THD Compensation C3 [7:0]",);
    REG(THDCOMP_C3_15_8, "THDCOMP_C3[15:8]", 0x1F, RW,                        "THD Compensation C3 [15:8]",);
    int16_t thd_comp_c3() const { return THDCOMP_C3_15_8() << 8 | THDCOMP_C3_7_0(); }

    REG(FIRRAMADDR, "FIRRAMADDR", 0x20, RW,                                   "Programmable FIR RAM Address",
      VAR(FIRRAMADDR, coeff_stage, "coeff_stage", 0x80, RW,                   "");
      VAR(FIRRAMADDR, coeff_addr, "coeff_addr", 0x7F, RW,                     "");
    );

    REG(FIRRAMDATA_7_0, "FIRRAMDATA[7:0]", 0x21, RW,                          "Programmable FIR RAM Data [7:0]",);
    REG(FIRRAMDATA_15_8, "FIRRAMDATA[15:8]", 0x22, RW,                        "Programmable FIR RAM Data [15:8]",);
    REG(FIRRAMDATA_23_16, "FIRRAMDATA[23:16]", 0x23, RW,                      "Programmable FIR RAM Data [23:16]",);
    REG(FIRRAMDATA_31_24, "FIRRAMDATA[31:24]", 0x24, RW,                      "Programmable FIR RAM Data [31:24]",);
    int32_t coeff_data() const { return (FIRRAMDATA_31_24() << 24) | (FIRRAMDATA_23_16()) << 16 | (FIRRAMDATA_15_8() << 8) | FIRRAMDATA_7_0(); }

    REG(FIRCONFIG, "FIRCONFIG", 0x25, RW,                                     "Programmable FIR Configuration",
      VAR(FIRCONFIG, bypass_osf, "bypass_osf", 0x80, RW,                      "");
      VAR(FIRCONFIG, filter_length, "filter_length", 0x10, RW,                "");
      VAR(FIRCONFIG, prog_ext, "prog_ext", 0x08, RW,                          "");
      VAR(FIRCONFIG, stage2_eve, "stage2_eve", 0x04, RW,                      "");
      VAR(FIRCONFIG, prog_we, "prog_we", 0x02, RW,                            "");
      VAR(FIRCONFIG, prog_en, "prog_en", 0x01, RW,                            "");
    );
    REG(DAC12MAP, "DAC12MAP", 0x26, RW,                                       "DAC 1-2 Mapping",
      VAR(DAC12MAP, ch2_map, "ch2_map", 0xF0, RW,                             "");
      VAR(DAC12MAP, ch1_map, "ch1_map", 0x0F, RW,                             "");
    );
    REG(DAC34MAP, "DAC34MAP", 0x27, RW,                                       "DAC 3-4 Mapping",
      VAR(DAC34MAP, ch4_map, "ch4_map", 0xF0, RW,                             "");
      VAR(DAC34MAP, ch3_map, "ch3_map", 0x0F, RW,                             "");
    );
    REG(DAC56MAP, "DAC56MAP", 0x28, RW,                                       "DAC 5-6 Mapping",
      VAR(DAC56MAP, ch6_map, "ch6_map", 0xF0, RW,                             "");
      VAR(DAC56MAP, ch5_map, "ch5_map", 0x0F, RW,                             "");
    );
    REG(DAC78MAP, "DAC78MAP", 0x29, RW,                                       "DAC 7-8 Mapping",
      VAR(DAC78MAP, ch8_map, "ch8_map", 0xF0, RW,                             "");
      VAR(DAC78MAP, ch7_map, "ch7_map", 0x0F, RW,                             "");
    );

    REG(PROGNCO_7_0, "PROGNCO[7:0]", 0x2A, RW,                                "Programmable NCO [7:0]",);
    REG(PROGNCO_15_8, "PROGNCO[15:8]", 0x2B, RW,                              "Programmable NCO [15:8]",);
    REG(PROGNCO_23_16, "PROGNCO[23:16]", 0x2C, RW,                            "Programmable NCO [23:16]",);
    REG(PROGNCO_31_24, "PROGNCO[31:24]", 0x2D, RW,                            "Programmable NCO [31:24]",);
    int32_t nco_num() const { return (PROGNCO_31_24() << 24) | (PROGNCO_23_16()) << 16 | (PROGNCO_15_8() << 8) | PROGNCO_7_0(); }

    REG(ADCCFG, "ADCCFG", 0x2E, RW,                                           "ADC Configuration",
      VAR(ADCCFG, adc_first_orderb, "adc_first_orderb", 0xC0, RW,             "");
      VAR(ADCCFG, adc_clk_sel, "adc_clk_sel", 0x30, RW,                       "");
      VAR(ADCCFG, adc_dither_enb, "adc_dither_enb", 0x0C, RW,                 "");
      VAR(ADCCFG, adc_pdb, "adc_pdb", 0x03, RW,                               "");
    );

    REG(ADCFLTFTR_7_0, "ADCFLTFTR[7:0]", 0x2F, RW,                            "ADC Configuration FTR Scale [7:0]",);
    REG(ADCFLTFTR_15_8, "ADCFLTFTR[15:8]", 0x30, RW,                          "ADC Configuration FTR Scale [15:8]",);
    uint16_t adc_ftr_scale() const { return ADCFLTFTR_15_8() << 8 | ADCFLTFTR_7_0(); }

    REG(ADCFLTFBQ1_7_0, "ADCFLTFBQ1[7:0]", 0x31, RW,                          "ADC Configuration FBQ Scale 1 [7:0]",);
    REG(ADCFLTFBQ1_15_8, "ADCFLTFBQ1[15:8]", 0x32, RW,                        "ADC Configuration FBQ Scale 1 [15:8]",);
    uint16_t adc_fbq_scale1() const { return ADCFLTFBQ1_15_8() << 8 | ADCFLTFBQ1_7_0(); }

    REG(ADCFLTFBQ2_7_0, "ADCFLTFBQ2[7:0]", 0x33, RW,                          "ADC Configuration FBQ Scale 2 [7:0]",);
    REG(ADCFLTFBQ2_15_8, "ADCFLTFBQ2[15:8]", 0x34, RW,                        "ADC Configuration FBQ Scale 2 [15:8]",);
    uint16_t adc_fbq_scale2() const { return ADCFLTFBQ2_15_8() << 8 | ADCFLTFBQ2_7_0(); }

    // 0x35 Reserved

    REG(R54, "R54", 0x36, RW,                                                 "Reserved 54",
      VAR(R54, dop_bypass, "dop_bypass", 0x80, RW,                            "");
    );

    // 0x37-0x3D Reserved

    REG(CHANGAIN, "CHANGAIN", 0x3E, RW,                                       "+18dB Channel Gain",
      VAR(CHANGAIN, data8_gain, "data8_gain", 0x80, RW,                       "");
      VAR(CHANGAIN, data7_gain, "data7_gain", 0x40, RW,                       "");
      VAR(CHANGAIN, data6_gain, "data6_gain", 0x20, RW,                       "");
      VAR(CHANGAIN, data5_gain, "data5_gain", 0x10, RW,                       "");
      VAR(CHANGAIN, data4_gain, "data4_gain", 0x08, RW,                       "");
      VAR(CHANGAIN, data3_gain, "data3_gain", 0x04, RW,                       "");
      VAR(CHANGAIN, data2_gain, "data2_gain", 0x02, RW,                       "");
      VAR(CHANGAIN, data1_gain, "data1_gain", 0x01, RW,                       "");
    );

    REG(AUTOCALMOD, "AUTOCALMOD", 0x3F, RW,                                   "Auto Calibration and Modulator Configuration",
      VAR(AUTOCALMOD, calib_en, "calib_en", 0x80, RW,                         "");
      VAR(AUTOCALMOD, calib_sel, "calib_sel", 0x40, RW,                       "");
      VAR(AUTOCALMOD, calib_latch, "calib_latch", 0x20, RW,                   "");
      VAR(AUTOCALMOD, mod_stable, "mod_stable", 0x0C, RW,                     "Must be set to 2'b10 to ensure the noise shaped modulatoris stable.");
    );

    REG(CHIPIDSTATUS, "CHIPIDSTATUS", 0x40, RO,                                "Chip ID and Status",
      VAR(CHIPIDSTATUS, chip_id, "chip_id", 0xFC, RO,                         "");
      VAR(CHIPIDSTATUS, automute_status, "automute_status", 0x2, RO,          "");
      VAR(CHIPIDSTATUS, lock_status, "lock_status", 0x01, RO,                 "");
    );

    REG(GPIORDBK, "GPIORDBK", 0x41, RO,                                       "GPIO Readback",
      VAR(GPIORDBK, gpio4, "gpio4", 0x08, RO,                                 "");
      VAR(GPIORDBK, gpio3, "gpio3", 0x04, RO,                                 "");
      VAR(GPIORDBK, gpio2, "gpio2", 0x02, RO,                                 "");
      VAR(GPIORDBK, gpio1, "gpio1", 0x01, RO,                                 "");
    );

    REG(DPLLNUM_7_0, "DPLLNUM[7:0]", 0x42, RO,                                "DPLL Ratio [7:0]",);
    REG(DPLLNUM_15_8, "DPLLNUM[15:8]", 0x43, RO,                              "DPLL Ratio [15:8]",);
    REG(DPLLNUM_23_16, "DPLLNUM[23:16]", 0x44, RO,                            "DPLL Ratio [23:16]",);
    REG(DPLLNUM_31_24, "DPLLNUM[31:24]", 0x45, RO,                            "DPLL Ratio [31:24]",);

    uint32_t dpll_num() const { return (DPLLNUM_31_24() << 24) | (DPLLNUM_23_16()) << 16 | (DPLLNUM_15_8() << 8) | DPLLNUM_7_0(); }

    REG(SPDIFCHAN, "SPDIFCHAN", 0x46, RO,                                     "S/PDIF Channel Status (192 bit from here)",)
    bool CSPro() const { return (user_bits() & 0x01) != 0; }

    // 0x5E-0x63 Reserved

    REG(INSTATUS, "INSTATUS", 0x64, RO,                                       "Input Status",
      VAR(INSTATUS, dop_valid, "dop_valid", 0x08, RO,                         "");
      VAR(INSTATUS, spdif_select, "spdif_select", 0x04, RO,                   "");
      VAR(INSTATUS, i2s_select, "i2s_select", 0x02, RO,                       "");
      VAR(INSTATUS, dsd_select, "dsd_select", 0x01, RO,                       "");
    );

    // 0x65-0x6D Reserved

    REG(ADC1GPIO2_7_0, "ADC1GPIO2[7:0]", 0x6E, RO,                            "ADC1 GPIO2 FBQ Scale [7:0]",);
    REG(ADC1GPIO2_15_8, "ADC1GPIO2[15:8]", 0x6F, RO,                          "ADC1 GPIO2 FBQ Scale [15:8]",);
    REG(ADC1GPIO2_23_16, "ADC1GPIO2[23:16]", 0x70, RO,                        "ADC1 GPIO2 FBQ Scale [23:16]",);
    uint16_t adc1_gpio2_fbq_scale2() const { return ADC1GPIO2_23_16() << 16 | ADC1GPIO2_15_8() << 8 | ADC1GPIO2_7_0(); }

    REG(ADC2GPIO1_7_0, "ADC2GPIO1[7:0]", 0x71, RO,                            "ADC2 GPIO1 FBQ Scale [7:0]",);
    REG(ADC2GPIO1_15_8, "ADC2GPIO1[15:8]", 0x72, RO,                          "ADC2 GPIO1 FBQ Scale [15:8]",);
    REG(ADC2GPIO1_23_16, "ADC2GPIO1[23:16]", 0x73, RO,                        "ADC2 GPIO1 FBQ Scale [23:16]",);
    uint16_t adc2_gpio1_fbq_scale2() const { return ADC2GPIO1_23_16() << 16 | ADC2GPIO1_15_8() << 8 | ADC2GPIO1_7_0(); }
  );

  // Consumer == S/PDIF status word
  REGISTER_TABLE_SET_TABLE(ES9028PRO, Consumer, uint8_t, uint8_t,
    REG(SPDIFChannelStatus, "CHANNEL_STATUS", 0x46, RO,                     "S/PDIF Channel Status",
      VAR(SPDIFChannelStatus, SPDIF_CONFIG_TYPE, "SPDIF_CONFIG_TYPE", 0x01, RO, "Configuration register table type: 0 = consumer, 1 = professional");
      VAR(SPDIFChannelStatus, AUDIO_DATA, "AUDIO_DATA", 0x02, RO,           "0 = audio, 1 = data");
      VAR(SPDIFChannelStatus, COPYRIGHT, "COPYRIGHT", 0x04, RO,             "0 = copyright, 1 = non-copyright");
      VAR(SPDIFChannelStatus, PREEMPH, "PREEMPH", 0x08, RO,                 "0 = no preemphasis, 1 = preemphasis");
      VAR(SPDIFChannelStatus, NUM_CHANNELS, "NUM_CHANNELS", 0x20, RO,       "0 = 2 channels, 1 = 4 channels");
    );
    REG(CategoryCode, "CATEGORY_CODE", 0x47, RO,                            "Category Code",);
    REG(CSNumbers, "CHAN_SRC_NUMBERS", 0x48, RO,                            "Channel & Source Numbers",
      VAR(CSNumbers, CHANNEL_NUMBER, "CHANNEL_NUMBER", 0xF0, RO,            "");
      VAR(CSNumbers, SOURCE_NUMBER, "SOURCE_NUMBER", 0x0F, RO,              "");
    );
    REG(ClockSampleFreq, "CLK_SMPL_FREQ", 0x49, RO,                         "Clock Accuracy & Sample Frequency",
      VAR(ClockSampleFreq, CLOCK_ACCURACY, "CLOCK_ACCURACY", 0x30, RO,      "");
      VAR(ClockSampleFreq, SAMPLE_FREQUENCY, "SAMPLE_FREQUENCY", 0x0F, RO,  "");
    );
    REG(WordLengthFieldSize, "WORD_LEN_FIELD_SZ", 0x40, RO,                 "Word Length & Field Size",
      VAR(WordLengthFieldSize, WORD_LENGTH, "WORD_LENGTH", 0x0E, RO,        "");
      VAR(WordLengthFieldSize, WORD_FIELD_SIZE, "WORD_FIELD_SIZE", 0x01, RO,  "");
    );
  );

  // Professional == AES3 status word
  REGISTER_TABLE_SET_TABLE(ES9028PRO, Professional, uint8_t, uint8_t,
    REG(SPS0, "CHANNEL_STATUS", 0x46, RO,                                   "S/PDIF Channel Status",
      VAR(SPS0, SPDIF_CONFIG_TYPE, "SPDIF_CONFIG_TYPE", 0x01, RO,           "Configuration register table type: 0 = consumer, 1 = professional");
      VAR(SPS0, AUDIO_NON_AUDIO, "AUDIO_NON_AUDIO", 0x02, RO,               "0 = audio, 1 = non-audio");
      VAR(SPS0, EMPHASIS, "EMPHASIS", 0x1C, RO,                             "");
      VAR(SPS0, LOCK, "LOCK", 0x20, RO,                                     "");
      VAR(SPS0, SAMPLING_FREQUENCY, "SAMPLING_FREQUENCY", 0xC0, RO,         "");
    );

    REG(SPS4, "SPS4", 0x50, RO,                                             "S/PDIF Channel Status 4",
      VAR(SPS4, SAMPLE_FREQUENCY, "SAMPLE_FREQUENCY", 0x78, RO,             "");
    );
  );
);

#undef REG
#undef VAR

#endif // _ES9028PRO_RT_H_
