// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CPU_TEMP_DEVICE_H_
#define _CPU_TEMP_DEVICE_H_

#include <vector>
#include <mutex>

#include "device.h"

class CPUTempDevice : public DeviceBase
{
public:
  CPUTempDevice(uint32_t frequency_divider = 1);
  virtual ~CPUTempDevice();

  virtual const char* Name() const override { return "CPU Temp"; }

  virtual void Write(std::ostream &os) const override;
  virtual void Read(std::istream &is) override;

  virtual void Reset() override;

  void UpdateFrequent() override;
  void UpdateInfrequent() override;
  void UpdateTimed() override;

  float Get() const;

protected:
  std::mutex mtx;
  float temperature;
  uint32_t frequency_divider, counter;
};

#endif // _CPU_TEMP_DEVICE_H_
