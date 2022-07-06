/////////////////////////////////////////////////////////////////////////////
// Name:        tls_error.hpp
// Purpose:     Wrapper around SSL error functions
// Author:      jay-tux
// Created:     July 04, 2022 1:28 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_TLS_ERROR_HPP
#define DOTCHAT_SERVER_TLS_ERROR_HPP

#include <string>
#include <exception>
#include <utility>
#include <array>
#include "openssl/err.h"
#include "logger.hpp"

namespace dotchat::tls {
struct tls_error : public std::exception {
  inline explicit tls_error(std::string msg) : std::exception(), msg{std::move(msg)} {
    std::array<char, 1024> buf = {};
    while(ERR_peek_error() != 0) {
      ERR_error_string_n(ERR_get_error(), buf.data(), 1024);
      this->msg += std::string("\n\tCaused by ") + std::string(buf.data());
    }
  }

  [[nodiscard]] inline const char * what() const noexcept override {
    return msg.c_str();
  }

  std::string msg;
};
}

#endif //DOTCHAT_SERVER_TLS_ERROR_HPP
