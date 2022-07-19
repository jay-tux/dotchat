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
#include <variant>
#include "../tls/tls_bytestream.hpp"

namespace dotchat::proto {
enum class command_type { AUTH, EXIT, LOAD, LOADO, LOADM, LOADC, LEA, STORE, OK, ERR };

struct message {
  message() = default;

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

  inline void set_arg(char key, int value) { args[key] = value; }
  inline void set_arg(char key, std::string value) { args[key] = value; }
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
    return std::holds_alternative<int>(args[key]);
  }
  [[nodiscard]] inline int get_int(char key) { return std::get<int>(args[key]); }
  [[nodiscard]] inline std::string get_str(char key) { return std::get<std::string>(args[key]); }

  command_type command;
  std::map<char, std::variant<int, std::string>> args;
};
}

#endif //DOTCHAT_CLIENT_MESSAGE_HPP
