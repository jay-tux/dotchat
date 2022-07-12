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
#include <variant>
#include "../tls/tls_bytestream.hpp"

namespace dotchat::proto {
enum class command_type { AUTH, EXIT, LOAD, LOADO, LOADM, STORE, OK, ERR };

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

  command_type command;
  std::map<char, std::variant<int, std::string>> args;
};
}

#endif //DOTCHAT_CLIENT_MESSAGE_HPP
