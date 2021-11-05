// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include "raw_ui.h"
#include "mcp3423.h"
#include "mcp3423_rt.h"

#include "mcp3423_ui.h"

std::shared_ptr<UI> make_mcp3423_raw_ui(std::shared_ptr<MCP3423> &device)
{
  return make_raw_ui<MCP3423, uint8_t, uint8_t>(device, device->RT);
}

#define EMPTY() fields.push_back(new Empty(row++, col));

MCP3423UI::MCP3423UI(std::shared_ptr<MCP3423> mcp3423) :
  UI()
{
  devices.insert(mcp3423);

  size_t row = 1, col = 1;

  Add(new TimeField(UI::statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  EMPTY();

  Add(new LField<const char*>(statusp, row++, col, 10, "Mode", "", [mcp3423]() {
    static const char* vals[] = { "One-shot", "Continuous" };
    return vals[mcp3423->RT.O_C()];
  }));
  Add(new LField<uint8_t>(statusp, row++, col, 10, "Channel", "", [mcp3423]() {
    return mcp3423->RT.C1_C0() + 1;
  },
  [mcp3423](const char* v) {
    if (strcmp(v, "2") == 0)
      mcp3423->Write((mcp3423->RT.Configuration() & 0x9F) | 0x20);
    else
      mcp3423->Write(mcp3423->RT.Configuration() & 0x9F);
  }));
  Add(new LField<const char*>(statusp, row++, col, 10, "Resolution", "b", [mcp3423]() {
    static const char* vals[] = { "12", "14", "16", "18" };
    return vals[mcp3423->RT.S1_S0()];
  }));
  Add(new LField<uint8_t>(statusp, row++, col, 10, "PGA", "x", [mcp3423]() {
    return 1 << mcp3423->RT.G1_G0();
  }));

  EMPTY();

  Add(new LField<int32_t>(statusp, row++, col, 10, "Reading raw", "", [mcp3423]() {
    return mcp3423->RawReading();
  }));
  Add(new LField<double>(statusp, row++, col, 10, "Reading", "V", [mcp3423]() {
    return mcp3423->Reading();
  }));
}
