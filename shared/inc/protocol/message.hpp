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

#include <string>
#include <map>
#include <sstream>
#include <utility>
#include <variant>
#include "../either.hpp"
#include "../tls/tls_bytestream.hpp"

namespace dotchat::proto {
enum class command_type { AUTH, EXIT, LOAD, LOADO, LOADM, LOADC, LEA, STORE, OK, ERR };

namespace _intl_ {
template <typename T> concept string_like = std::same_as<T, std::string> || std::convertible_to<T, std::string>;
template <typename T> concept int_like = std::same_as<T, int> || std::convertible_to<T, int>;
template <typename T> concept int_vector = std::same_as<T, std::vector<int>>;

template <typename T> struct conv_to_arg {};
template <string_like T> struct conv_to_arg<T> {
  using type = std::string;
  static type conv(const T &arg) { return static_cast<type>(arg); }
};

template <int_like T> struct conv_to_arg<T> {
  using type = int;
  static type conv(const T &arg) { return static_cast<type>(arg); }
};

template <int_vector T> struct conv_to_arg<T> {
  using type = std::vector<T>;
  static type conv(const T &arg) { return static_cast<type>(arg); }
};
}

template <typename T>
concept valid_msg_arg = _intl_::string_like<T> || _intl_::int_like<T> || _intl_::int_vector<T>;

struct message {
  message() = default;

  template <valid_msg_arg ... TArgs>
  explicit message(command_type cmd, std::pair<char, TArgs> ... args) : command{cmd}, args{decltype(this->args)()} {
    (set_arg(std::get<0>(args), _intl_::conv_to_arg<TArgs>::conv(std::get<1>(args))),...);
  }

  using arg_type = either<int, std::string>;

  explicit message(tls::bytestream &source);
  void write_to(std::ostream &target) const;
  [[nodiscard]] std::string as_string() const;

  inline friend std::ostream &operator<<(std::ostream &target, const message &m) {
    m.write_to(target);
    return target;
  }

  inline friend tls::bytestream &operator>>(tls::bytestream &source, message &m) {
    m = message(source);
    return source;
  }

  inline void set_arg(char key, int value) { args[key] = arg_type(value); }
  inline void set_arg(char key, const std::string &value) { args[key] = arg_type(value); }
  inline void set_arg(char key, const std::vector<int> &values) {
    std::stringstream res;
    res << "'";
    if(!values.empty()) {
      res << values[0];
      for(size_t i = 1; i < values.size(); i++) res << ";" << values[i];
    }
    res << "'";
    args[key] = res.str();
  }

  [[nodiscard]] inline bool has_arg(char key) const {
    return args.contains(key);
  }

  inline bool is_int(char key) {
    return args[key].holds<int>();
  }
  [[nodiscard]] inline int get_int(char key) { return args[key].get<int>(); }
  [[nodiscard]] inline std::string get_str(char key) { return args[key].get<std::string>(); }

  command_type command = command_type::OK;
  std::map<char, arg_type> args;
};
}

#endif //DOTCHAT_CLIENT_MESSAGE_HPP
