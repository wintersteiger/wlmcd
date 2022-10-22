// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <fstream>

#include "cpu_temp.h"

CPUTempDevice::CPUTempDevice(uint32_t frequency_divider) :
  frequency_divider(frequency_divider),
  counter(-1)
{
  UpdateTimed();
}

CPUTempDevice::~CPUTempDevice()
{
}

void CPUTempDevice::Write(std::ostream &os) const
{
}

void CPUTempDevice::Read(std::istream &is)
{
}

void CPUTempDevice::Reset()
{
}

void CPUTempDevice::UpdateFrequent()
{
}

void CPUTempDevice::UpdateInfrequent()
{
}

void CPUTempDevice::UpdateTimed()
{
  counter = (counter + 1) % frequency_divider;
  if (counter == 0)
  {
    try {
      std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
      uint32_t t;
      f >> t;
      temperature = t / 1000.0f;
    }
    catch (...) {}
  }
}

float CPUTempDevice::Get() const
{
  return temperature;
}