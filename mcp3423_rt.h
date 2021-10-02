// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _MCP3423_RT_H_
#define _MCP3423_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint8_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint8_t, R, N, P, M, RW, D)

REGISTER_TABLE(MCP3423, RegisterTable, uint8_t, uint8_t,
  REG(Configuration, "Configuration", 0x00, RW,                 "Configuration",
    VAR(Configuration, RDY, "RDY", 0x80, RW,                    "");
    VAR(Configuration, C1_C0, "C1-C0", 0x60, RW,                "");
    VAR(Configuration, O_C, "O/C", 0x10, RW,                    "");
    VAR(Configuration, S1_S0, "S1-S0", 0x0C, RW,                "");
    VAR(Configuration, G1_G0, "G1-G0", 0x03, RW,                "");
  );
);

#endif // _MCP3423_RT_H_
