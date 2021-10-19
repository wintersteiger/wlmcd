// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_H_
#define _ENOCEAN_H_

#include <string>
#include <vector>

#include "decoder.h"
#include "encoder.h"
#include "state.h"
#include "field_types.h"

class EnOcean {
public:
  class State : public ::State {
  public:
    State();
    virtual ~State() {}

    void virtual Update(const std::vector<uint8_t> &msg);
  };

  class Decoder : public ::Decoder {
  protected:
    char tmp[2048];
    std::string tmp_str;

  public:
    EnOcean::State state;

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
