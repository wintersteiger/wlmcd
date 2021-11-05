// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _INA219_FIELDS_H_
#define _INA219_FIELDS_H_

#include <memory>

#include "ina219.h"
#include "ina219_rt.h"
#include "field.h"
#include "ui.h"

namespace INA219UIFields {

#define DOUBLE_FIELD_MP(W, N, S, D, U, G)                                                    \
  class N : public Field<double>                                                             \
  {                                                                                          \
  protected:                                                                                 \
    std::shared_ptr<D> device;                                                               \
                                                                                             \
  public:                                                                                    \
    N(int row, int col, std::shared_ptr<D> device) :                                         \
      Field<double>(W, row, col, S, "", U), device(device) {}                                \
    virtual double Get() { return G; }                                                       \
  };

  DOUBLE_FIELD_MP(UI::statusp, BusVoltage, "Bus voltage", INA219, "V", device->BusVoltage());
  DOUBLE_FIELD_MP(UI::statusp, ShuntVoltage, "Shunt voltage", INA219, "mV", device->ShuntVoltage() * 1000.0);
  DOUBLE_FIELD_MP(UI::statusp, Current, "Current", INA219, "A", device->Current());
  DOUBLE_FIELD_MP(UI::statusp, Power, "Power", INA219, "W", device->Power());

#define STRINGFIELD(N, D, U, G)                                                \
  class N : public Field<const char *> {                                       \
  protected:                                                                   \
    std::shared_ptr<INA219> ina219;                                            \
                                                                               \
  public:                                                                      \
    N(int row, int col, std::shared_ptr<INA219> ina219)                        \
        : Field<const char *>(UI::statusp, row, col, (D), "", U),              \
          ina219(ina219) {}                                                    \
    virtual ~N() {}                                                            \
    virtual const char *Get() { G; }                                           \
  };

static const std::vector<const char*> mode_map = {
      "Power-down",
      "Shunt trggd",
      "Bus trggd",
      "S&B trggd",
      "ADC off",
      "Shunt cont",
      "Bus cont",
      "S&B cont"
    };

STRINGFIELD(OpMode, "Mode", "", {
  uint8_t value = ina219->RT._vMode(ina219->RT.Configuration());
  return mode_map[value];
});

static const std::vector<const char*> brng_map = { "16", "32" };
STRINGFIELD(BusVoltageRange, "Bus vltg rng", "V", {
  uint8_t value = ina219->RT._vBRNG(ina219->RT.Configuration());
  return brng_map[value];
});

static const std::vector<const char*> pggain_map = { "1", "1/2", "1/4", "1/8" };
STRINGFIELD(PGAGain, "PGA gain", "", {
  uint8_t value = ina219->RT._vPG(ina219->RT.Configuration());
  return pggain_map[value];
});

static const std::vector<const char*> pgrange_map = { "+-40", "+-80", "+-160", "+-320" };
STRINGFIELD(PGARange, "PGA range", "mV", {
  uint8_t value = ina219->RT._vPG(ina219->RT.Configuration());
  return pgrange_map[value];
});

} // INA219UIFields


#endif // _INA219_FIELDS_H_
