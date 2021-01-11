// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _RELAY_DEVICE_H_
#define _RELAY_DEVICE_H_

#include <vector>
#include <mutex>

#include "device.h"

class RelayDevice : public Device<int, bool>
{
public:
  RelayDevice(const char *chip, const std::vector<unsigned> offsets, bool inverted = false);
  virtual ~RelayDevice();

  virtual const char* Name() const { return "Relays"; }

  virtual void WriteConfig(const std::string &filename);
  virtual void ReadConfig(const std::string &filename);

  const std::vector<unsigned>& Offsets() const { return offsets; }

  bool ReadBuffered(size_t pin_index);

  virtual bool Read(const int &addr);
  virtual std::vector<bool> Read(const int &addr, size_t length);

  virtual void Write(const int &addr, const bool &value);
  virtual void Write(const int &addr, const std::vector<bool> &values);

  virtual void Reset();

  void UpdateFrequent();
  void UpdateInfrequent();

protected:
  std::mutex mtx;
  static constexpr const char *gpio_consumer = "relays";
  struct gpiod_chip *chip;
  struct gpiod_line_bulk *bulk;
  std::vector<unsigned> offsets;
  bool inverted;
  std::vector<int> buffer;
};

#endif // _RELAY_DEVICE_H_
