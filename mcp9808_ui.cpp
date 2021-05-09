// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "raw_ui.h"
#include "mcp9808.h"
#include "mcp9808_rt.h"

#include "mcp9808_ui.h"

std::shared_ptr<UI> make_mcp9808_raw_ui(std::shared_ptr<MCP9808> &device)
{
  return make_raw_ui<MCP9808, uint8_t, uint16_t>(device, device->RT);
}
