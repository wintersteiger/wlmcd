// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _BASIC_H_
#define _BASIC_H_

#include <cstdint>
#include <string>
#include <vector>

#include "decoder.h"
#include "encoder.h"
#include "state.h"
#include "ui.h"

class Basic {
public:
  class Message
  {
  public:
    Message() {}
    virtual ~Message() {}

    std::string str;
    virtual std::string ToString() const { return str; }
  };

  class State : public ::State
  {
  public:
    State();
    virtual ~State();
    virtual void Update(const std::vector<uint8_t> &msg);
  };

  class Decoder : public ::Decoder {
  protected:
    char tmp[2048];
    Message message;
    std::string tmp_str;

  public:
    Basic::State state;

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