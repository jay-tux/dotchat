/////////////////////////////////////////////////////////////////////////////
// Name:        tls_bytestream.hpp
// Purpose:     Bytestream for the TLS connection
// Author:      jay-tux
// Created:     July 12, 2022 6:01 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short Bytestream for the TLS connection.
 */

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

/**
 * \short Namespace containing all code related to the dotchat OpenSSL TLS wrappers.
 */
namespace dotchat::tls {
/**
 * \short Namespace for internal helper code.
 */
namespace _intl_ {
/**
 * \short Concept relaying the meaning of having a start point and an end point.
 * \tparam T The type to check.
 *
 * To have a start and and end point, a type needs to support the following (for any `T a`):
 * - The expression `a.begin()` needs to be valid, and `std::input_iterator<decltype(a.begin())>` must be satisfied; and
 * - The expression `a.end()` needs to be valid, and `std::input_iterator<decltype(a.end())>` must be satisfied.
 */
template<typename T>
concept has_begin_end = requires(T a) {
  { a.begin() } -> std::input_iterator;
  { a.end() } -> std::input_iterator;
};

/**
 * \short Concept relaying the meaning of being iterable.
 * \tparam T The type to check.
 *
 * In addition to satisfying `dotchat::tls::_intl_::has_begin_end<T>`, this concept also requires that the expression
 * `a.begin() != a.end()` is well-formed and results in a `bool`, for any `T a`.
 */
template<typename T>
concept is_iterable = has_begin_end<T> && requires(T a) {
  { a.begin() != a.end() } -> std::same_as<bool>;
};

/**
 * \short Concept relaying the meaning of not being iterable.
 * \tparam T The type to check.
 *
 * A type is is not iterable if it doesn't satisfy `docthat::tls::_intl_::is_iterable<T>`; in other words:
 * - it doesn't have a member `T::begin()`; or
 * - `T::begin()` doesn't satisfy `std::input_iterator`; or
 * - it doesn't have a member `T::end()`; or
 * - `T::end()` doesn't satisfy `std::input_iterator`; or
 * - for any `T a`, `a.begin() != a.end()` is not a valid expression; or
 * - for any `T a`, `a.begin() != a.end()` doesn't result in a `bool`.
 */
template <typename T>
concept not_iterable = !is_iterable<T>;
}

/**
 * \short Structure representing a stream of bytes from which objects can be read and to which objects can be written.
 */
struct bytestream {
public:
  /**
   * \short Type alias for the byte type used (`uint8_t`).
   */
  using byte = uint8_t;
  /**
   * \short Type alias for the raw type of a type (`std::array<dotchat::tls::bytestream::byte, sizeof(T)>`).
   * \tparam T The type whose raw type is required.
   *
   * This type is used in `std::bit_cast` to read the raw bytes for a structure.
   */
  template <typename T>
  using raw_t = std::array<byte, sizeof(T)>;

  /**
   * \short Writes a single value to the stream.
   * \tparam T The type of the value to write.
   * \param val The value to write.
   */
  template <typename T>
  void add(const T &val) {
    raw_t<T> raw = as_raw(val);
    data.reserve(data.size() + raw.size());
    for(const auto &b : raw) data.push_back(b);
  }

  /**
   * \short Extracts a single value from the stream.
   * \tparam T The type of the value to extract; this type should satisfy `dotchat::tls::_intl_::not_iterable<T>`.
   * \param out A reference to the variable to extract into.
   *
   * If the buffer has passed over a significant amount of bytes already (100), the stream is sanitized.
   */
  template <_intl_::not_iterable T>
  void extract(T &out) {
    raw_t<T> res;
    for(size_t i = 0; i < res.size(); i++) res[i] = data[offset + i];
    offset += res.size();
    out = std::bit_cast<T>(res);
    if(offset > 100) sanitize();
  }

  /**
   * \short Converts any value to its raw value.
   * \tparam T The type of the value to convert.
   * \param val The value to convert.
   * \returns A byte array (`dotchat::tls::bytestream::raw_t<T>`) which holds the same byte sequence as `val`.
   */
  template <typename T>
  static raw_t<T> as_raw(const T &val) {
    return std::bit_cast<raw_t<T>>(val);
  }

  /**
   * \short Sanitizes the byte stream, resetting its offset and cleaning its buffer.
   */
  inline void sanitize() {
    auto end_it = data.begin() + (decltype(data.begin())::difference_type)offset;
    data.erase(data.begin(), end_it);
    offset = 0;
  }

  /**
   * \short Reads bytes into a buffer until either the given buffer is full, or the end of the stream is reached.
   * \param span The buffer to write to.
   * \returns The amount of bytes read. This value will always be smaller than or equal to `span.size()`.
   */
  inline size_t read(const std::span<byte> &span) {
    size_t i = 0;
    for(; i < span.size() && i + offset < data.size(); i++) {
      span[i] = data[i + offset];
    }
    offset += i;
    return i;
  }

  /**
   * \short Writes all bytes from the given buffer to the stream.
   * \param span The buffer to copy from.
   */
  inline void write(const std::span<byte> &span) {
    data.reserve(data.size() + span.size());
    for(const auto &v: span) {
      data.push_back(v);
    }
  }

  /**
   * \short Clears the buffer, then copies all bytes from the given buffer into the stream.
   * \param span The data to copy.
   */
  inline void overwrite(const std::span<byte> &span) {
    cleanse();
    write(span);
  }

  /**
   * \short Returns the amount of bytes left in the stream.
   * \returns The amount of bytes left to be read in the stream.
   *
   * The returned value might not represent the actual size of the internal buffer, as an offset is used to prevent
   * unnecessary re-allocations.
   */
  [[nodiscard]] constexpr size_t size() const {
    return data.size() - offset;
  }

  /**
   * \short Returns a pointer to the beginning of the internal buffer.
   * \returns A pointer to the beginning of the internal buffer.
   *
   * The returned pointer may not point to the next byte to be read, as an offset is used to prevent unnecessary re-
   * allocations.
   */
  inline const byte *buffer() {
    return data.data();
  }

  /**
   * \short Returns a pointer to the next byte to be read in the internal buffer.
   * \returns A pointer to the next byte to be read in the internal buffer.
   *
   * The returned pointer may not point to the actual begin of the data buffer, as an offset is used to prevent
   * unnecessary re-allocations.
   */
  inline const byte *read_start() {
    return data.data() + offset;
  }

  /**
   * \short Clears all data in the stream buffer.
   */
  inline void cleanse() {
    offset = 0;
    data.clear();
  }

private:
  /**
   * \short The offset into the buffer.
   */
  size_t offset = 0;
  /**
   * \short The actual internal data buffer.
   */
  std::vector<byte> data = std::vector<byte>();
};

/**
 * \short Writes a single value into the stream.
 * \tparam T The type of value to insert.
 * \param sink The stream to write to.
 * \param val The value to write.
 * \returns A reference to the modified stream.
 *
 * If `T` satisfies `dotchat::tls::_intl_::is_iterable<T>`, then each value is inserted separately. Otherwise, the
 * single value is inserted on its own using the `bytestream::add(T)` method.
 */
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

/**
 * \short Inserts a single value into the byte stream.
 * \tparam T The type of the value to insert; it should satisfy `dotchat::tls::_intl_::not_iterable<T>`.
 * \param source The stream to extract from.
 * \param out A reference to the variable to extract into.
 * \returns A reference to the modified stream.
 */
template <_intl_::not_iterable T>
bytestream &operator>>(bytestream &source, T &out) {
  source.extract(out);
  return source;
}
}

#endif //DOTCHAT_CLIENT_TLS_BYTESTREAM_HPP
