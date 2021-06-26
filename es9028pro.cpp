// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>

#include "es9028pro.h"
#include "es9028pro_rt.h"

ES9028PRO::ES9028PRO(const std::string &bus, uint8_t device_address, double F_MCLK) :
  I2CDevice<uint8_t, uint8_t>(bus, device_address),
  RTS(new RegisterTableSet(*this)),
  F_MCLK(F_MCLK)
{
  Reset();

  RTS->Main.Refresh(false);

  uint8_t cid = RTS->Main.chip_id();
  if (cid != 0x28)
    throw std::runtime_error("Unexpected ES9028PRO chip ID: " + std::to_string(cid));
}

ES9028PRO::~ES9028PRO()
{
  delete(RTS);
}

void ES9028PRO::Reset()
{
  I2CDevice::Reset();
  // Write(RTS->Main._rSystemRegisters, 0x01); // soft reset
}

void ES9028PRO::Write(std::ostream &os)
{
  RTS->Main.Write(os);
}

void ES9028PRO::Read(std::istream &is)
{
  RTS->Main.Read(is);
}

void ES9028PRO::UpdateTimed()
{
  RTS->Main.Refresh(false);
  RTS->Consumer.Refresh(false);
  // RTS->Professional.Refresh(false);
}

double ES9028PRO::FSR() const {
  return (RTS->Main.dpll_num() * F_MCLK) / pow(2.0, 32);
}

double ES9028PRO::VolumeRampRate() const {
  return ((1 << RTS->Main.volume_rate()) * FSR()) / 512;
}

double ES9028PRO::SoftStartTime() const {
  return 4096.0 * ((1 << (RTS->Main.soft_start_time() + 1)) / F_MCLK);
}