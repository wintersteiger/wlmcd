// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_H_
#define _ENOCEAN_H_

#include <string>
#include <vector>
#include <map>

#include <decoder.h>
#include <encoder.h>
#include "state.h"
#include "field_types.h"

namespace EnOcean
{
  using EEP = uint32_t;
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

  protected:
    std::vector<uint8_t> buffer;
  };

  class Decoder : public ::Decoder {
  public:
    Decoder();
    virtual ~Decoder();

    virtual const std::string& Decode(std::vector<uint8_t> &bytes);

    virtual std::vector<Frame> get_frames(const std::vector<uint8_t> &bytes);
  };

  class Encoder : public ::Encoder {
  public:
    Encoder();
    virtual ~Encoder();

    virtual std::vector<uint8_t> Encode(const std::vector<uint8_t> &data);

    virtual std::vector<uint8_t> Encode(const Frame& frame);
  };
}

#endif
