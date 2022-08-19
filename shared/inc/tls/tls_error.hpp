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
#include <stdexcept>
#include <utility>
#include <array>
#include "openssl/err.h"

/**
 * \short Namespace containing all code related to the dotchat OpenSSL TLS wrappers.
 */
namespace dotchat::tls {
/**
 * \short Structure representing an error during TLS operations.
 * \see `std::logic_error`
 */
struct tls_error : public std::logic_error {
  /**
   * \short Constructs a new TLS error; the message is appended by lines stating where things went wrong (using the
   * OpenSSL error queue).
   * \param msg The error message.
   */
  inline explicit tls_error(const std::string &msg) : std::logic_error(msg + openssl_caused_by()) {}

  /**
   * \short Gets a string containing the OpenSSL error queue.
   * \return A string with the error queue, in human-readable format (starting with a newline, all indented by 1 tab).
   */
  inline static std::string openssl_caused_by() {
    std::string res;
    std::array<char, 1024> buf = {};
    while(ERR_peek_error() != 0) {
      ERR_error_string_n(ERR_get_error(), buf.data(), 1024);
      res += std::string("\n\tCaused by ") + std::string(buf.data());
    }
    return res;
  }
};
}

#endif //DOTCHAT_SERVER_TLS_ERROR_HPP
