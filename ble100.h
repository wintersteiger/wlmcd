// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _GENIUS_BLE100_
#define _GENIUS_BLE100_

#include "device.h"

class BLE100 : public Device<uint8_t, uint8_t>
{
public:
  BLE100();
  virtual ~BLE100();
};

#endif // _GENIUS_BLE100_