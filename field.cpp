// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cinttypes>
#include <ctime>
#include <cstring>
#include <cassert>

#include <mutex>
#include <curses.h>

#include "field.h"
#include "field_types.h"

char FieldBase::tmp[1024];

void FieldBase::Move(int row, int col)
{
  if (wndw)
    for (size_t i=0; i < Width(); i++)
      mvwprintw(wndw, this->row, this->col + i, " ");
  this->row = row;
  this->col = col;
  Update(true);
}

void FieldBase::Update(bool full)
{
  assert(key_width > 0);

  if (wndw) {
    if (attributes != -1) wattron(wndw, attributes);
    if (full) {
      if (active) wattron(wndw, A_STANDOUT);
      snprintf(tmp, sizeof(tmp), "%%- %ds", key_width);
      mvwprintw(wndw, row, col, tmp, key.c_str());
      if (active) wattroff(wndw, A_STANDOUT);
      mvwprintw(wndw, row, col + key_width, ": ");
    }
    if (colors != -1) wattron(wndw, COLOR_PAIR(colors));
    if (stale && !active) wattron(wndw, A_DIM);
    snprintf(tmp, sizeof(tmp), "%% %ds", value_width);
    mvwprintw(wndw, row, col + key_width + 2, tmp, value.c_str());
    if (stale && !active) wattroff(wndw, A_DIM);
    if (colors != -1) wattroff(wndw, COLOR_PAIR(colors));
    if (full) {
      if (units_width > 0) {
        snprintf(tmp, sizeof(tmp), "%%- %ds", units_width);
        mvwprintw(wndw, row, col + key_width + 2 + value_width + 1, tmp, units.c_str());
      }
    }
    if (attributes != -1) wattroff(wndw, attributes);
  }
}

template<>
void Field<bool>::Update(bool full) {
  bool val = Get();
  this->value = val ? "true" : "false";
  FieldBase::Update(full);
};

template<>
void Field<uint8_t>::Update(bool full) {
  uint8_t val = Get();
  snprintf(tmp, sizeof(tmp), "%7" PRIu8, val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<int8_t>::Update(bool full) {
  int8_t val = Get();
  snprintf(tmp, sizeof(tmp), "%7" PRIi8, val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<uint16_t>::Update(bool full) {
  uint16_t val = Get();
  snprintf(tmp, sizeof(tmp), "%7" PRIu16, val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<int16_t>::Update(bool full) {
  int16_t val = Get();
  snprintf(tmp, sizeof(tmp), "%7" PRIi16, val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<uint32_t>::Update(bool full) {
  uint32_t val = Get();
  snprintf(tmp, sizeof(tmp), "%" PRIu32, val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<int32_t>::Update(bool full) {
  int32_t val = Get();
  snprintf(tmp, sizeof(tmp), "%" PRIi32, val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<uint64_t>::Update(bool full) {
  uint64_t val = Get();
  snprintf(tmp, sizeof(tmp), "%" PRIu64, val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<int64_t>::Update(bool full) {
  int64_t val = Get();
  snprintf(tmp, sizeof(tmp), "% 7" PRId64, val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<float>::Update(bool full) {
  float val = Get();
  snprintf(tmp, sizeof(tmp), "% 5.3f", val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<double>::Update(bool full) {
  double val = Get();
  snprintf(tmp, sizeof(tmp), "% 5.3f", val);
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<std::string>::Update(bool full) {
  this->value = Get();
  FieldBase::Update(full);
};

template<>
void Field<const char *>::Update(bool full) {
  this->value = Get();
  FieldBase::Update(full);
};

template<>
void Field<pair_uint8_t>::Update(bool full) {
  pair_uint8_t v = Get();
  snprintf(tmp, sizeof(tmp), "%03d/%03d", v.first, v.second);
  this->value = tmp;
  FieldBase::Update(full);
};

std::string TimeField::Get() {
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);
  strftime(tmp, sizeof(tmp), "%Y%m%d %H:%M:%S", lt);
  return std::string(tmp);
}

void TimeField::Update(bool full)
{
  if (wndw) {
    std::string tmp = Get();
    if (tmp != value || full) {
      value = tmp;
      mvwprintw(wndw, row, col, "%-18s", value.c_str());
    }
  }
}

template<>
void Field<bytes_t>::Update(bool full) {
  bytes_t bytes = Get();
  char *p = tmp;
  *p = '\0';
  for (auto b : bytes) {
   snprintf(p, 3, "%02x", b);
   p += 2;
   if (p >= tmp + sizeof(tmp))
    break;
  }
  this->value = tmp;
  FieldBase::Update(full);
};

template<>
void Field<opt_double>::Update(bool full) {
  opt_double val = Get();
  if (val.second) {
    snprintf(tmp, sizeof(tmp), "% 5.2f", val.first);
    value = tmp;
  } else
    value = "?";
  FieldBase::Update(full);
};

template<>
void Field<opt_uint64_t>::Update(bool full)
{
  opt_uint64_t val = Get();
  if (val.second) {
    snprintf(tmp, sizeof(tmp), "%" PRIu64, val.first);
    value = tmp;
  } else
    value = "?";
  FieldBase::Update(full);
};

void Label::Update(bool full)
{
  assert(key_width > 0);
  if (wndw && full) {
    snprintf(tmp, sizeof(tmp), "%%- %ds", key_width);
    if (colors != -1) wattron(wndw, COLOR_PAIR(colors));
    if (bold) wattron(wndw, A_BOLD);
    mvwprintw(wndw, row, col, tmp, key.c_str());
    if (bold) wattroff(wndw, A_BOLD);
    if (colors != -1) wattroff(wndw, COLOR_PAIR(colors));
  }
};

void IndicatorField::Set(const char* value) {
  if (value != NULL && strcmp(value, "") != 0)
    Set(strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
}

void IndicatorField::Update(bool full)
{
  if (wndw) {
    colors = Get() ? ENABLED_PAIR : DISABLED_PAIR;
    wattron(wndw, COLOR_PAIR(colors));
    if (active) wattron(wndw, A_STANDOUT);
    mvwprintw(wndw, row, col, "%s", key.c_str());
    if (active) wattroff(wndw, A_STANDOUT);
    wattroff(wndw, COLOR_PAIR(colors));
  }
}

void WarningField::Update(bool full)
{
  if (wndw) {
    colors = Get() ? LOW_PAIR : ENABLED_PAIR;
    if (colors != -1) wattron(wndw, COLOR_PAIR(colors));
    mvwprintw(wndw, row, col, "%s", key.c_str());
    if (colors != -1) wattroff(wndw, COLOR_PAIR(colors));
  }
}

void StringField::Update(bool full)
{
  if (wndw && full) {
    const char *s = Get();
    value = s ? s : "";
    if (colors != -1) wattron(wndw, COLOR_PAIR(colors));
    mvwprintw(wndw, row, col, "%s", value.c_str());
    if (colors != -1) wattroff(wndw, COLOR_PAIR(colors));
  }
}

void DecField::Update(bool full)
{
  assert(key_width > 0);
  if (wndw) {
    char tmp2[256];
    snprintf(tmp, sizeof(tmp), "%%0%dd", key_width);
    snprintf(tmp2, sizeof(tmp2), tmp, f());
    key = tmp2;
    if (active) wattron(wndw, A_STANDOUT);
    snprintf(tmp, sizeof(tmp), "%%- %ds", key_width);
    mvwprintw(wndw, row, col, tmp, key.c_str());
    if (active) wattroff(wndw, A_STANDOUT);
  }
}

void HexField::Update(bool full)
{
  assert(key_width > 0);
  if (wndw) {
    char tmp2[256];
    snprintf(tmp, sizeof(tmp), "%%0%d" PRIx64, key_width);
    snprintf(tmp2, sizeof(tmp2), tmp, f());
    key = tmp2;
    if (active) wattron(wndw, A_STANDOUT);
    snprintf(tmp, sizeof(tmp), "%%- %ds", key_width);
    mvwprintw(wndw, row, col, tmp, key.c_str());
    if (active) wattroff(wndw, A_STANDOUT);
  }
}

void CharField::Update(bool full)
{
  if (wndw) {
    if (colors != -1) wattron(wndw, COLOR_PAIR(colors));
    if (active) wattron(wndw, A_STANDOUT);
    mvwprintw(wndw, row, col, "%c", f());
    if (active) wattroff(wndw, A_STANDOUT);
    if (colors != -1) wattroff(wndw, COLOR_PAIR(colors));
  }
}
