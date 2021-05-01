// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>

#include "es9018k2m.h"
#include "es9018k2m_rt.h"

ES9018K2M::ES9018K2M(const std::string &bus, uint8_t device_address) :
  I2CDevice<uint8_t, uint8_t>(bus, device_address),
  RTS(*(new RegisterTableSet(*this)))
{
  Reset();

  RTS.Main.Refresh(false);

  uint8_t cid = RTS.Main.CHIP_ID();
  uint8_t rev = RTS.Main.REVISION();
  if (cid != 0x04)
    throw std::runtime_error("Unexpected ES9018K2M chip ID: " + std::to_string(cid) + "." + std::to_string(rev));
}

ES9018K2M::~ES9018K2M()
{
  delete(&RTS);
}

void ES9018K2M::Reset()
{
  I2CDevice::Reset();
  // Write(RTS.Main._rSystemSettings, 0x01);
}

void ES9018K2M::Write(std::ostream &os)
{
  RTS.Main.Write(os);
}

void ES9018K2M::Read(std::istream &is)
{
  RTS.Main.Read(is);
}

void ES9018K2M::UpdateTimed()
{
  RTS.Main.Refresh(false);
  RTS.Consumer.Refresh(false);
  // RTS.Professional.Refresh(false);
}
