/////////////////////////////////////////////////////////////////////////////
// Name:        message.hpp
// Purpose:     Message parser and struct for the protocol
// Author:      jay-tux
// Created:     July 06, 2022 3:14 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_CLIENT_MESSAGE_HPP
#define DOTCHAT_CLIENT_MESSAGE_HPP

#include <map>
#include <string>
#include <concepts>
#include <any>
#include <utility>
#include <stdexcept>
#include "../tls/tls_bytestream.hpp"

namespace dotchat::proto {
namespace _intl_ {
enum class val_types: int8_t {
  INT8 = 0x01, INT16 = 0x02, INT32 = 0x03, UINT8 = 0x11, UINT16 = 0x12, UINT32 = 0x13,
  CHAR = 0x21, STRING = 0x22, SUB_OBJECT = 0x31, LIST = 0x41
};

class arg;
class arg_list;
class arg_obj;

template <val_types V>
struct matching_type {};

template <> struct matching_type<val_types::INT8> { using type = int8_t; };
template <> struct matching_type<val_types::INT16> { using type = int16_t; };
template <> struct matching_type<val_types::INT32> { using type = int32_t; };
template <> struct matching_type<val_types::UINT8> { using type = uint8_t; };
template <> struct matching_type<val_types::UINT16> { using type = uint16_t; };
template <> struct matching_type<val_types::UINT32> { using type = uint32_t; };
template <> struct matching_type<val_types::CHAR> { using type = char; };
template <> struct matching_type<val_types::STRING> { using type = std::string; };
template <> struct matching_type<val_types::SUB_OBJECT> { using type = arg_obj; };
template <> struct matching_type<val_types::LIST> { using type = arg_list; };

template <val_types V>
using matching_type_t = typename matching_type<V>::type;

template <typename T>
struct matching_enum {};
template <> struct matching_enum<int8_t> { const static val_types val = val_types::INT8; };
template <> struct matching_enum<int16_t> { const static val_types val = val_types::INT16; };
template <> struct matching_enum<int32_t> { const static val_types val = val_types::INT32; };
template <> struct matching_enum<uint8_t> { const static val_types val = val_types::UINT8; };
template <> struct matching_enum<uint16_t> { const static val_types val = val_types::UINT16; };
template <> struct matching_enum<uint32_t> { const static val_types val = val_types::UINT32; };
template <> struct matching_enum<char> { const static val_types val = val_types::CHAR; };
template <> struct matching_enum<std::string> { const static val_types val = val_types::STRING; };
template <> struct matching_enum<arg_obj> { const static val_types val = val_types::SUB_OBJECT; };
template <> struct matching_enum<arg_list> { const static val_types val = val_types::LIST; };

template <typename T, typename ... Ts> struct one_of;

template <typename T, typename T1, typename ... Ts>
struct one_of<T, T1, Ts...> {
  const static bool val = std::same_as<T, T1> || one_of<T, Ts...>::val;
};

template <typename T, typename T1>
struct one_of<T, T1> {
  const static bool val = std::same_as<T, T1>;
};

template <typename T>
concept is_trivially_repr = one_of<
    T,
    matching_type_t<val_types::INT8>, matching_type_t<val_types::INT16>,
    matching_type_t<val_types::INT32>, matching_type_t<val_types::UINT8>,
    matching_type_t<val_types::UINT16>, matching_type_t<val_types::UINT32>,
    matching_type_t<val_types::CHAR>, matching_type_t<val_types::STRING>
>::val;

class arg {
public:
  constexpr arg() = default;

  template <is_trivially_repr T>
  explicit inline arg(const T &val) : _type{matching_enum<T>::val}, _content{val} {}
  explicit arg(const arg_list &val);
  explicit arg(const arg_obj &val);

  [[nodiscard]] inline val_types type() const { return _type; }

  template <val_types T>
  inline matching_type_t<T> get() const { return std::any_cast<matching_type_t<T>>(_content); }

  template <is_trivially_repr T>
  arg &operator=(const T &val) {
    _type = matching_enum<T>::val;
    _content.reset();
    _content = val;
    return *this;
  }
  arg &operator=(const arg_list &l);
  arg &operator=(const arg_obj &lo);

  template <is_trivially_repr T>
  explicit operator T() const {
    if(_type == matching_enum<T>::val) return std::any_cast<T>(_content);
    else throw std::bad_any_cast();
  }
  explicit operator arg_list() const;
  explicit operator arg_obj() const;

private:
  val_types _type = val_types::INT8;
  std::any _content = (int8_t)0;
};

class arg_list {
public:
  [[nodiscard]] inline size_t size() const { return _content.size(); }
  [[nodiscard]] inline val_types type() const { return _contained; }
  inline arg &operator[](size_t n) { return _content[n]; }
  inline const arg &operator[](size_t n) const { return _content[n]; }

  template <typename T>
  inline T &get_as(size_t n) {
    if(_contained == matching_enum<T>::val)
      return *std::any_cast<T *>(&_content[n]);
    else
      throw std::bad_any_cast();
  }

  arg_list &get_list(size_t n);
  arg_obj &get_obj(size_t n);

  template <typename T>
  inline T &operator[](size_t n) {
    if constexpr(is_trivially_repr<T>)
      return get_as<T>(n);
    else if constexpr(std::same_as<T, arg_list>)
      return get_list(n);
    else if constexpr(std::same_as<T, arg_obj>)
      return get_obj(n);
    else
      throw std::bad_any_cast();
  }

  template <typename T>
  inline const T &get_as(size_t n) const {
    if(_contained == matching_enum<T>::val)
      return std::any_cast<T>(_content[n]);
    else
      throw std::bad_any_cast();
  }

  [[nodiscard]] const arg_list &get_list(size_t n) const;
  [[nodiscard]] const arg_obj &get_obj(size_t n) const;

  template <typename T>
  inline const T &operator[](size_t n) const {
    if constexpr(is_trivially_repr<T>)
      return get_as<T>(n);
    else if constexpr(std::same_as<T, arg_list>)
      return get_list(n);
    else if constexpr(std::same_as<T, arg_obj>)
      return get_obj(n);
    else
      throw std::bad_any_cast();
  }

  template <is_trivially_repr T>
  inline void push_back(const T &val) {
    if(_content.empty())
      _contained = matching_enum<T>::val;
    if(_contained == matching_enum<T>::val)
      _content.emplace_back(val);
    else
      throw std::bad_any_cast();
  }
  void push_back(const arg_list &val);
  void push_back(const arg_obj &val);

  inline void push_back(const arg &arg) {
    if(_content.empty()) _contained = arg.type();
    if(arg.type() != _contained) throw std::bad_any_cast();
    _content.push_back(arg);
  }

  template <typename T> struct iterable;

  template <typename T>
  struct iterator {
  private:
    iterator(size_t idx, std::vector<arg> &source) : idx{idx}, source{source} {}

    size_t idx;
    std::vector<arg> &source;
  public:
    T &operator*() { return source[idx]; }
    iterator<T> operator++() { return { idx + 1, source }; }
    bool operator!=(const iterator &other) const {
      return &source != &(other.source) || idx != other.idx;
    }

    friend iterable<T>;
  };

  struct const_iterator {
    inline arg operator*() const { return *it; }
    inline const_iterator operator++() { return { ++it }; }
    inline bool operator!=(const const_iterator &other) const {
      return it != other.it;
    }

    std::vector<arg>::const_iterator it;
  };

  template <typename T>
  struct iterable {
  private:
    explicit iterable(std::vector<arg> &source) : source{source} {}

    std::vector<arg> &source;
  public:
    iterator<T> begin() { return { 0, source }; }
    iterator<T> end() { return { source.size(), source }; }

  friend arg_list;
  };

  template <is_trivially_repr T>
  iterable<T> iterable_for() {
    if(_contained == matching_enum<T>::val)
      return arg_list::iterable<arg_list>{ _content };
    else
      throw std::bad_any_cast();
  }
  iterable<arg_list> iterable_for_sublist();
  iterable<arg_obj> iterable_for_sub_obj();

  template <typename T>
  iterable<T> as_iterable() {
    if constexpr(is_trivially_repr<T>)
      return iterable_for<T>();
    else if constexpr(std::same_as<T, arg_list>)
      return iterable_for_sublist();
    else if constexpr(std::same_as<T, arg_obj>)
      return iterable_for_sub_obj();
    else
      throw std::bad_any_cast();
  }

  [[nodiscard]] inline const_iterator begin() const { return { _content.cbegin() }; }
  [[nodiscard]] inline const_iterator end() const { return { _content.cend() }; }

private:
  val_types _contained;
  std::vector<arg> _content;
};

class arg_obj {
public:
  using map_type = std::map<std::string, arg, std::less<>>;

  [[nodiscard]] inline bool contains(const std::string &key) const { return values.contains(key); }
  [[nodiscard]] inline val_types type(const std::string &key) const { return values.at(key).type(); }

  inline arg &operator[](const std::string &key) { return values[key]; }
  inline const arg &operator[](const std::string &key) const { return values.at(key); }

  template <typename T>
  inline T operator[](const std::string &key) { return static_cast<T>(values[key]); }
  template <typename T>
  inline T operator[](const std::string &key) const { return static_cast<T>(values.at(key)); }

  template <typename T>
  inline T as(const std::string &key) const { return (*this).template operator[]<T>(key); }

  template <typename T>
  void set(std::pair<std::string, T> val) {
    arg a0;
    a0 = val.second;
    values[val.first] = a0;
  }

  [[nodiscard]] inline size_t size() const { return values.size(); }

  inline void set(const std::pair<std::string, arg> &val) {
    values[val.first] = val.second;
  }

  struct iterator {
    std::string operator*() const { return it->first; }
    iterator operator++() { return {++it}; }
    bool operator!=(const iterator &other) const {
      return it != other.it;
    }

    map_type::const_iterator it;
  };

  [[nodiscard]] iterator begin() const { return {values.begin()}; }
  [[nodiscard]] iterator end() const { return {values.end()}; }

private:
  map_type values;
};

template <typename T>
concept is_repr = is_trivially_repr<T> || one_of<
    T,
    matching_type_t<val_types::SUB_OBJECT>, matching_type_t<val_types::LIST>
>::val;
}

struct message_error : public std::logic_error {
  using logic_error::logic_error;
};

class message {
public:
  using byte = tls::bytestream::byte;
  using command = std::string;
  using key = std::string;
  using arg = _intl_::arg;
  using arg_obj = _intl_::arg_obj;
  using arg_type = _intl_::val_types;
  using arg_list = _intl_::arg_list;

  explicit message(tls::bytestream &stream);

  message() = default;

  template <_intl_::is_repr ... Ts>
  explicit message(command cmd, std::pair<key, Ts>... values): cmd{std::move(cmd)} {
    (args.set(values), ...);
  }

  template <_intl_::is_repr T1, _intl_::is_repr ... Ts>
  message(const message &other, std::pair<key, T1> mod1, std::pair<key, Ts>... mods) {
    *this = other;
    args.set(mod1);
    (args.set(mods),...);
  }

  inline arg_obj &map() { return args; }
  inline command &get_command() { return cmd; }

  inline const arg_obj &map() const { return args; }
  inline const command &get_command() const { return cmd; }

  inline static byte preferred_major_version() { return 0x00; }
  inline static byte preferred_minor_version() { return 0x01; }

  inline static bool magic_number_match(byte b1, byte b2) {
    return b1 == 0x2e && b2 == 0x43;
  }

  void send_to(tls::bytestream &strm) const;

  inline friend tls::bytestream &operator<<(tls::bytestream &stream, const message &m) {
    m.send_to(stream);
    return stream;
  }

private:
  byte protocol_major = preferred_major_version();
  byte protocol_minor = preferred_minor_version();
  command cmd;
  arg_obj args;
};
}

/*
 * --- MESSAGE FORMAT ---
 * .C hexadecimal (2 bytes; magic number: 0x2E 0x43)
 * protocol_version (2 bytes; major minor)
 * cmd_len (1 byte)
 * cmd (n bytes; indicated by cmd_len)
 * arg_count (1 byte)
 * args.
 */

/*
 *  --- ARGUMENT FORMAT ---
 *  key_len (1 byte)
 *  key (n bytes; indicated by key_len)
 *  val_type (1 byte)
 *  val (integral/float types: byte-per-byte; other types: see below).
 */

/*
 *  --- VALUE FORMAT (STRINGS) ---
 *  val_len (1 byte)
 *  val_cnt (n bytes; indicated by val_len)
 */

/*
 *  --- VALUE FORMAT (LISTS) ---
 *  cnt_type (1 byte)
 *  list_len (4 bytes)
 *  list_values (n bytes; each is same as val in ARGUMENT FORMAT)
 */

/*
 *  --- VALUE FORMAT (SUB-OBJECTS) ---
 *  sub_obj_count (1 byte)
 *  sub_objs (same as args).
 */

#endif //DOTCHAT_CLIENT_MESSAGE_HPP
