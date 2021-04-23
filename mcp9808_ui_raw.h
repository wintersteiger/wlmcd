// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _MCP9808_UI_RAW_H_
#define _MCP9808_UI_RAW_H_

#include <cstdint>
#include <vector>

#include "ui.h"
#include "field.h"

class MCP9808;

class MCP9808UIRaw : public UI {
public:
  MCP9808UIRaw(MCP9808 &MCP9808);
  virtual ~MCP9808UIRaw();

  virtual std::string Name() const { return "MCP9808 (raw)"; }
};

#endif // _MCP9808_UI_RAW_H_
