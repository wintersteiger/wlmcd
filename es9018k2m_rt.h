// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ES9018K2M_RT_H_
#define _ES9018K2M_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE_SET(ES9018K2M, ES9018K2M, RegisterTableSet,

REGISTER_TABLE_SET_TABLE_W(ES9018K2M, Main, uint8_t, uint8_t,
  REG(SystemSettings, "System Settings", 0x00, RW,                        "System Settings",
    VAR(SystemSettings, OSC_DRV, "OSC_DRV", 0xF0, RW,                     "");
    VAR(SystemSettings, SOFT_RESET, "SOFT_RESET", 0x01, RW,               "");
  );
  REG(InputConfiguration, "Input Configuration", 0x01, RW,                "Input Configuration",
    VAR(InputConfiguration, I2S_LENGTH, "I2S_LENGTH", 0xC0, RW,           "");
    VAR(InputConfiguration, I2S_MODE, "I2S_MODE", 0x30, RW,               "");
    VAR(InputConfiguration, AUTO_INPUT_SELECT, "AUTO_INPUT_SELECT", 0x0C, RW, "");
    VAR(InputConfiguration, INPUT_SELECT, "INPUT_SELECT", 0x03, RW,       "");
  );
  REG(SVC1, "SVC1", 0x04, RW,                                             "Soft Volume Control 1 (Automute Time)",
    VAR(SVC1, AUTOMUTE_TIME, "AUTOMUTE_TIME", 0xFF, RW,                   "");
  );
  REG(SVC2, "SVC2", 0x05, RW,                                             "Soft Volume Control 2 (Automute Level)",
    VAR(SVC2, AUTOMUTE_LOOPBACK, "AUTOMUTE_LOOPBACK", 0x80, RW,           "");
    VAR(SVC2, AUTOMUTE_LEVEL, "AUTOMUTE_LEVEL", 0x7F, RW,                 "");
  );
  REG(SVC3, "SVC3", 0x06, RW,                                             "Soft Volume Control 3 & De-emphasis",
    VAR(SVC3, SPDIF_AUTO_DEEMPH, "SPDIF_AUTO_DEEMPH", 0x80, RW,           "");
    VAR(SVC3, DEEMPH_BYPASS, "DEEMPH_BYPASS", 0x40, RW,                   "");
    VAR(SVC3, DEEMPH_SEL, "DEEMPH_SEL", 0x30, RW,                         "");
    VAR(SVC3, VOL_RATE, "VOL_RATE", 0x07, RW,                             "");
  );
  REG(GeneralSettings, "GENERAL_SETTINGS", 0x07, RW,                      "General Settings",
    VAR(GeneralSettings, FILTER_SHAPE, "FILTER_SHAPE", 0x60, RW,          "");
    VAR(GeneralSettings, IIR_BW, "IIR_BW", 0x0C, RW,                      "");
    VAR(GeneralSettings, MUTE, "MUTE", 0x03, RW,                          "");
  );
  REG(GPIOConfiguration, "GPIO_CONFIG", 0x08, RW,                         "GPIO Configuration",
    VAR(GPIOConfiguration, GPIO2_CFG, "GPIO2_CFG", 0xF0, RW,              "");
    VAR(GPIOConfiguration, GPIO1_CFG, "GPIO1_CFG", 0x0F, RW,              "");
  );

  REG(MasterModeControl, "MASTER_MODE_CONTROL", 0x0A, RW,                 "Master Mode Control",
    VAR(MasterModeControl, MASTER_CLOCK_ENABLE, "MASTER_CLOCK_ENABLE", 0x80, RW,  "");
    VAR(MasterModeControl, CLOCK_DIVIDER_SELECT, "CLOCK_DIVIDER_SELECT", 0x60, RW,  "");
    VAR(MasterModeControl, SYNC_MODE, "SYNC_MODE", 0x10, RW,              "");
    VAR(MasterModeControl, STOP_DIV, "STOP_DIV", 0x0F, RW,                "");
  );
  REG(ChannelMapping, "CHANNEL_MAPPING", 0x0B, RW,                        "Channel Mapping",
    VAR(ChannelMapping, SPDIF_SEL, "SPDIF_SEL", 0x70, RW,                 "");
    VAR(ChannelMapping, CH2_ANALOG_SWAP, "CH2_ANALOG_SWAP", 0x08, RW,     "");
    VAR(ChannelMapping, CH1_ANALOG_SWAP, "CH1_ANALOG_SWAP", 0x04, RW,     "");
    VAR(ChannelMapping, CH2_SEL, "CH2_SEL", 0x02, RW,                     "");
    VAR(ChannelMapping, CH1_SEL, "CH1_SEL", 0x01, RW,                     "");
  );
  REG(DPLLASRCSettings, "DPLL_ASRC_SETTINGS", 0x0C, RW,                   "DPLL/ASRC Settings",
    VAR(DPLLASRCSettings, DPLL_BW_I2S, "DPLL_BW_I2S", 0xF0, RW,           "");
    VAR(DPLLASRCSettings, DPLL_BW_DSD, "DPLL_BW_DSD", 0x0F, RW,           "");
  );
  REG(THDCompensation, "THD_COMPENSATION", 0x0D, RW,                      "THD Compensation",
    VAR(THDCompensation, BYPASS_THD, "BYPASS_THD", 0x40, RW,              "");
  );
  REG(SoftStartSettings, "SOFT_START_SETTINGS", 0x0E, RW,                 "Soft Start Settings",
    VAR(SoftStartSettings, SOFT_START, "SOFT_START", 0x80, RW,            "");
    VAR(SoftStartSettings, SOFT_START_ON_LOCK, "SOFT_START_ON_LOCK", 0x40, RW,  "");
    VAR(SoftStartSettings, MUTE_ON_LOCK, "MUTE_ON_LOCK", 0x20, RW,        "");
    VAR(SoftStartSettings, SOFT_START_TIME, "SOFT_START_TIME", 0x1F, RW,  "");
  );
  REG(Volume1, "VOLUME1", 0x0F, RW,                                       "Volume 1",
    VAR(Volume1, VOLUME1, "VOLUME1", 0xFF, RW,                            "");
  );
  REG(Volume2, "VOLUME2", 0x10, RW,                                       "Volume 2",
    VAR(Volume2, VOLUME2, "VOLUME2", 0xFF, RW,                            "");
  );

  REG(MasterTrim_7_0, "MASTER_TRIM[7:0]", 0x11, RW,                       "Master Trim 0 (LSB)",);
  REG(MasterTrim_15_8, "MASTER_TRIM[15:8]", 0x12, RW,                     "Master Trim 1",);
  REG(MasterTrim_23_16, "MASTER_TRIM[23:16]", 0x13, RW,                   "Master Trim 2",);
  REG(MasterTrim_31_24, "MASTER_TRIM[31:24]", 0x14, RW,                   "Master Trim 3 (MSB)",);
  int32_t MasterTrim() const { return (MasterTrim_31_24() << 24) | (MasterTrim_23_16()) << 16 | (MasterTrim_15_8() << 8) | MasterTrim_7_0(); }

  REG(GPIOInputSelection, "GPIO_INPUT_SELECTION", 0x15, RW,               "GPIO Input Selection & OSF Bypass",
    VAR(GPIOInputSelection, GPIO_INPUT_SEL2, "GPIO_INPUT_SEL2", 0xC0, RW, "");
    VAR(GPIOInputSelection, GPIO_INPUT_SEL1, "GPIO_INPUT_SEL1", 0x30, RW, "");
    VAR(GPIOInputSelection, BYPASS_IIR, "BYPASS_IIR", 0x04, RW,           "");
    VAR(GPIOInputSelection, BYPASS_OSF, "BYPASS_OSF", 0x01, RW,           "");
  );

  REG(HarmonicCompensation2nd_7_0, "HARMONIC_COMP_2ND[7:0]", 0x16, RW,    "2nd Harmonic Compensation Coefficients 0 (LSB)",);
  REG(HarmonicCompensation2nd_15_8, "HARMONIC_COMP_2ND[15:8]", 0x17, RW,  "2nd Harmonic Compensation Coefficients 1 (MSB)",);
  uint16_t HarmonicCompensation2nd() const { return (HarmonicCompensation2nd_15_8() << 8) | HarmonicCompensation2nd_7_0(); }

  REG(HarmonicCompensation3rd_7_0, "HARMONIC_COMP_2ND[7:0]", 0x18, RW,    "3rd Harmonic Compensation Coefficients 0 (LSB)",);
  REG(HarmonicCompensation3rd_15_8, "HARMONIC_COMP_2ND[15:8]", 0x19, RW,  "3rd Harmonic Compensation Coefficients 1 (MSB)",);
  uint16_t HarmonicCompensation3rd() const { return (HarmonicCompensation3rd_15_8() << 8) | HarmonicCompensation3rd_7_0(); }

  REG(ProgFilterAddress, "PROG_FILTER_ADDRESS", 0x1A, RW,                 "Programmable Filter Address",
    VAR(ProgFilterAddress, PROG_COEFF_STAGE, "PROG_COEFF_STAGE", 0x80, RW,  "");
    VAR(ProgFilterAddress, PROG_COEFF_ADDR, "PROG_COEFF_ADDR", 0x7F, RW,  "");
  );
  REG(ProgFilterCoef_7_0, "PROG_FILTER_COEFFICIENT[7:0]", 0x1B, RW,       "Programmable Filter Coefficient 0 (LSB)",);
  REG(ProgFilterCoef_15_8, "PROG_FILTER_COEFFICIENT[15:8]", 0x1C, RW,     "Programmable Filter Coefficient 1",);
  REG(ProgFilterCoef_23_16, "PROG_FILTER_COEFFICIENT[23:16]", 0x1D, RW,   "Programmable Filter Coefficient 2 (MSB)",);
  uint32_t ProgFilterCoef() const { return (ProgFilterCoef_23_16() << 16) | (ProgFilterCoef_15_8() << 8) | ProgFilterCoef_7_0(); }
  REG(ProgFilterControl, "PROG_FILTER_CONTROL", 0x1E, RW,                 "Programmable Filter Control",
    VAR(ProgFilterControl, EVEN_STAGE2_COEFF, "EVEN_STAGE2_COEFF", 0x04, RW,  "");
    VAR(ProgFilterControl, PROG_COEFF_WE, "PROG_COEFF_WE", 0x02, RW,      "");
    VAR(ProgFilterControl, PROG_COEFF_EN, "PROG_COEFF_EN", 0x01, RW,      "");
  );


  REG(ChipStatus, "CHIP STATUS", 0x40, RO,                                "Chip Status",
    VAR(ChipStatus, REVISION, "REVISION", 0x20, RO,                       "");
    VAR(ChipStatus, CHIP_ID, "CHIP_ID", 0x1C, RO,                         "");
    VAR(ChipStatus, AUTOMUTE_STATUS, "AUTOMUTE_STATUS", 0x2, RO,          "");
    VAR(ChipStatus, LOCK_STATUS, "LOCK_STATUS", 0x01, RO,                 "");
  );
  REG(GPIOStatus, "GPIO STATUS", 0x41, RO,                                "GPIO Status",
    VAR(GPIOStatus, GPIO_I_1_0, "GPIO_I[1:0]", 0x03, RO,                  "");
  );
  REG(DPLLRatio_7_0, "DPLL_RATIO[7:0]", 0x42, RO,                         "DPLL Ratio 0 (LSB)",);
  REG(DPLLRatio_15_8, "DPLL_RATIO[15:8]", 0x43, RO,                       "DPLL Ratio 1",);
  REG(DPLLRatio_23_16, "DPLL_RATIO[23:16]", 0x44, RO,                     "DPLL Ratio 2",);
  REG(DPLLRatio_31_24, "DPLL_RATIO[31:24]", 0x45, RO,                     "DPLL Ratio 3 (MSB)",);

  uint32_t DPLLRatio() const { return (DPLLRatio_31_24() << 24) | (DPLLRatio_23_16()) << 16 | (DPLLRatio_15_8() << 8) | DPLLRatio_7_0(); }

  REG(SPDIFChannelStatus, "CHANNEL_STATUS", 0x46, RO,                     "S/PDIF Channel Status",
    VAR(SPDIFChannelStatus, SPDIF_CONFIG_TYPE, "SPDIF_CONFIG_TYPE", 0x01, RO, "Configuration register table type: 0 = consumer, 1 = professional");
  )
);

REGISTER_TABLE_SET_TABLE(ES9018K2M, Consumer, uint8_t, uint8_t,
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

REGISTER_TABLE_SET_TABLE(ES9018K2M, Professional, uint8_t, uint8_t,
  REG(Status0, "CHANNEL_STATUS", 0x46, RO,                                "S/PDIF Channel Status",
    VAR(Status0, SPDIF_CONFIG_TYPE, "SPDIF_CONFIG_TYPE", 0x01, RO,        "Configuration register table type: 0 = consumer, 1 = professional");
    VAR(Status0, AUDIO_NON_AUDIO, "AUDIO_NON_AUDIO", 0x02, RO,            "0 = audio, 1 = non-audio");
    VAR(Status0, EMPHASIS, "EMPHASIS", 0x1C, RO,                          "");
    VAR(Status0, LOCK, "LOCK", 0x20, RO,                                  "");
    VAR(Status0, SAMPLING_FREQUENCY, "SAMPLING_FREQUENCY", 0xC0, RO,      "");
  );
);

);

#endif // _ES9018K2M_RT_H_
