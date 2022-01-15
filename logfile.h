// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _LOGFILE_DEVICE_H_
#define _LOGFILE_DEVICE_H_

#include <vector>
#include <mutex>
#include <string>
#include <functional>

#include "device.h"

class LogFile : public DeviceBase
{
public:
  LogFile(const char *filename, std::function<std::vector<std::string>()> fun);
  LogFile(const std::string &filename, std::function<std::vector<std::string>()> fun);
  LogFile(const std::string &filename, std::vector<std::string> header, std::function<std::vector<std::string>()> fun);
  virtual ~LogFile();

  virtual const char* Name() const override { return "LogFile"; }

  virtual void Write(std::ostream &os) const override;
  virtual void Read(std::istream &is) override;

  virtual void Reset() override;

  virtual void UpdateTimed() override;
  virtual void UpdateFrequent() override;
  virtual void UpdateInfrequent() override;

protected:
  std::mutex mtx;
  std::string filename;
  FILE *file;
  std::function<std::vector<std::string>()> fun;
};

#endif // _LOGFILE_DEVICE_H_
