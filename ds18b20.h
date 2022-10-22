// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _DS18B20_H_
#define _DS18B20_H_

#include <vector>
#include <mutex>

#include "device.h"

class DS18B20 : public Device<int, double>
{
public:
  DS18B20(const char *path, const char *id);
  virtual ~DS18B20();

  virtual const char* Name() const { return "DS18B20"; }

  using Device::Read;
  using Device::Write;

  virtual void Write(std::ostream &os) const;
  virtual void Read(std::istream &is);

  virtual double Read(const int &addr);
  virtual std::vector<double> Read(const int &addr, size_t length);

  virtual void Write(const int &addr, const double &value);
  virtual void Write(const int &addr, const std::vector<double> &values);

  virtual void Reset();

  void UpdateTimed();
  void UpdateFrequent();
  void UpdateInfrequent();

  double Temperature() const { return buffer; }

  const char *ID() const { return id; }

protected:
  std::mutex mtx;
  const char *path, *id;
  double buffer;
  FILE *file;
  size_t counter;
};

#endif // _DS18B20_H_
