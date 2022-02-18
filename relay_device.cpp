// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <gpiod.h>

#include "relay_device.h"

RelayDevice::RelayDevice(const char *chip_path, const std::vector<unsigned> offsets, bool inverted) :
  offsets(offsets),
  inverted(inverted)
{
  buffer.resize(offsets.size(), 0 ^ inverted);

  chip = gpiod_chip_open(chip_path);
  if (!chip)
    throw std::runtime_error("gpiod_chip_open failed\n");

  if ((bulk = (struct gpiod_line_bulk*) malloc(sizeof(struct gpiod_line_bulk))) == 0)
    throw std::bad_alloc();

  gpiod_line_bulk_init(bulk);

  if (gpiod_chip_get_lines(chip, this->offsets.data(), this->offsets.size(), bulk) != 0)
    throw std::runtime_error("gpiod_chip_get_lines failed\n");

  if (gpiod_line_request_bulk_output(bulk, gpio_consumer, buffer.data()) != 0)
    throw std::runtime_error("gpiod_line_request_bulk_output failed\n");

  if (gpiod_chip_get_lines(chip, this->offsets.data(), this->offsets.size(), bulk) != 0)
    throw std::runtime_error("gpiod_chip_get_lines failed\n");

  Reset();
}

RelayDevice::~RelayDevice()
{
  std::vector<bool> tmp(offsets.size(), false ^ inverted);
  for (size_t i=0; i < offsets.size(); i++)
    tmp[i] = false ^ inverted;
  Write(0, tmp);
  gpiod_line_release_bulk(bulk);
  free(bulk);
}

void RelayDevice::Write(std::ostream &os) const
{
}

void RelayDevice::Read(std::istream &is)
{
}

bool RelayDevice::Read(const int &addr)
{
  mtx.lock();
  if (gpiod_line_get_value_bulk(bulk, buffer.data()) != 0)
    throw std::runtime_error("gpiod_line_set_value_bulk failed\n");
  bool r = (buffer[addr] > 0) ^ inverted;
  mtx.unlock();
  return r;
}

std::vector<bool> RelayDevice::Read(const int &addr, size_t length)
{
  std::vector<bool> res(length, false);
  mtx.lock();
  if (gpiod_line_get_value_bulk(bulk, buffer.data()) != 0)
    throw std::runtime_error("gpiod_line_set_value_bulk failed\n");
  for (size_t i=0; i < length; i++)
    res[i] = (buffer[addr+i] > 0) ^ inverted;
  mtx.unlock();
  return res;
}

void RelayDevice::Write(const int &addr, const bool &value)
{
  mtx.lock();
  buffer[addr] = (value ^ inverted) ? 1 : 0;
  if (gpiod_line_set_value_bulk(bulk, buffer.data()) != 0)
    throw std::runtime_error("gpiod_line_set_value_bulk failed\n");
  mtx.unlock();
}

void RelayDevice::Write(const int &addr, const std::vector<bool> &values)
{
  mtx.lock();
  for (size_t i=0; i < values.size(); i++)
    buffer[addr+i] = (values[i] ^ inverted) ? 1 : 0;
  if (gpiod_line_set_value_bulk(bulk, buffer.data()) != 0)
    throw std::runtime_error("gpiod_line_set_value_bulk failed\n");
  mtx.unlock();
}

bool RelayDevice::ReadBuffered(size_t addr)
{
  mtx.lock();
  bool r = (buffer[addr] > 0) ^ inverted;
  mtx.unlock();
  return r;
}

void RelayDevice::Reset()
{
  mtx.lock();
  buffer.resize(offsets.size(), false ^ inverted);
  mtx.unlock();
}

void RelayDevice::UpdateFrequent()
{
  Read(0, offsets.size());
}

void RelayDevice::UpdateInfrequent()
{
}
