// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cstring>
#include <vector>

#include "raw_ui.h"

template <>
bool ValueParser<uint8_t>::Parse(const char *v_str, uint8_t &value) {
  return sscanf(v_str, "%02hhx", &value) == 1;
}

template <>
bool ValueParser<uint16_t>::Parse(const char *v_str, uint16_t &value) {
  return sscanf(v_str, "%04hx", &value) == 1;
}

template <>
bool ValueParser<uint32_t>::Parse(const char *v_str, uint32_t &value) {
  return sscanf(v_str, "%08x", &value) == 1;
}

template <>
bool ValueParser<std::vector<uint8_t>>::Parse(const char *v_str, std::vector<uint8_t> &value) {
  value.clear();
  size_t n = strlen(v_str);
  for (size_t i = 0; i < n; i+=2) {
    uint8_t v;
    if (sscanf(v_str+i, "%02hhx", &v) != 1)
      return false;
    value.push_back(v);
  }
  return true;
}

template <typename T>
static void format_basic(const uint8_t& value, std::string &formatted) {
  static std::string fmt = std::string("%0") + std::to_string(2 * sizeof(T)) + "x";
  formatted.resize(2 * sizeof(T) + 1);
  snprintf(const_cast<char*>(formatted.data()), formatted.size(), fmt.c_str(), value);
}

template <>
void ValueFormatter<uint8_t>::Format(const uint8_t& value, std::string &formatted) {
  format_basic<uint8_t>(value, formatted);
}

template <>
void ValueFormatter<uint16_t>::Format(const uint16_t& value, std::string &formatted) {
  format_basic<uint16_t>(value, formatted);
}

template <>
void ValueFormatter<uint32_t>::Format(const uint32_t& value, std::string &formatted) {
  format_basic<uint32_t>(value, formatted);
}

template <>
void ValueFormatter<std::vector<uint8_t>>::Format(const std::vector<uint8_t>& value, std::string &formatted) {
  if (value.empty()) {
    formatted = "empty";
  }
  else {
    formatted.resize(2 * value.size() + 1);
    char *s = const_cast<char*>(formatted.data());
    char *p = s;
    for (const auto &byte : value) {
      snprintf(p, formatted.size() - (p - s), "%02x", byte);
      p += 2;
    }
  }
}