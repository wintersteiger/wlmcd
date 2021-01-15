// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <stdexcept>

#include <gpiod.h>

#include "gpio_button.h"

GPIOButton::GPIOButton(const char *chip_path, int offset, bool inverted) :
  chip(NULL),
  line(NULL),
  chip_path(chip_path),
  offset(offset),
  inverted(inverted),
  buffer(0)
{
  snprintf(name, sizeof(name), "GPIO %02d", offset);
  Reset();
}

GPIOButton::~GPIOButton()
{
  Write(false);
  gpiod_line_release(line);
  gpiod_chip_close(chip);
}

void GPIOButton::Reset()
{
  {
    std::lock_guard<std::mutex> lock(mtx);
    if (line) gpiod_line_release(line);
    if (chip) gpiod_chip_close(chip);

    chip = gpiod_chip_open(chip_path);
    if (!chip)
      throw std::runtime_error("gpiod_chip_open failed\n");

    if ((line = gpiod_chip_get_line(chip, offset)) == NULL)
      throw std::runtime_error("gpiod_chip_get_line failed\n");

    buffer = 0 ^ inverted;

    if (gpiod_line_request_output(line, gpio_consumer, buffer) == -1)
      throw std::runtime_error("gpiod_line_request_output failed\n");
  }

  Write(false);
}

bool GPIOButton::Read() {
  std::lock_guard<std::mutex> lock(mtx);
  if ((buffer = gpiod_line_get_value(line)) == -1)
    throw std::runtime_error("gpiod_line_get_value failed\n");
  return (buffer != 0) ^ inverted;
}

bool GPIOButton::ReadBuffered() const
{
  return buffer ^ inverted;
}

void GPIOButton::Write(bool value)
{
  std::lock_guard<std::mutex> lock(mtx);
  buffer = value ^ inverted;
  if (gpiod_line_set_value(line, buffer) == -1)
    throw std::runtime_error("gpiod_line_set_value failed\n");
}

void GPIOButton::UpdateTimed() {}
void GPIOButton::UpdateFrequent() {}
void GPIOButton::UpdateInfrequent() { Read(); }
void GPIOButton::WriteConfig(const std::string &filename) {}
void GPIOButton::ReadConfig(const std::string &filename) {}
