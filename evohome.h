// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _EVOHOME_H_
#define _EVOHOME_H_

#include <map>
#include <vector>

#include "decoder.h"
#include "encoder.h"
#include "state.h"
#include "ui.h"

class Evohome {
public:
  class Message
  {
  public:
    Message() {}
    virtual ~Message() {}

    uint8_t header;
    std::vector<size_t> device_ids;
    uint16_t command;
    uint8_t payload_length;
    std::vector<uint8_t> payload;
    size_t num_unparsed;

    std::string str;
    virtual std::string ToString() const { return str; }
  };

  class State : public ::State
  {
  public:
    State();
    virtual ~State();

    typedef struct {
      uint8_t max_flow_temp;
      uint8_t pump_run_time;
      uint8_t actuator_run_time;
      uint8_t min_flow_temp;
      double demand;
      double setpoint;
    } Zone;

    std::map<size_t, Zone> zones;

    typedef enum { FSM_OFF, FSM_20_80, FSM_UNKNOWN } failsafe_mode_t;

    typedef struct {
      std::string datetime;
      double status;
      double boiler_modulation_level;
      uint8_t flame_status;
      uint8_t domain_id;
      double cycle_rate;
      double minimum_on_time;
      double minimum_off_time;
      double proportional_band_width;
      uint8_t device_number;
      failsafe_mode_t failsafe_mode;
      double state;
    } Device;

    std::map<size_t, Device> devices;

    mutable bool need_ui_rebuild;

    virtual void Update(const std::vector<uint8_t> &msg);
  };

  class Decoder : public ::Decoder {
  protected:
    char tmp[2048];
    Message message;

  public:
    Evohome::State state;

    Decoder();
    virtual ~Decoder();

    virtual const std::string& Decode(std::vector<uint8_t> &bytes);
  };

  class Encoder : public ::Encoder {
  public:
    Encoder();
    virtual ~Encoder();

    virtual std::vector<uint8_t> Encode(const std::vector<uint8_t> &data);
  };
};

#endif
