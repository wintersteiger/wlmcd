// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <sys/time.h>

#include <chrono>

#include "logfile.h"

LogFile::LogFile(const char *filename, std::function<std::vector<std::string>()> fun) :
  filename(filename),
  file(NULL),
  fun(fun)
{
  Reset();
}

LogFile::LogFile(const std::string &filename, std::function<std::vector<std::string>()> fun) :
  filename(filename),
  file(NULL),
  fun(fun)
{
  Reset();
}

LogFile::~LogFile()
{
  if (file)
    fclose(file);
}

void LogFile::Write(std::ostream &os) {}
void LogFile::Read(std::istream &is) {}

void LogFile::Reset()
{
  DeviceBase::Reset();
  if (file)
    fclose(file);
  file = fopen(filename.c_str(), "ab");
}

void LogFile::UpdateTimed()
{
  mtx.lock();
  static char time_str[2048] = "???";
  struct timeval tv;
  struct tm *tm;
  gettimeofday(&tv, NULL);
  if ((tm = localtime(&tv.tv_sec)) != NULL) {
    size_t n = strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm);
    snprintf(time_str+n, sizeof(time_str)-n, ".%06ld", tv.tv_usec);
  }
  fprintf(file, "%s", time_str);
  std::vector<std::string> columns = fun();
  for (const std::string &s : columns)
    fprintf(file, ",%s", s.c_str());
  fprintf(file, "\n");
  fflush(file);
  mtx.unlock();
}

void LogFile::UpdateFrequent() {}

void LogFile::UpdateInfrequent() {}
