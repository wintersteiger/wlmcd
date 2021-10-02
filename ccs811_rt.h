// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CCS811_RT_H_
#define _CCS811_RT_H_

#include "register_table.h"

#define REG(N, P, A, SZ, RW, H, V) VREGDECL(uint8_t, SZ, N, P, A, RW, H, V)
#define VAR(R, N, NN, M, RW, D) VARDECL(uint8_t, std::vector<uint8_t>, R, N, NN, M, RW, D)

static const std::vector<uint8_t> ECO2_MASK =     { 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const std::vector<uint8_t> TVOC_MASK =     { 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 };
static const std::vector<uint8_t> STATUS_MASK =   { 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00 };
static const std::vector<uint8_t> ERROR_ID_MASK = { 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00 };
static const std::vector<uint8_t> RAW_DATA_MASK = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF };
static const std::vector<uint8_t> CURRENT_MASK =  { 0xFC, 0x00 };
static const std::vector<uint8_t> RAW_ADC_MASK =  { 0x03, 0xFF };

REGISTER_TABLE_SPARSE_VAR(CCS811, RegisterTable, uint8_t,
  REG(STATUS, "STATUS", 0x00, 1, RO,                    "Status",
    VAR(STATUS, FW_MODE, "FW_MODE", { 0x80 }, RO,       "");
    VAR(STATUS, APP_VALID, "APP_VALID", { 0x10 }, RO,   "");
    VAR(STATUS, DATA_READY, "DATA_READY", { 0x08 }, RO, "");
    VAR(STATUS, ERROR, "ERROR", { 0x01 }, RO,           "");
  );
  REG(MEAS_MODE, "MEAS_MODE", 0x01, 1, RW,              "Measurement mode",
    VAR(MEAS_MODE, DRIVE_MODE, "DRIVE_MODE", { 0x70 }, RO,    "");
    VAR(MEAS_MODE, INT_DATARDY, "INT_DATARDY", { 0x08 }, RO,  "");
    VAR(MEAS_MODE, INT_THRESH, "INT_THRESH", { 0x04 }, RO,    "");
  );
  REG(ALG_RESULT_DATA, "ALG_RESULT_DATA", 0x02, 8, RO,                    "Algorithm result",
    VAR(ALG_RESULT_DATA, ECO2, "eCO2", ECO2_MASK, RO,                     "");
    VAR(ALG_RESULT_DATA, TVOC, "TVOC", TVOC_MASK, RO,                     "");
    VAR(ALG_RESULT_DATA, STATUS_, "STATUS_", STATUS_MASK, RO,             "");
    VAR(ALG_RESULT_DATA, ERROR_ID_, "ERROR_IDSTATUS", ERROR_ID_MASK, RO,  "");
    VAR(ALG_RESULT_DATA, RAW_DATA_, "RAW_DATA", RAW_DATA_MASK, RO,        "");
  );
  REG(RAW_DATA, "RAW_DATA", 0x03, 2, RO,                                  "Raw ADC data",
    VAR(RAW_DATA, CURRENT, "CURRENT", CURRENT_MASK, RO,                   "");
    VAR(RAW_DATA, RAW_ADC, "RAW_ADC", RAW_ADC_MASK, RO,                   "");
  );
  REG(ENV_DATA, "ENV_DATA", 0x05, 4, WO,                "Environment data", );
  REG(NTC, "NTC", 0x06, 4, RO,                          "Reference and NTC voltages", );
  REG(THRESHOLDS, "THRESHOLDS", 0x10, 5, WO,            "Thresholds", );
  REG(BASELINE, "BASELINE", 0x11, 2, RO,                "Baseline values", );
  REG(HW_ID, "HW_ID", 0x20, 1, RO,                      "Hardware ID", );
  REG(HW_VERSION, "HW Version", 0x21, 1, RO,            "Hardware version", );
  REG(FW_BOOT_VERSION, "FW_Boot_Version", 0x23, 2, RO,  "Firmware boot version", );
  REG(FW_APP_VERSION, "FW_App_Version", 0x24, 2, RO,    "Firmware application version", );
  REG(ERROR_ID, "ERROR_ID", 0xE0, 1, RO,                "Error ID", );
  REG(SW_RESET, "SW_RESET", 0xFF, 4, WO,                "Soft reset", );
);

#undef REG
#undef VAR

#endif // _CCS811_RT_H_
