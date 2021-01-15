// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _GPIODEVICE_H_
#define _GPIODEVICE_H_

#include "device.h"

struct gpiod_chip;
struct gpiod_line;
struct gpiod_line_bulk;

typedef struct {
  size_t chip;
  size_t line;
} gpio_address_t;

class GPIODevice : public Device<gpio_address_t, int>
{
public:
  GPIODevice(std::vector<Decoder*> decoders = {});
  virtual ~GPIODevice();

  virtual const char* Name() const { return "GPIO"; }

  virtual int Read(const gpio_address_t &addr);
  virtual std::vector<int> Read(const gpio_address_t &addr, size_t length);

  virtual void Write(const gpio_address_t &addr, const int &value);
  virtual void Write(const gpio_address_t &addr, const std::vector<int> &values);

  virtual void UpdateFrequent();
  virtual void UpdateInfrequent();

  virtual void WriteConfig(const std::string &filename);
  virtual void ReadConfig(const std::string &filename);

  size_t NumChips() const { return chips.size(); }
  size_t NumLines(int chip) const { return chips[chip].num_lines; }

  typedef struct {
    struct gpiod_chip* chip;
    std::string name;
    std::string label;
    size_t num_lines;
  } chip_state_t;

  const chip_state_t& ChipState(int inx) const { return chips[inx]; }

  typedef struct {
    struct gpiod_line* line;
    int value;
    unsigned int offset;
    const char *name;
    const char *consumer;
    int direction;
    int active_state;
    // int bias;
    int used;
    int open_drain;
    int open_source;
  } line_state_t;

  const line_state_t& LineState(const gpio_address_t &addr) const {
    return line_buffer[addr.chip][addr.line];
    }

protected:
  std::vector<chip_state_t> chips;
  std::vector<std::vector<line_state_t>> line_buffer;
};

#endif
