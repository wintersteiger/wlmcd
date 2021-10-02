// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _MCP3423_UI_H_
#define _MCP3423_UI_H_

#include <memory>

#include "ui.h"

class MCP3423;

std::shared_ptr<UI> make_mcp3423_raw_ui(std::shared_ptr<MCP3423> &device);

class MCP3423UI : public UI {
public:
  MCP3423UI(std::shared_ptr<MCP3423> mcp3423);
  virtual ~MCP3423UI() {}

  virtual std::string Name() const { return "MCP3423"; }
};

#endif // _MCP3423_UI_H_
