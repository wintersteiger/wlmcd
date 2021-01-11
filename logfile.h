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
  virtual ~LogFile();

  virtual const char* Name() const { return "LogFile"; }

  virtual void WriteConfig(const std::string &filename);
  virtual void ReadConfig(const std::string &filename);

  virtual void Reset();

  void UpdateTimed();
  void UpdateFrequent();
  void UpdateInfrequent();

protected:
  std::mutex mtx;
  const char *filename;
  FILE *file;
  std::function<std::vector<std::string>()> fun;
};

#endif // _LOGFILE_DEVICE_H_
