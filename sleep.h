// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _SLEEP_H_
#define _SLEEP_H_

#include <assert.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>

inline void sleep_ms(uint64_t ms)
{
  struct timespec ts = {0};
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  assert(0 <= ts.tv_nsec && ts.tv_nsec <= 999999999);

  while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
    ;
}

inline void sleep_us(uint64_t us)
{
  struct timespec ts = {0};
  ts.tv_sec = us / 1000000;
  ts.tv_nsec = (us % 1000000) * 1000;
  assert(0 <= ts.tv_nsec && ts.tv_nsec <= 999999999);

  while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
    ;
}

inline void sleep_ns(uint64_t ns)
{
  struct timespec ts = {0};
  ts.tv_sec = ns / 1000000000;
  ts.tv_nsec = ns % 1000000000;
  assert(0 <= ts.tv_nsec && ts.tv_nsec <= 999999999);

  while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
    ;
}

#endif // _SLEEP_H_
