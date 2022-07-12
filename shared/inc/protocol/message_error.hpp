/////////////////////////////////////////////////////////////////////////////
// Name:        message_error.hpp
// Purpose:     Exception type for message errors
// Author:      jay-tux
// Created:     July 06, 2022 3:36 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_MESSAGE_ERROR_HPP
#define DOTCHAT_SERVER_MESSAGE_ERROR_HPP

#include <exception>
#include <string>
#include <map>
#include <utility>

namespace dotchat::proto {
struct msg_error : std::exception {
  explicit inline msg_error(std::string r) : std::exception(), r{std::move(r)}, details{decltype(details)()} {}
  inline msg_error(std::string r, std::map<char, std::string> details) : std::exception(), r{std::move(r)}, details{std::move(details)} {}
  [[nodiscard]] inline const char * what() const noexcept override {
    return r.c_str();
  }
  std::string r;
  std::map<char, std::string> details;
};
}

#endif //DOTCHAT_SERVER_MESSAGE_ERROR_HPP
