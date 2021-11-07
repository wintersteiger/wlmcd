// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _FIELD_H_
#define _FIELD_H_

#include <cstdint>
#include <string>
#include <mutex>
#include <vector>
#include <functional>

#include <curses.h>


#define HIGH_PAIR 1
#define LOW_PAIR 2
#define ENABLED_PAIR 3
#define DISABLED_PAIR 4
#define HIGHLIGHT_PAIR 5
#define STALE_PAIR 6

#define HL(X) ((X) ? "high" : "low"), "", ((X) ? HIGH_PAIR : LOW_PAIR)
#define ED(X) ((X) ? "enabled" : "disabled"), "", ((X) ? ENABLED_PAIR : DISABLED_PAIR)
#define OF(X) ((X) ? "OK" : "FAILED"), ""
#define ONOFF(X) "", "", ((X) ? ENABLED_PAIR : DISABLED_PAIR)

class FieldBase {
protected:
  WINDOW *wndw;
  int row, col;
  std::string key, value, units;
  int colors;
  static char tmp[1024];
  size_t key_width, value_width, units_width;
  bool active, stale;
  int attributes;

public:
  FieldBase(WINDOW *wndw, int row, int col, const std::string &key, const std::string &value, const std::string &units) :
    wndw(wndw), row(row), col(col), key(key), value(value), units(units), colors(-1),
    key_width(14), value_width(8), units_width(units.size()), active(false), stale(false), attributes(-1) {}
  virtual ~FieldBase() {}

  const std::string& Key() const { return key; }
  const std::string& Value() const { return value; }
  const std::string& Units() const { return units; }

  void Move(int row, int col);
  void Resize(size_t key_width, size_t value_width, size_t units_width) {
    this->key_width = key_width;
    this->value_width = value_width;
    this->units_width = units_width;
  }
  int Row() const { return row; }
  int Col() const { return col; }

  virtual size_t Width() { return key_width + 2 + value_width + 1 + units_width; }
  virtual void Update(bool full=false);
  virtual void Active(bool a) { active = a; }
  virtual bool Activateable() const { return true; }
  virtual bool ReadOnly() const { return true; }
  virtual void Set(const char*) {}
  virtual std::string Describe() const { return "(empty)"; }
  virtual void Flip() {}
  virtual bool Flippable() const { return false; }
  virtual void Right() {}
  virtual void Left() {}
};

template <typename T>
class Field : public FieldBase {
public:
  Field(WINDOW *wndw, int row, int col, const std::string &key, const std::string &value, const std::string &units) :
    FieldBase(wndw, row, col, key, value, units) {}
  virtual ~Field() {}
  virtual T Get() = 0;
  virtual void Update(bool full=false);
  virtual bool Activateable() const { return true; }
};

class TimeField : public Field<std::string> {
public:
  TimeField(WINDOW *w, int row, int col) :
    Field<std::string>(w, row, col, "", "", "") {}
  virtual ~TimeField() {}
  virtual std::string Get();
  virtual void Update(bool full=false);
  virtual bool Activateable() const { return false; }
};

class Empty : public FieldBase {
public:
  Empty(int row, int col) : FieldBase(NULL, row, col, "", "", "") {}
  virtual ~Empty() {}
  virtual size_t Width() { return 0; }
  virtual bool Activateable() const { return false; }
};

class Label : public FieldBase {
protected:
  int bold;
public:
  Label(WINDOW *w, int row, int col, const std::string &key, int bold = 1, int colors = -1)
    : FieldBase(w, row, col, key, "A", std::string("B")), bold(bold) { this->colors = colors; }
  virtual ~Label() {}
  virtual size_t Width() { return key.size(); }
  virtual void Update(bool full=false);
  virtual bool Activateable() const { return false; }
};

class IndicatorField : public Field<bool> {
public:
  IndicatorField(WINDOW *w, int r, int c, const std::string &key) :
    Field<bool>(w, r, c, key, "", "") {}
  virtual ~IndicatorField() {}
  virtual bool Get() = 0;
  virtual void Set(const char* value);
  virtual void Set(bool value) {}
  virtual void Update(bool full=false);
};

class WarningField : public FieldBase {
public:
  WarningField(WINDOW *w, int r, int c, const std::string &key) :
    FieldBase(w, r, c, key, "", "") {}
  virtual ~WarningField() {}
  virtual bool Get() = 0;
  virtual void Update(bool full=false);
};

class StringField : public Field<const char*>
{
public:
  StringField(WINDOW *w, int row, int col) :
    Field<const char*>(w, row, col, "", "", "") {}
  virtual ~StringField() {}
  virtual const char* Get() = 0;
  virtual size_t Width() { return value.size(); }
  virtual void Update(bool full=false);
};

class DecField : public FieldBase {
  std::function<long(void)> f;
public:
  DecField(WINDOW *wndw, int row, int col, size_t width, std::function<long(void)> f) :
    FieldBase(wndw, row, col, "", "", ""), f(f) { key_width = width; }
  virtual ~DecField() {}
  virtual void Update(bool full=false);
  virtual bool Activateable() const { return true; }
};

class HexField : public FieldBase {
  std::function<uint64_t(void)> f;
public:
  HexField(WINDOW *wndw, int row, int col, size_t width, std::function<uint64_t(void)> f) :
    FieldBase(wndw, row, col, "", "", ""), f(f) { key_width = width; }
  virtual ~HexField() {}
  virtual void Update(bool full=false);
  virtual bool Activateable() const { return true; }
};

class CharField : public FieldBase {
  std::function<char(void)> f;
public:
  CharField(WINDOW *wndw, int row, int col, std::function<char(void)> f, int colors = -1) :
    FieldBase(wndw, row, col, "", "", ""), f(f) { key_width = 1; value_width = 0; units_width = 0; this->colors = colors;}
  virtual ~CharField() {}
  virtual size_t Width() { return 1; }
  virtual void Update(bool full=false);
  virtual bool Activateable() const { return true; }
};

#define DOUBLE_FIELD(W, N, S, D, U, G)                                                       \
  class N : public Field<double>                                                             \
  {                                                                                          \
  protected:                                                                                 \
    D &device;                                                                               \
                                                                                             \
  public:                                                                                    \
    N(int row, int col, D &device) : Field<double>(W, row, col, S, "", U), device(device) {} \
    virtual double Get() { return G; }                                                       \
  };

template <typename T>
class LField : public Field<T> {
protected:
  std::function<T(void)> get = nullptr;
  std::function<void(const char*)> set = nullptr;
  std::function<void()> left = nullptr;
  std::function<void()> right = nullptr;

public:
  LField(WINDOW *wndw, int row, int col, int value_width,
            const std::string &key,
            const std::string &units,
            std::function<T(void)> &&get,
            std::function<void(const char*)> &&set = nullptr,
            std::function<void()> &&left = nullptr,
            std::function<void()> &&right = nullptr) :
    Field<T>(wndw, row, col, key, "", units),
    get(get),
    set(set),
    left(left),
    right(right)
  {
    FieldBase::value_width = value_width;
  }
  virtual ~LField() = default;

  virtual bool ReadOnly() const override { return set == nullptr; }
  virtual T Get() override { return get(); };
  virtual void Set(const char* value) override { if (set) set(value); };
  virtual void Left() override { if (left) left(); }
  virtual void Right() override { if (right) right(); }
};

template <int TCOL, int FCOL>
class LIndicator : public LField<bool> {
public:
  LIndicator(WINDOW *wndw, int row, int col,
            const std::string &key,
            std::function<bool(void)> &&get) :
    LField<bool>(wndw, row, col, 0, key, "", std::move(get)) {
      units_width = 0;
    }
  virtual ~LIndicator() {}

  using LField<bool>::Get;

  virtual void Update(bool full) override
  {
    colors = Get() ? TCOL : FCOL;
    if (wndw) {
      if (active) wattron(wndw, A_STANDOUT);
      if (colors != -1) wattron(wndw, COLOR_PAIR(colors));
      mvwprintw(wndw, row, col, "%s", key.c_str());
      if (colors != -1) wattroff(wndw, COLOR_PAIR(colors));
      if (active) wattroff(wndw, A_STANDOUT);
    }
  }
};

typedef LIndicator<ENABLED_PAIR, DISABLED_PAIR> LEnabledIndicator;
typedef LIndicator<LOW_PAIR, ENABLED_PAIR> LWarningIndicator;

class LSwitch : public LIndicator<ENABLED_PAIR, DISABLED_PAIR> {
protected:
  std::function<void(bool)> bset = nullptr;

public:
  LSwitch(WINDOW *wndw, int row, int col,
          const std::string &key,
          std::function<bool(void)> &&get,
          std::function<void(bool)> &&bset);
  virtual ~LSwitch() {}
  using LIndicator::Get;
  virtual void Flip() override { if (bset) bset(!Get()); };
  virtual bool Flippable() const override { return bset != nullptr; }
  virtual bool ReadOnly() const override { return bset == nullptr; }
};

#endif
