// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <ctime>
#include <memory>

#include "device.h"

#include "radbot_ui.h"

template <typename T>
class RadbotField : public Field<T> {
protected:
  const Radbot::State &state;

public:
  RadbotField(int row, const std::string &key, const std::string &units, const Radbot::State &state) :
    Field<T>(UI::statusp, row, 1, key, "", units), state(state) { FieldBase::key_width = 19; }
  virtual ~RadbotField() {}
  virtual T Get() = 0;
};

#define RBF(N,K,T,U,G) \
  class N##RF : public RadbotField<T> { \
  public: \
    N##RF(int row, const Radbot::State &state) : \
      RadbotField<T>(row, K, U, state) {} \
    virtual T Get() { return G; } \
  }; \
  fields.push_back(new N##RF(row++, state));

#define RBW(N,K,G) \
  class N##RBW : public WarningField { \
  protected: \
    const Radbot::State &state; \
  public: \
    N##RBW(int r, int c, const Radbot::State &state) : \
      WarningField(UI::statusp, r, c, K), state(state) {} \
    virtual size_t Width() { return key.size(); } \
    virtual bool Get() { return G; } \
  };

RBW(Fault, "Fault", state.fault);
RBW(Battery, "Battery", state.low_battery);
RBW(Tamper, "Tamper", state.tamper_protect);
RBW(Frost, "Frost", state.frost_risk);

RadbotUI::RadbotUI(const Radbot::State &state, std::vector<std::shared_ptr<DeviceBase>> devices) :
  UI(),
  last_update(time(NULL)),
  state(state)
{
  for (auto d: devices)
    this->devices.insert(d.get());

  size_t row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  fields.push_back(new Empty(row++, col));

  fields.push_back(new FaultRBW(row, col, state));
  fields.push_back(new BatteryRBW(row, col + 7, state));
  fields.push_back(new TamperRBW(row, col + 16, state));
  fields.push_back(new FrostRBW(row++, col + 24, state));

  fields.push_back(new Empty(row++, col));

  static std::vector<const char*> occupancy_map = { "unknown", "none", "possible", "likely" };

#if 0
  // Only old versions, not the Radbot?
  RBF(Valve, "Valve status (?)", opt_double, "%", mk_pair(state.valve_, true));
  RBF(Occupancy, "Occupany (?)", const char*, "", occupancy_map[state.occupancy_], true);
#endif

  RBF(RoomTemp, "Room temperature", opt_double, "C",  std::make_pair(state.room_temp.first / 16.0, state.room_temp.second));
  RBF(TargetTemp, "Target temperature", opt_uint64_t, "C", state.target_temp);

  fields.push_back(new Empty(row++, col));

  RBF(RelHum, "Rel humidity", opt_uint64_t, "%", state.rel_humidity);
  RBF(ValveInternal, "Valve status", opt_uint64_t, "%", state.valve_status);
  RBF(AmbientLight, "Ambient light", opt_double, "%", std::make_pair(100.0 * (state.ambient_light.first / 256.0), state.ambient_light.second));

  fields.push_back(new Empty(row++, col));

  RBF(OccupancyStats, "Occupancy", const char*, "", state.occupancy.second ? occupancy_map[state.occupancy.first] : "?");
  RBF(Vacancy, "Vacancy", opt_uint64_t, "h", state.vacancy);

  fields.push_back(new Empty(row++, col));
  RBF(SupplyVoltage, "Supply voltage", opt_double, "V", std::make_pair(state.supply_voltage.first / 100.0, state.supply_voltage.second));
  RBF(SetbackTemp, "Setback temperature", opt_uint64_t, "C", state.setback_temp);
  RBF(CumValve, "Cum valve mvt", opt_uint64_t, "%", state.cum_valve);
  RBF(SetbackLockout, "Setback lockout", opt_uint64_t, "C", state.setback_lockout);
  RBF(ResetCounter, "Reset counter", opt_uint64_t, "", state.reset_counter);
  RBF(ErrorReport, "Error report", opt_uint64_t, "", state.error_report);
}

RadbotUI::~RadbotUI() {}
