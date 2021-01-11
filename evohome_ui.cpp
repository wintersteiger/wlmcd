// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <ctime>

#include <evohome_ui.h>

template <typename T>
class EvohomeField : public Field<T> {
protected:
  const Evohome::State &state;

public:
  EvohomeField(int row, const std::string &key, const std::string &units, const Evohome::State &state) :
    Field<T>(UI::statusp, row, 1, key, "", units), state(state) { FieldBase::key_width = 19; }
  virtual ~EvohomeField() {}
  virtual T Get() = 0;
};

#define EHF(N,K,T,U,G) \
  class N##EHFld : public EvohomeField<T> { \
  protected: \
    size_t index; \
  public: \
    N##EHFld(int row, const Evohome::State &state, size_t index) : \
      EvohomeField<T>(row, K, U, state), index(index) {} \
    virtual T Get() { G; } \
  };

#define EHW(N,K,G) \
  class N##EHW : public WarningField { \
  protected: \
    const Evohome::State &state; \
  public: \
    N##EHW(int r, int c, const Evohome::State &state) : \
      WarningField(UI::statusp, r, c, K), state(state) {} \
    virtual size_t Width() { return key.size(); } \
    virtual bool Get() { return G; } \
  };

EHF(Setpoint, "Setpoint", double, "C", { return state.zones.at(index).setpoint; });
EHF(Demand, "Demand", double, "%", { return state.zones.at(index).demand; });
EHF(MaxFlow, "Max flow temp", uint8_t, "C", { return state.zones.at(index).max_flow_temp; });
EHF(PumpTime, "Pump run time", uint8_t, "min", { return state.zones.at(index).pump_run_time; });
EHF(Acttr, "Actuator run time", uint8_t, "sec", { return state.zones.at(index).actuator_run_time; });
EHF(MinFlow, "Min flow temp", uint8_t, "C", { return state.zones.at(index).min_flow_temp; });

EHF(Datetime, "Last time report", std::string, "", { return state.devices.at(index).datetime; });
EHF(Status, "Status", double, "%", { return state.devices.at(index).status; });
EHF(BoilerModulation, "Boiler mod lvl", double, "%", { return state.devices.at(index).boiler_modulation_level; });
EHF(FlameStatus, "Flame status", uint8_t, "", { return state.devices.at(index).flame_status; });
EHF(DomainID, "Domain ID", uint8_t, "", { return state.devices.at(index).domain_id; });
EHF(CycleRate, "Cycle rate", double, "c/h", { return state.devices.at(index).cycle_rate; });
EHF(MinOnTime, "Min on time", double, "min", { return state.devices.at(index).minimum_on_time; });
EHF(MinOffTime, "Min off time", double, "min", { return state.devices.at(index).minimum_off_time; });
EHF(PropBW, "Prop B/W", double, "", { return state.devices.at(index).proportional_band_width; });
EHF(DevNo, "Device number", uint8_t, "", { return state.devices.at(index).device_number; });
static std::vector<const char*> failsafe_map = { "off", "20/80", "unknown" };
EHF(FailsafeMode, "Failsafe mode", const char*, "", { return failsafe_map[state.devices.at(index).failsafe_mode]; });
EHF(State, "State", double, "", { return state.devices.at(index).state; });

EvohomeUI::EvohomeUI(const Evohome::State &state) :
  UI(),
  last_update(time(NULL)),
  state(state)
{
  Build();
}

EvohomeUI::~EvohomeUI() {}

void EvohomeUI::Build() {
  size_t row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  fields.push_back(new Empty(row++, col));

  for (auto kv: state.zones)
  {
    fields.push_back(new Label(UI::statusp, row++, col, std::string("Zone ") + std::to_string(kv.first)));

    fields.push_back(new SetpointEHFld(row++, state, kv.first));
    fields.push_back(new DemandEHFld(row++, state, kv.first));
    fields.push_back(new MaxFlowEHFld(row++, state, kv.first));
    fields.push_back(new MinFlowEHFld(row++, state, kv.first));
    fields.push_back(new PumpTimeEHFld(row++, state, kv.first));
    fields.push_back(new ActtrEHFld(row++, state, kv.first));
    fields.push_back(new Empty(row++, col));
  }

  for (auto kv: state.devices)
  {
    static char tmp[16];
    snprintf(tmp, sizeof(tmp), "Device %06x", kv.first);
    fields.push_back(new Label(UI::statusp, row++, col, tmp));

    fields.push_back(new DatetimeEHFld(row++, state, kv.first));
    fields.push_back(new DevNoEHFld(row++, state, kv.first));
    fields.push_back(new DomainIDEHFld(row++, state, kv.first));

    fields.push_back(new StatusEHFld(row++, state, kv.first));
    fields.push_back(new BoilerModulationEHFld(row++, state, kv.first));
    fields.push_back(new FlameStatusEHFld(row++, state, kv.first));
    fields.push_back(new CycleRateEHFld(row++, state, kv.first));
    fields.push_back(new MinOnTimeEHFld(row++, state, kv.first));
    fields.push_back(new MinOffTimeEHFld(row++, state, kv.first));
    fields.push_back(new PropBWEHFld(row++, state, kv.first));
    fields.push_back(new FailsafeModeEHFld(row++, state, kv.first));
    fields.push_back(new StateEHFld(row++, state, kv.first));

    fields.push_back(new Empty(row++, col));
  }
}

void EvohomeUI::Update(bool full)
{
  if (state.need_ui_rebuild) {
    fields.clear();
    Build();
    Layout();
    state.need_ui_rebuild = false;
    wclear(UI::statusp);
    UI::Update(true);
  }
  else
    UI::Update(full);
}
