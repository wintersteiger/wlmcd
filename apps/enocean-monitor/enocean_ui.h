// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _ENOCEAN_UI_H_
#define _ENOCEAN_UI_H_

#include <memory>
#include <vector>
#include <map>

#include <ui.h>

class DeviceBase;
class BME280;

namespace EnOcean {
  class Gateway;
};

class EnOceanUI : public UI {
public:
  EnOceanUI(const std::unique_ptr<EnOcean::Gateway>& gateway,
            const std::vector<std::shared_ptr<DeviceBase>> radio_devices,
            std::shared_ptr<BME280> bme280,
            uint64_t *num_manual);
  virtual ~EnOceanUI();

  void Update(bool full) { UI::Update(full); }

  virtual std::string Name() const { return "EnOcean"; }
};

#endif
