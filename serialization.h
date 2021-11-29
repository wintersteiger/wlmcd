// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _WLCMD_SERIALIZATION_H_
#define _WLCMD_SERIALIZATION_H_

#include <cstdint>
#include <type_traits>

#include "json.hpp"

inline uint8_t hex_char_to_byte(char c)
{
  if ('0' <= c && c <= '9')
    return c - '0';
  else if ('a' <= c && c <= 'f')
    return 10 + (c - 'a');
  else if ('A' <= c && c <= 'F')
    return 10 + (c - 'A');
  return 0x00;
}

inline uint8_t hex_to_byte(char c1, char c2)
{
  return hex_char_to_byte(c1) << 4 | hex_char_to_byte(c2);
}

std::vector<uint8_t> hex_string_to_bytes(const char *s);

std::string bytes_to_hex(const std::vector<uint8_t> &bytes);

template <size_t SZ>
inline std::string bytes_to_hex(const std::array<uint8_t, SZ> &bytes) {
  return bytes_to_hex(std::vector<uint8_t>(bytes.begin(), bytes.end()));
}

std::vector<uint8_t> from_hex(const std::string &data);

template <typename T, typename = void>
struct hex_scalar_serializer
{
    template <typename BasicJsonType, typename U = T,
              typename std::enable_if<!(std::is_integral<U>::value && !std::is_same<U, bool>::value), int>::type = 0>
    static void from_json(const BasicJsonType& j, U& t)
    {
        nlohmann::from_json(j, t);
    }

    template <typename BasicJsonType, typename U = T,
              typename std::enable_if<!(std::is_integral<U>::value && !std::is_same<U, bool>::value), int>::type = 0>
    static void to_json(BasicJsonType& j, const T& t)
    {
        nlohmann::to_json(j, t);
    }

    template <typename BasicJsonType, typename U = T,
              typename std::enable_if<std::is_integral<U>::value && !std::is_same<U, bool>::value, int>::type = 0>
    static void from_json(const BasicJsonType& j, U& t)
    {
      if (!j.is_string())
        throw std::runtime_error("unexpected json structure");

      std::string s;
      nlohmann::from_json(j, s);

      if (s.size() != 2*sizeof(t))
        throw std::runtime_error("unexpected json string length");

      t = 0;
      for (size_t i=0; i < sizeof(t); i += 2) {
        uint8_t b = 0;
        if (sscanf(s.c_str(), "%02hhx", &b) != 1)
          throw std::runtime_error("invalid hex character in json string");
        t = t << 8 | b;
      }
    }

    template <typename BasicJsonType, typename U = T,
              typename std::enable_if<std::is_integral<U>::value && !std::is_same<U, bool>::value, int>::type = 0>
    static void to_json(BasicJsonType& j, const T& t) noexcept
    {
      char tmp[2 * sizeof(t) + 1];
      for (size_t i=0; i < sizeof(t); i++)
        sprintf(&tmp[2*i], "%02X", (t >> (8*(sizeof(t) - i - 1))) & 0xFF);
      nlohmann::to_json(j, tmp);
    }
};

using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, double, std::allocator, hex_scalar_serializer>;

#endif // _WLCMD_SERIALIZATION_H_
