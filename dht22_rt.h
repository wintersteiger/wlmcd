// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _DHT22_RT_H_
#define _DHT22_RT_H_

#include "register_table.h"

#define REG(N, P, A, RW, H, V) REGDECL(uint8_t, uint64_t, N, P, A, RW, H, V)
#define VAR(R, N, P, M, RW, D) VARDECL(uint8_t, uint64_t, R, N, P, M, RW, D)

REGISTER_TABLE_W(DHT22, RegisterTable, uint8_t, uint64_t,
  REG(Data, "Data", 0x00, RO,                                       "",
      VAR(Data, Checksum, "Checksum", 0x00000000000000FF, RO,       "");
      VAR(Data, Humidity, "Humidity", 0x000000FFFF000000, RO,       "");
      VAR(Data, Temperature, "Temperature", 0x00000000007FFF00, RO, "");
  );
);

#endif // _DHT22_RT_H_
