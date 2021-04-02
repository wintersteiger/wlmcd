// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SLEEP_H_
#define _SLEEP_H_

#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>

#include <algorithm>

inline void sleep_ms(unsigned ms)
{
  struct timespec ts = {0};
  ts.tv_sec = ms / 1000ul;
  ts.tv_nsec = (ms % 1000ul) * 1000000ul;

  struct timespec *pdur = &duration, *prem = &remaining;
  while (nanosleep(pdur, prem) == -1)
    std::swap(pdur, prem);
}

inline void sleep_us(unsigned us)
{
  if (us < 100) {
    struct timeval tNow, tulong, tEnd;

    gettimeofday(&tNow, NULL);
    tulong.tv_sec  = us / 1000000;
    tulong.tv_usec = us % 1000000;
    timeradd(&tNow, &tulong, &tEnd);

    while (timercmp(&tNow, &tEnd, <))
      gettimeofday(&tNow, NULL);
  }
  else {
    struct timespec ts = {0};
    ts.tv_sec = us / 1000000ul;
    ts.tv_nsec = (us % 1000000ul) * 1000ul;

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
      ;
  }
}

inline void sleep_ns(unsigned ns)
{
  struct timespec ts = {0};
  ts.tv_sec = ns / 1000000000ul;
  ts.tv_nsec = ns % 1000000000ul;

  struct timespec *pdur = &duration, *prem = &remaining;
  while (nanosleep(pdur, prem) == -1)
    std::swap(pdur, prem);
}

#endif // _SLEEP_H_
