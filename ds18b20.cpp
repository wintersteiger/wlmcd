// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <stdio.h>

#include "ds18b20.h"

DS18B20::DS18B20(const char *path, const char *id) :
  path(path),
  id(id),
  buffer(0.0),
  counter(0)
{
  Reset();
}

DS18B20::~DS18B20()
{
}

void DS18B20::Write(std::ostream &os)
{
  throw std::runtime_error("not supported");
}

void DS18B20::Read(std::istream &is)
{
  throw std::runtime_error("not supported");
}

double DS18B20::Read(const int &addr)
{
  int r = 0;
  std::lock_guard<std::mutex> lock(mtx);
  if ((file = fopen(path, "r")) == NULL)
		throw std::runtime_error("Could not open sysfs path");
  if (fseek(file, 0, SEEK_SET) == -1 ||
      fscanf(file, "%d", &r) != 1 ||
      fclose(file) != 0)
    return std::numeric_limits<double>::quiet_NaN();
	buffer = r / 1000.0;
  return buffer;
}

std::vector<double> DS18B20::Read(const int &addr, size_t length)
{
  throw std::runtime_error("not supported");
}

void DS18B20::Write(const int &addr, const double &value)
{
  throw std::runtime_error("not supported");
}

void DS18B20::Write(const int &addr, const std::vector<double> &values)
{
  throw std::runtime_error("not supported");
}

void DS18B20::Reset()
{
  Read(0);
}

void DS18B20::UpdateTimed()
{
  if (counter == 0)
    Read(0);
  counter = (counter + 1) % 5;
}

void DS18B20::UpdateFrequent()
{
}

void DS18B20::UpdateInfrequent()
{
}
