// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _STATE_H_
#define _STATE_H_

#include <cinttypes>

class State
{
  public:
    State() {};
    virtual ~State() {}

    virtual void Update(const std::vector<uint8_t> &msg) = 0;
  };

#endif
