// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _MCP9808_UI_H_
#define _MCP9808_UI_H_

#include <memory>

#include "ui.h"

class MCP9808;

template <typename D, typename AT, typename VT>
class RegisterTable;

std::shared_ptr<UI> make_mcp9808_raw_ui(std::shared_ptr<MCP9808> &device);

#endif // _MCP9808_UI_H_
