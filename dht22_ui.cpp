// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cmath>
#include <curses.h>
#include <limits>
#include <sstream>
#include <iomanip>

#include "field.h"
#include "field_types.h"
#include "dht22.h"
#include "dht22_ui.h"

namespace DHT22UIFields {

class Temperature : public Field<float> {
protected:
  std::shared_ptr<DHT22> dht22;
  float last;
  bool last_stale;

public:
  Temperature(std::shared_ptr<DHT22> dht22, int row, int col) :
    Field<float>(UI::statusp, row, col, "Temperature", "", "C"), dht22(dht22), last_stale(true)
  {}
  virtual ~Temperature() {}

  virtual float Get() {
    if (!dht22->ChecksumOK())
      last_stale = true;
    else {
      last_stale = false;
      last = dht22->Temperature();
    }

    return last;
  }

  virtual void Update(bool full=false)
  {
    stale = last_stale;
    colors = last_stale ? STALE_PAIR : -1;
    attributes = last_stale ? A_DIM : -1;
    Field<float>::Update(full);
  }
};

class Humidity : public Field<float> {
protected:
  std::shared_ptr<DHT22> dht22;
  float last;
  bool last_stale;

public:
  Humidity(std::shared_ptr<DHT22> dht22, int row, int col) : Field<float>(UI::statusp, row, col, "Humidity", "", "%"), dht22(dht22), last(0), last_stale(true) {}
  virtual ~Humidity() {}

    virtual float Get() {
    if (!dht22->ChecksumOK())
      last_stale = true;
    else {
      last_stale = false;
      last = dht22->Humidity();
    }

    return last;
  }

  virtual void Update(bool full=false)
  {
    stale = last_stale;
    colors = last_stale ? STALE_PAIR : -1;
    attributes = last_stale ? A_DIM : -1;
    Field<float>::Update(full);
  }
};

class Checksum : public IndicatorField {
protected:
  std::shared_ptr<DHT22> dht22;

public:
  Checksum(std::shared_ptr<DHT22> dht22, int row, int col) : IndicatorField(UI::statusp, row, col, "CRC"), dht22(dht22) {}
  virtual ~Checksum() {}

  virtual bool Get() {
    return dht22->ChecksumOK();
  }
};

class LastReadTime : public Field<double> {
protected:
  std::shared_ptr<DHT22> dht22;

public:
  LastReadTime(std::shared_ptr<DHT22> dht22, int row, int col) : Field<double>(UI::statusp, row, col, "Last read time", "", "ms"), dht22(dht22) {}
  virtual ~LastReadTime() {}

  virtual double Get() {
    double r = dht22->LastReadTime() / 1e6;
    return r > 999999.9 ? std::numeric_limits<double>::infinity() : r;
  }

  void Update(bool full) {
    double val = Get();
    snprintf(tmp, sizeof(tmp), "% 5.1f", val);
    this->value = tmp;
    FieldBase::Update(full);
  };
};

class Tries : public Field<uint64_t> {
protected:
  std::shared_ptr<DHT22> dht22;

public:
  Tries(std::shared_ptr<DHT22> dht22, int row, int col) : Field<uint64_t>(UI::statusp, row, col, "# tries", "", ""), dht22(dht22) {}
  virtual ~Tries() {}

  virtual uint64_t Get() {
    return dht22->Tries();
  }
};

class Reads : public Field<double> {
protected:
  std::shared_ptr<DHT22> dht22;

public:
  Reads(std::shared_ptr<DHT22> dht22, int row, int col) : Field<double>(UI::statusp, row, col, "bad reads", "0.0", "%"), dht22(dht22) {}
  virtual ~Reads() {}

  virtual double Get() {
    return 100.0 * (dht22->BadReads() / (double)dht22->Reads());
  }
};

} // DHT22UIFields

using namespace DHT22UIFields;

#define EMPTY() Add(new Empty(row++, col))

DHT22UI::DHT22UI(std::shared_ptr<DHT22> dht22) : UI()
{
  devices.insert(dht22);

  size_t row = 1, col = 1;

  Add(new TimeField(statusp, row, col));
  Add(new Label(UI::statusp, row++, col + 18, Name()));
  EMPTY();

  Add(new Checksum(dht22, row++, col));
  Add(new LastReadTime(dht22, row++, col));
  Add(new Tries(dht22, row++, col));
  Add(new Reads(dht22, row++, col));
  Add(new Temperature(dht22, row++, col));
  Add(new Humidity(dht22, row++, col));
}

DHT22UI::~DHT22UI() {}

void DHT22UI::Layout() {
  UI::Layout();
}
