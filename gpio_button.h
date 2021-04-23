// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _GPIO_BUTTON_H_
#define _GPIO_BUTTON_H_

#include <mutex>

#include "device.h"

class GPIOButton : public DeviceBase {
public:
  GPIOButton(const char *chip_path, int offset, bool inverted=false);
  virtual ~GPIOButton();

  virtual const char* Name() const { return name; }

  virtual void UpdateTimed();
  virtual void UpdateFrequent();
  virtual void UpdateInfrequent();

  using DeviceBase::Read;
  using DeviceBase::Write;

  virtual void Write(std::ostream &os);
  virtual void Read(std::istream &is);

  virtual void Reset();

  bool Read();
  bool ReadBuffered() const;
  void Write(bool value);

protected:
  std::mutex mtx;
  char name[256];
  static constexpr const char *gpio_consumer = "relays";
  struct gpiod_chip *chip;
  struct gpiod_line *line;
  const char *chip_path;
  unsigned offset;
  bool inverted;
  int buffer;
};

#endif // _GPIO_BUTTON_H_
