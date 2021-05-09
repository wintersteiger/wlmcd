// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>

#include "bme680.h"
#include "bme680_rt.h"

BME680::BME680(const std::string &bus, uint8_t device_address) :
  I2CDevice<uint8_t, uint8_t>(bus, device_address),
  RT(new RegisterTable(*this))
{
  Reset();
  if (RT->Id() != 0x61)
    throw std::runtime_error("Invalid chip ID or unresponsive BME680");
}

BME680::~BME680()
{
  delete(RT);
}

void BME680::Reset()
{
  RT->Initialize();
  I2CDevice::Reset();
  Write(RT->_rReset, 0xB6);
  RT->Refresh(false);
}

void BME680::Write(std::ostream &os)
{
  RT->Write(os);
}

void BME680::Read(std::istream &is)
{
  RT->Read(is);
}

void BME680::UpdateTimed()
{
  RT->Refresh(false);
}

void BME680::RegisterTable::Refresh(bool frequent)
{
  for (auto reg : registers)
    buffer[reg->Address()] = device.Read(*reg);
}

void BME680::RegisterTable::Write(std::ostream &os) {}

void BME680::RegisterTable::Read(std::istream &is) {}
