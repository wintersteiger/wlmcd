// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_H_
#define _ENOCEAN_H_

#include <string>
#include <vector>
#include <map>

#include <serialization.h>
#include <decoder.h>
#include <encoder.h>

namespace EnOcean
{
  struct EEP {
    uint8_t rorg;
    uint8_t func;
    uint8_t type;

    operator uint32_t() const { return rorg << 16 | func << 8 | type; }
    void to_json(json& j) const { j["rorg"] = rorg; j["func"] = func; j["type"] = type; }
    void from_json(const json& j) { rorg = j["rorg"]; func = j["func"]; type = j["type"]; }
  };

  using TXID = uint32_t;
  using MID = uint16_t;

  class Frame {
  public:
    Frame() {};
    Frame(std::vector<uint8_t>&& fbytes);
    Frame(uint8_t rorg, const std::vector<uint8_t> &payload, TXID source, uint8_t status);
    virtual ~Frame();

    uint8_t rorg() const;
    const uint8_t* data() const;
    TXID txid() const;
    uint8_t status() const;
    uint8_t hash() const;

    size_t num_repeater_hops() const;
    bool crc_ok() const;

    size_t size() const { return buffer.size(); }

    enum class IntegrityMechanism { Checksum = 0, CRC8 = 1 };
    IntegrityMechanism integrity_mechanism(bool skip_last = true) const;

    std::string describe() const;

    operator const std::vector<uint8_t>&() const { return buffer; }
    bool operator==(const Frame &other) const { return buffer == other.buffer; }
    bool operator!=(const Frame &other) const { return buffer != other.buffer; }

    inline void to_json(json &j) const { j = buffer; }
    inline void from_json(const json &j) { buffer = j.get<std::vector<uint8_t>>(); }

  protected:
    std::vector<uint8_t> buffer;
  };
}

inline void to_json(json &j, const EnOcean::EEP &v) { v.to_json(j); }
inline void from_json(const json &j, EnOcean::EEP &v) { v.from_json(j); }
inline void to_json(json &j, const EnOcean::Frame &v) { v.to_json(j); }
inline void from_json(const json &j, EnOcean::Frame &v) { v.from_json(j); }

#endif
