// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SLEEP_H_
#define _SLEEP_H_

#include <time.h>

#include <algorithm>

inline void sleep_ms(unsigned ms)
{
  struct timespec duration = {0}, remaining = {0};
  duration.tv_sec = 0;
  duration.tv_nsec = ms * 1000000L;

  struct timespec *pdur = &duration, *prem = &remaining;
  while (nanosleep(pdur, prem) == -1)
    std::swap(pdur, prem);
}

inline void sleep_us(unsigned us)
{
  struct timespec duration = {0}, remaining = {0};
  duration.tv_sec = 0;
  duration.tv_nsec = us * 1000L;

  struct timespec *pdur = &duration, *prem = &remaining;
  while (nanosleep(pdur, prem) == -1)
    std::swap(pdur, prem);
}

inline void sleep_ns(unsigned ns)
{
  struct timespec duration = {0}, remaining = {0};
  duration.tv_sec = 0;
  duration.tv_nsec = ns * 1000L;

  struct timespec *pdur = &duration, *prem = &remaining;
  while (nanosleep(pdur, prem) == -1)
    std::swap(pdur, prem);
}

#endif // _SLEEP_H_
