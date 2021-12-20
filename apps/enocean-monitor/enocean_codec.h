// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_CODEC_H_
#define _ENOCEAN_CODEC_H_

#include <cstdint>
#include <memory>

#include <decoder.h>
#include <encoder.h>

#include "enocean_frame.h"

namespace EnOcean
{
  class Decoder : public ::Decoder {
  public:
    Decoder();
    virtual ~Decoder();

    virtual const std::string& Decode(std::vector<uint8_t> &bytes);

    virtual std::vector<std::shared_ptr<Frame>> get_frames(const std::vector<uint8_t> &bytes) noexcept;
  };

  class Encoder : public ::Encoder {
  public:
    Encoder();
    virtual ~Encoder();

    virtual std::vector<uint8_t> Encode(const std::vector<uint8_t> &data);

    virtual std::vector<uint8_t> Encode(const Frame& frame);
  };
}

#endif // _ENOCEAN_CODEC_H_
