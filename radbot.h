// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RADBOT_H_
#define _RADBOT_H_

#include <string>
#include <vector>

#include "decoder.h"
#include "encoder.h"
#include "state.h"
#include "field_types.h"

class Radbot {
public:
  class State : public ::State {
  public:
    typedef enum { OC_UNREPORTED=0, OC_NONE=1, OC_POSSIBLE=2, OC_LIKELY=3 } Occupancy;

    State();
    virtual ~State() {}

    bool fault;
    bool low_battery;
    bool tamper_protect;
    bool have_stats;
    bool frost_risk;

    float valve_;
    Occupancy occupancy_;

    // https://github.com/opentrv/OTWiki/wiki/Data-Formats
    opt_uint64_t supply_voltage; // B|cV  The battery voltage, in centiVolts.
    opt_uint64_t room_temp; // "T|C16"  The room temperature in 16ths of a °C.
    opt_uint64_t ambient_light; // "L" 	The current light level 0-255 (uncalibrated).
    opt_uint64_t vacancy; // "vac|h" 	Vacancy time in hours.
    opt_uint64_t valve_status; // "v|%" 	The valve open status, as a percentage of fully open.
    opt_uint64_t target_temp; // "tT|C"  	The target room temperature in C.
    opt_uint64_t setback_temp; // "tS|C" 	The energy-saving temperature setback in C.
    opt_uint64_t cum_valve; // "vC|%" 	Valve movement Cumulative (%). [Get cumulative valve movement %; rolls at 8192 in range [0,8191], ie non-negative.]
    opt_uint64_t rel_humidity; // "H|%" 	The relative Humidity (%).
    opt_uint64_t occupancy; // "O"  A determination of room occupancy: 0 unknown, 1 vacant, 2 possibly occupied, 3 definitely occupied
    opt_uint64_t setback_lockout; // "gE"  Number of days until main energy savings are enabled.
    opt_uint64_t reset_counter; // "R" // Reset counter. Low priority.
    opt_uint64_t error_report; // "err"  Error report, see https://github.com/opentrv/OTRadioLink/blob/f7fc1fdf4728a3608cc15cdebf8ec83d5254c87b/content/OTRadioLink/utility/OTV0P2BASE_ErrorReport.h#L72

    void virtual Update(const std::vector<uint8_t> &msg);
  };

  class Decoder : public ::Decoder {
  protected:
    uint8_t id[8], key[16];
    uint8_t iv[12];
    char tmp[2048];
    std::string tmp_str;

  public:
    Radbot::State state;

    Decoder(const std::string &id, const std::string &key);
    virtual ~Decoder();

    virtual const std::string& Decode(std::vector<uint8_t> &bytes);
  };

  class Encoder : public ::Encoder {
  protected:
    uint8_t id[8], key[16];
  public:
    Encoder(const std::string &id, const std::string &key);
    virtual ~Encoder();

    virtual std::vector<uint8_t> Encode(const std::vector<uint8_t> &data);
  };
};

#endif
