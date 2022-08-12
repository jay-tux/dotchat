/////////////////////////////////////////////////////////////////////////////
// Name:        tls_bytestream.hpp
// Purpose:     Bytestream for the TLS connection
// Author:      jay-tux
// Created:     July 12, 2022 6:01 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_CLIENT_TLS_BYTESTREAM_HPP
#define DOTCHAT_CLIENT_TLS_BYTESTREAM_HPP

#include <vector>
#include <cstdint>
#include <string>
#include <array>
#include <bit>
#include <span>
#include <cstring>
#include <stdexcept>

namespace dotchat::tls {
namespace _intl_ {
template<typename T>
concept has_begin_end = requires(T a) {
  { a.begin() } -> std::input_iterator;
  { a.end() } -> std::input_iterator;
};

template<typename T>
concept is_iterable = has_begin_end<T> && requires(T a) {
  requires(std::same_as<decltype(a.begin()), decltype(a.end())>);
};

template <typename T>
concept not_iterable = !is_iterable<T>;
}

struct bytestream {
  using byte = uint8_t;
  template <typename T>
  using raw_t = std::array<byte, sizeof(T)>;

  template <typename T>
  void add(const T &val) {
    raw_t<T> raw = as_raw(val);
    data.reserve(data.size() + raw.size());
    for(const auto &b : raw) data.push_back(b);
  }

  template <_intl_::not_iterable T>
  void extract(T &out) {
    raw_t<T> res;
    for(size_t i = 0; i < res.size(); i++) res[i] = data[offset + i];
    offset += res.size();
    out = std::bit_cast<T>(res);
    if(offset > 100) sanitize();
  }

  template <typename T>
  raw_t<T> as_raw(const T &val) {
    return std::bit_cast<raw_t<T>>(val);
  }

  inline void sanitize() {
    auto end_it = data.begin() + offset;
    data.erase(data.begin(), end_it);
    offset = 0;
  }

  inline size_t read(const std::span<byte> &span) {
    size_t i = 0;
    for(; i < span.size() && i + offset < data.size(); i++) {
      span[i] = data[i + offset];
    }
    offset += i;
    return i;
  }

  inline void write(const std::span<byte> &span) {
    data.reserve(data.size() + span.size());
    for(const auto &v: span) {
      data.push_back(v);
    }
  }

  inline void overwrite(const std::span<byte> &span) {
    data.clear();
    write(span);
  }

  [[nodiscard]] constexpr size_t size() const {
    return data.size() - offset;
  }

  inline const byte *buffer() {
    return data.data();
  }

  inline void cleanse() {
    offset = 0;
    data.clear();
  }

  size_t offset = 0;
  std::vector<byte> data = std::vector<byte>();
};

template <typename T>
bytestream &operator<<(bytestream &sink, const T &val) {
  if constexpr(_intl_::is_iterable<T>) {
    for(const auto &c : val) {
      sink << c;
    }
  }
  else {
    sink.add(val);
  }
  return sink;
}

template <_intl_::not_iterable T>
bytestream &operator>>(bytestream &source, T &out) {
  source.extract(out);
  return source;
}
}

#endif //DOTCHAT_CLIENT_TLS_BYTESTREAM_HPP
