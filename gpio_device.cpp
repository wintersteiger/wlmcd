// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <stdexcept>

#include <gpiod.h>

#include "gpio_device.h"

GPIODevice::GPIODevice(std::vector<Decoder*> decoders) :
  Device<gpio_address_t, int>(decoders)
{
  UpdateInfrequent();
}

GPIODevice::~GPIODevice()
{
  for (auto cs : chips)
    gpiod_chip_close(cs.chip);
}

int GPIODevice::Read(const gpio_address_t &addr)
{
  int r = 0;

  if (((unsigned)addr.chip) >= line_buffer.size() ||
      ((unsigned)addr.line) >= line_buffer[addr.chip].size())
      throw std::runtime_error("no such GPIO address");

  const line_state_t &lb = line_buffer[addr.chip][addr.line];
  struct gpiod_line *line = lb.line;

  // int req = gpiod_line_request_input(line, "monitor");
  // if (r == -1)
  //   throw std::runtime_error(std::string("line input request failed for " +
  //                                         std::to_string(addr.chip) + "/" +
  //                                         std::to_string(addr.line)));

  // if (req == -1)
  //   return 0;

  // if (!gpiod_line_is_requested(line))
  //   return 0;

  r = gpiod_line_get_value(line);

  // if (r == -1)
  //   throw std::runtime_error(std::string("line value request failed for " +
  //                                         std::to_string(addr.chip) + "/" +
  //                                         std::to_string(addr.line)));

  // gpiod_line_release(line);

  // r = gpiod_line_is_used(line);
  // r = lb.direction == GPIOD_LINE_DIRECTION_INPUT;
  // r = lb.active_state == GPIOD_LINE_ACTIVE_STATE_LOW;

  return r;
}

std::vector<int> GPIODevice::Read(const gpio_address_t &addr, size_t length)
{
  std::vector<int> res(length, 0);
  gpio_address_t a = addr;
  for (size_t i = 0; i < length; i++) {
    res[i] = Read(a);
    a.line++;
  }
  return res;
}

void GPIODevice::Write(const gpio_address_t &addr, const int &value)
{
}

void GPIODevice::Write(const gpio_address_t &addr, const std::vector<int> &values)
{
}

void GPIODevice::UpdateFrequent()
{
  UpdateInfrequent();
}

void GPIODevice::UpdateInfrequent()
{
  size_t j = 0;
  struct gpiod_chip *chip;

  struct gpiod_chip_iter *cit = gpiod_chip_iter_new();
  if (cit) {
    gpiod_foreach_chip_noclose(cit, chip)
    {
      if (j >= chips.size())
        chips.resize(j+1, { NULL, "", "", 0 });

      if (!chips[j].chip)
        chips[j] = {
          .chip=chip,
          .name=gpiod_chip_name(chip),
          .label=gpiod_chip_label(chip),
          .num_lines=gpiod_chip_num_lines(chip) };

      j++;
    }
    gpiod_chip_iter_free_noclose(cit);
    chips.resize(j);
    line_buffer.resize(j);
  }

  for (size_t i=0; i < chips.size(); i++) {
    j = 0;
    line_buffer[i].resize(chips[i].num_lines);
    struct gpiod_line *line;
    struct gpiod_line_iter *lit = gpiod_line_iter_new(chips[i].chip);
    if (lit) {
      gpiod_foreach_line(lit, line)
      {
        line_state_t& l = line_buffer[i][j++];

        l.line = line;
        l.value = -1;
        l.offset = gpiod_line_offset(line);
        l.name = gpiod_line_name(line);
        l.consumer = gpiod_line_consumer(line);
        l.direction = gpiod_line_direction(line);
        l.active_state = gpiod_line_active_state(line);
        l.used = gpiod_line_is_used(l.line);
        l.open_drain = gpiod_line_is_open_drain(l.line);
        l.open_source = gpiod_line_is_open_source(l.line);

        if (!gpiod_line_is_free(line))
          continue;

        struct gpiod_line_request_config config = {
          .consumer = "WLMCD-GPIO",
          .request_type = GPIOD_LINE_REQUEST_DIRECTION_AS_IS,
          .flags = 0
        };

        if (l.offset >= 34)
          continue;

        if (gpiod_line_request(line, &config, 0) == 0) {
          l.value = gpiod_line_get_value(line);
          gpiod_line_release(line);
        }
      }
      gpiod_line_iter_free(lit);
    }
  }
}

void GPIODevice::WriteConfig(const std::string &filename) {}
void GPIODevice::ReadConfig(const std::string &filename) {}
