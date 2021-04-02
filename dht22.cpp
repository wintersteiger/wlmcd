// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <unistd.h>

#include <ctime>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <thread>
#include <cinttypes>

#include <gpiod.h>

#include <wiringPi.h>

#include "sleep.h"
#include "dht22.h"
#include "dht22_rt.h"

static const uint64_t MAX_WAIT = 80000;
static const uint64_t MAX_TRIES = 1;

thread_local struct timespec time_point;
const clockid_t clk = CLOCK_BOOTTIME;

static inline uint64_t get_time_ns() {
  // auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
  // return std::chrono::duration_cast<std::chrono::nanoseconds>(t).count();
  clock_gettime(clk, &time_point);
  return (time_point.tv_sec * 1000000000ul) + time_point.tv_nsec;
}

static double estimate_clock_gettime() {
  const size_t n = 100;
  struct timespec ts[n];
  uint64_t xs[n];

  for (size_t i = 0; i < n; i++)
    clock_gettime(clk, &ts[i]);

  for (size_t i = 0; i < n; i++)
    xs[i] = (ts[i].tv_sec * 1000000000ul) + ts[i].tv_nsec;

  return (xs[n-1] - xs[0]) / (double)n;
}

#define TIME(STR, G)                                      \
  {                                                       \
    const size_t n = 1000;                                \
    uint64_t xs[n + 1];                                   \
    uint64_t before = get_time_ns();                      \
    for (size_t i = 0; i < n; i++)                        \
    {                                                     \
      G;                                                  \
    }                                                     \
    uint64_t after = get_time_ns();                       \
    double avg = (after - before) / (double)n;            \
    std::cout << STR << ": " << avg << "ns" << std::endl; \
  }

DHT22::DHT22(const char *gpio_device, int gpio_offset) :
  RT(*new DHT22::RegisterTable(*this)),
  gpio_device(gpio_device),
  gpio_offset(gpio_offset),
  chip(NULL),
  line(NULL),
  read_time(0), tries(0),
  reads(0), bad_reads(0),
  blocked_until(0),
  block_2s(false)
{
  // memset(&sparam, 0, sizeof(sparam));
  // sparam.sched_priority = sched_get_priority_max(SCHED_RR);
  // sched_setscheduler(0, SCHED_RR, &sparam);

  // TIME("Get time", get_time_ns());

  // std::cout << "Estimate: " << estimate_clock_gettime() << "ns" << std::endl;

  // TIME("sleep_us(1)", sleep_us(1));

  // struct timespec ts = {0};
  // TIME("nanosleep(1000)", {
  //   ts.tv_sec = 0;
  //   ts.tv_nsec = 1000;
  //   nanosleep(&ts, NULL);
  // });

  // TIME("delayMicroseconds(1)", {
  //   delayMicroseconds(1);
  // });


  if ((chip = gpiod_chip_open(gpio_device)) == NULL)
    throw std::runtime_error("gpiod_chip_open failed\n");

  if ((line = gpiod_chip_get_line(chip, gpio_offset)) == NULL)
    throw std::runtime_error("gpiod_chip_get_line failed\n");

  // if (gpiod_line_request_input(line, gpio_consumer) == -1)
  //   throw std::runtime_error("gpiod_line_request_input failed\n");

  // TIME("gpiod_line_get_value", { gpiod_line_get_value(line); });

  // gpiod_line_release(line);

  // // Release();

  // wiringPiSetup();

  // TIME("digitalRead", { digitalRead(7); });

  Reset();
  RT.Initialize();
  RT.Refresh(false);
}

DHT22::~DHT22()
{
  Release();
}

void DHT22::Release()
{
  if (line) {
    gpiod_line_release(line);
    line = NULL;
  }

  if (chip) {
    gpiod_chip_close(chip);
    chip = NULL;
  }
}

void DHT22::Reset()
{
  mtx.lock();

  // Release();

  mtx.unlock();
}

void DHT22::WriteConfig(const std::string &filename)
{}

void DHT22::ReadConfig(const std::string &filename)
{}

static std::mutex mtx;
typedef std::vector<std::pair<bool, struct timespec>> times_t;

static int EventCallback(int event_type, unsigned int offset, const struct timespec *timestamp, void *data)
{
  int r = GPIOD_CTXLESS_EVENT_CB_RET_OK;
  times_t *times = (times_t*)data;

  // if (times->size() >= 82) {
  //   printf("limit\n");
  //   r = GPIOD_CTXLESS_EVENT_CB_RET_STOP;
  // }

  switch (event_type) {
    case GPIOD_CTXLESS_EVENT_CB_RISING_EDGE:
      mtx.lock();
      times->push_back(std::make_pair(true, *timestamp));
      clock_gettime(clk, &times->back().second);
      mtx.unlock();
      break;
    case GPIOD_CTXLESS_EVENT_CB_FALLING_EDGE:
      mtx.lock();
      times->push_back(std::make_pair(false, *timestamp));
      clock_gettime(clk, &times->back().second);
      mtx.unlock();
      break;
    case GPIOD_CTXLESS_EVENT_CB_TIMEOUT:
      r = GPIOD_CTXLESS_EVENT_CB_RET_STOP;
      break;
    default:
      printf("unknown edge %d\n", event_type);
      r = GPIOD_CTXLESS_EVENT_CB_RET_STOP;
  }

  return r;
}

uint64_t DHT22::Read(const uint8_t &addr)
{
  mtx.lock();

  // Data is highly dependent on time measurements, so we bump the priority.
  // int prio_before = sched_getscheduler(0);

  // memset(&sparam, 0, sizeof(sparam));
  // sparam.sched_priority = sched_get_priority_max(SCHED_RR);
  // sched_setscheduler(0, SCHED_RR, &sparam);

  uint64_t before = get_time_ns();
  uint64_t all_before = before;
  bool ok = false;
  size_t tries = 0;
  uint64_t data = 0;
  std::pair<uint64_t, uint64_t> ms[42];

  while (!ok && tries++ < MAX_TRIES) {

    if (block_2s)
      while (get_time_ns() < this->blocked_until)
        ;

    ok = true;
    this->reads++;

    if (gpiod_line_request_output(line, gpio_consumer, 1) == -1)
      throw std::runtime_error("gpiod_line_request_output failed\n");

    // warmup
    if (gpiod_line_set_value(line, 1) == -1) {
      printf("errno=%d\n", errno);
      throw std::runtime_error("Could not set GPIO line value A");
    }
    sleep_us(1100);

    // pull pin down for at least 1ms
    if (gpiod_line_set_value(line, 0) == -1)
      throw std::runtime_error("Could not set GPIO line value B");
    sleep_us(1100);

    // pull up for 20-40us
    if (gpiod_line_set_value(line, 1) == -1)
      throw std::runtime_error("Could not set GPIO line value C");
    sleep_us(20);

    gpiod_line_release(line);

    times_t times;
    times.reserve(82);
    struct timespec ts_before;
    clock_gettime(clk, &ts_before);

    // if (gpiod_line_request_both_edges_events(line, gpio_consumer) == -1)
    //   throw std::runtime_error("gpiod_line_request_both_edges_events failed");

    // for (size_t i=0; i < 42; i++)
    // {
    //   struct timespec ts = { 0, 80000 };
    //   int rv = gpiod_line_event_wait(this->line, &ts);
    //   if (rv == -1)
    //     throw std::runtime_error("gpiod_line_event_wait failed");
    //   else if (rv == 0) {
    //     printf("timeout\n");
    //     break;
    //   }
    //   else {
    //     struct gpiod_line_event event;
    //     if ((rv = gpiod_line_event_read(this->line, &event)) != 0)
    //       throw std::runtime_error("gpiod_line_event_read failed");
    //     times.push_back(std::make_pair(event.event_type == 1, event.ts));
    //     // clock_gettime(clk, &times.back().second);
    //   }
    // }

    // gpiod_line_release(line);

    {
      // Switch to input mode
      if (gpiod_line_request_input(line, gpio_consumer) == -1)
        throw std::runtime_error("Could not switch GPIO line direction");

      int last = 1;
      while (times.size() < 82 && (get_time_ns() - before) < 6720000ul)
      {
        int val = gpiod_line_get_value(line);
        if (val != last) {
          times.resize(times.size()+1);
          times.back().first = val == 1;
          clock_gettime(clk, &times.back().second);
          last = val;
        }
      }

      gpiod_line_release(line);
    }

    size_t j = times[0].first ? 3 : 2;
    for (size_t i = 0; i < 41; i++) {
      uint64_t time_j2 = (times[j-2].second.tv_sec * 1000000000ul) + times[j-2].second.tv_nsec;
      uint64_t time_j1 = (times[j-1].second.tv_sec * 1000000000ul) + times[j-1].second.tv_nsec;
      uint64_t time_j0 = (times[j].second.tv_sec * 1000000000ul) + times[j].second.tv_nsec;
      ms[i].first = time_j1 - time_j2;
      ms[i].second = time_j0 - time_j1;
      j += 2;
    }

    // struct timespec timeout = { .tv_sec = 0, .tv_nsec = 6720000 };
    // if (gpiod_ctxless_event_monitor(gpio_device, GPIOD_CTXLESS_EVENT_BOTH_EDGES,
    //                                 gpio_offset, false, "WLCMD-DHT22", &timeout,
    //                                 NULL /* poll callback */,
    //                                 EventCallback,
    //                                 &times) != 0)
    //   throw("could not start GPIO event monitor");


    // for (size_t i = 0; i < 42; i++)
    // {
    //   uint64_t mid = before, after;
    //   while ((digitalRead(7) == LOW) && (mid - before) < MAX_WAIT) {
    //     mid = get_time_ns();
    //     sleep_us(1);
    //   }
    //   after = mid;
    //   while ((digitalRead(7) == HIGH) && (after - mid) < MAX_WAIT) {
    //     after = get_time_ns();
    //     sleep_us(1);
    //   }
    //   ms[i].first = mid - before;
    //   ms[i].second = after - mid;
    //   before = after;
    // }

    // printf("OLD:");
    // for (size_t i = 0; i < 41; i++)
    //   printf(" %" PRIu64 "/%" PRIu64 "", ms[i].first, ms[i].second);
    // printf("\n");
    // printf("New:");
    // uint64_t last = (ts_before.tv_sec * 1000000000ul) + ts_before.tv_nsec;
    // for (auto &t : times) {
    //   uint64_t time = (t.second.tv_sec * 1000000000ul) + t.second.tv_nsec;
    //   if (time < last) printf("NON-CAUSAL\n");
    //   printf(" %d:%" PRIu64 "", t.first, (time-last)/1000);
    //   last = time;
    // }
    // printf("\n");

    // Sensor should pull low for 80us then high for 80us
    // if (ms[0].second < 75000 || ms[0].second > 85000) {
    //   printf("fail: 0 (%" PRIu64 ")\n", ms[0].second);
    //   ok = false;
    // }

    for (size_t i=1; i < 41; i++) {
      // if (ms[i].second >= MAX_WAIT) {
      //   ok = false;
      //   printf("fail: %u (%u)\n", i, ms[i].second);
      // }

      data <<= 1;
      if (ms[i].second > 35000) // && ms[i].second < MAX_WAIT)
        data |= 1;
    }

    if (ok) {
      uint8_t checksum = 0, checksum_dev = 0;
      checksum = ((data >> 8) & 0xFF) + ((data >> 16) & 0xFF) + ((data >> 24) & 0xFF) + ((data >> 32) & 0xFF);
      checksum_dev = data & 0xFF;
      checksum_ok = checksum == checksum_dev;

      if (!checksum_ok) {
        ok = false;
        printf("crc failed\n");
      }
    }

    // Release();

    if (!ok) {
      this->bad_reads++;
      sleep_ms(2000);
      break;
    }
  }

  uint64_t all_after = get_time_ns();
  this->read_time = (all_after >= before) ? all_after - all_before : 0;
  this->tries = tries;

  // Wait at last 2 seconds
  this->blocked_until = get_time_ns() + 2000000000ul;

  mtx.unlock();

  // sparam.sched_priority = prio_before;
  // sched_setscheduler(0, SCHED_RR, &sparam);

  return data;
}

std::vector<uint64_t> DHT22::Read(const uint8_t &addr, size_t length)
{
  std::vector<uint64_t> res(length, 0);
  for (size_t i=0; i < length; i++)
    res[i] = Read(addr);
  return res;
}

void DHT22::UpdateTimed()
{
  RT.Refresh(false);
}

void DHT22::UpdateFrequent()
{
}

void DHT22::UpdateInfrequent()
{
}

float DHT22::Temperature() const {
  uint64_t data = RT.Data();
  float r = RT._vTemperature(RT.Data()) / 10.0f;
  if (data & 0x800000) r = -r;
  return r;
}

float DHT22::Humidity() const {
  uint64_t data = RT.Data();
  return RT._vHumidity(data) / 10.0f;
}

bool DHT22::ChecksumOK() const { return checksum_ok; }

uint64_t DHT22::LastReadTime() const { return read_time; }

uint64_t DHT22::Tries() const { return tries; }

uint64_t DHT22::Reads() const { return reads; }

uint64_t DHT22::BadReads() const { return bad_reads; }

void DHT22::RegisterTable::Refresh(bool frequent)
{
  if (buffer.size() != 1)
    buffer.resize(1, 0);
  buffer[0] = device.Read(0);
}

void DHT22::RegisterTable::WriteFile(const std::string &filename)
{
}

void DHT22::RegisterTable::ReadFile(const std::string &filename)
{

}

void DHT22::RegisterTable::Write(const Register<uint8_t, uint64_t> &reg, const uint64_t &value)
{
}
