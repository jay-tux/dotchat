/////////////////////////////////////////////////////////////////////////////
// Name:        tls_server_socket.hpp
// Purpose:     Wrapper around socket + ssl context
// Author:      jay-tux
// Created:     June 29, 2022 8:14 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_TLS_CLIENT_SOCKET_HPP
#define DOTCHAT_SERVER_TLS_CLIENT_SOCKET_HPP

// forward declares
namespace dotchat::server {
class tls_client_socket;
}

#include <exception>
#include <utility>
#include "tls_context.hpp"
#include "tls_connection.hpp"

namespace dotchat::server {
class tls_client_socket {
public:
  struct socket_error : std::exception {
    explicit inline socket_error(std::string msg) : std::exception(), msg{std::move(msg)} {}
    [[nodiscard]] inline const char * what() const noexcept override {
      return msg.c_str();
    }
    std::string msg;
  };
  using sock_handle = int;
  explicit tls_client_socket(tls_context &ctxt);
  tls_client_socket(const tls_client_socket &) = delete;
  tls_client_socket(tls_client_socket &&other) = delete;

  tls_client_socket &operator=(const tls_client_socket &) = delete;
  tls_client_socket &operator=(tls_client_socket &&other) = delete;

  [[nodiscard]] tls_connection connect(const std::string &ip, uint16_t port) const;

  ~tls_client_socket();

private:
  tls_context &ctxt;
  sock_handle handle;
};
}

#endif //DOTCHAT_SERVER_TLS_CLIENT_SOCKET_HPP
