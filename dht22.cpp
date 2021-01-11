// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <ctime>
#include <algorithm>

#include <wiringPi.h>

#include "dht22.h"
#include "dht22_rt.h"

static const uint64_t MAX_WAIT = 80000;
static const uint64_t MAX_TRIES = 10;
static const float INTERVAL = 60.0; /* seconds */

struct timespec time_point;
clockid_t clk = CLOCK_BOOTTIME;

static inline uint64_t get_time_ns() {
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

DHT22::DHT22(int pin)
    : RT(*new DHT22::RegisterTable(*this)), pin(pin), read_time(0), tries(0),
      reads(0), bad_reads(0), blocked_until(0),
      block_2s(false) {
  pullUpDnControl(pin, PUD_OFF);
  RT.Initialize();
}

DHT22::~DHT22()
{
}

void DHT22::Reset()
{}

void DHT22::WriteConfig(const std::string &filename)
{}

void DHT22::ReadConfig(const std::string &filename)
{}

uint64_t DHT22::Read(const uint8_t &addr)
{
  mtx.lock();

  uint64_t before = get_time_ns();
  uint64_t all_before = before;
  bool ok = false;
  size_t tries = 0;
  uint64_t data = 0;
  std::pair<uint64_t, uint64_t> ms[42];

  while (!ok && ++tries < MAX_TRIES) {

    if (block_2s)
      while (get_time_ns() < this->blocked_until)
        ;

    ok = true;
    this->reads++;

    // warmup
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delayMicroseconds(1100);

    // pull pin down for at least 1ms
    digitalWrite(pin, LOW);
    delayMicroseconds(1100);

    digitalWrite(pin, HIGH);
    delayMicroseconds(40);

    // Switch to input mode
    pinMode(pin, INPUT);

    before = get_time_ns();

    for (size_t i = 0; i < 42; i++) {
      uint64_t mid = before, after;
      while (digitalRead(pin) == LOW && (mid - before) < MAX_WAIT) {
        mid = get_time_ns();
        delayMicroseconds(1);
      }
      after = mid;
      while (digitalRead(pin) == HIGH && (after - mid) < MAX_WAIT) {
        after = get_time_ns();
        delayMicroseconds(1);
      }
      ms[i].first = mid - before;
      ms[i].second = after - mid;
      before = after;

    }

    // Sensor should pull low for 80us then high for 80us
    if (ms[0].second < 75000 || ms[0].second > 85000)
      ok = false;

    for (size_t i=1; i < 41; i++) {
      if (ms[i].second >= MAX_WAIT)
        ok = false;

      data <<= 1;
      if (ms[i].second > 48000 && ms[i].second < MAX_WAIT)
        data |= 1;
    }

    if (ok) {
      uint8_t checksum = 0, checksum_dev = 0;
      checksum = ((data >> 8) & 0xFF) + ((data >> 16) & 0xFF) + ((data >> 24) & 0xFF) + ((data >> 32) & 0xFF);
      checksum_dev = data & 0xFF;
      checksum_ok = checksum == checksum_dev;

      // if (!checksum_ok)
      //  ok = false;

      if (!checksum_ok)
        this->bad_reads++;
    }

    if (!ok)
      this->bad_reads++;

    if (!ok)
      delay(2000);
  }

  uint64_t all_after = get_time_ns();
  this->read_time = (all_after >= before) ? all_after - all_before : 0;
  this->tries = tries;

  // Prepare for next read
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);

  // Wait at last 2 seconds
  this->blocked_until = get_time_ns() + 2000000000ul;

  mtx.unlock();

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
