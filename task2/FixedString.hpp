#pragma once

#include <string_view>
#include <cstring>

template<std::size_t max_length>
struct FixedString {
  constexpr FixedString(const char *string, size_t length) : length(length) {
    for (std::size_t i = 0; i < length; ++i) {
      data[i] = string[i];
    }
  }

  constexpr ~FixedString() = default;

  constexpr operator std::string_view() const {
    return {data, length};
  }

  char data[max_length]{};
  const std::size_t length;
};

// operator ""_cstr ?

constexpr FixedString<256> operator ""_cstr(const char *c_str, std::size_t length) {
  return {c_str, length};
}
