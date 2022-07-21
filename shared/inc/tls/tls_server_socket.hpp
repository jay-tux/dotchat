/////////////////////////////////////////////////////////////////////////////
// Name:        tls_server_socket.hpp
// Purpose:     Wrapper around socket + ssl context
// Author:      jay-tux
// Created:     June 29, 2022 8:14 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_TLS_SERVER_SOCKET_HPP
#define DOTCHAT_SERVER_TLS_SERVER_SOCKET_HPP

// forward declares
namespace dotchat::tls {
class tls_server_socket;
}

#include <optional>
#include <utility>
#include "tls_context.hpp"
#include "tls_connection.hpp"

namespace dotchat::tls {
class tls_server_socket {
public:
  struct socket_error : std::exception {
    explicit inline socket_error(std::string msg) : std::exception(), msg{std::move(msg)} {}
    [[nodiscard]] inline const char * what() const noexcept override {
      return msg.c_str();
    }
    std::string msg;
  };
  using sock_handle = int;
  tls_server_socket(uint16_t port, tls_context &ctxt);
  tls_server_socket(const tls_server_socket &) = delete;
  tls_server_socket(tls_server_socket &&other) = delete;

  tls_server_socket &operator=(const tls_server_socket &) = delete;
  tls_server_socket &operator=(tls_server_socket &&other) = delete;

  [[nodiscard]] tls_connection accept() const;
  [[nodiscard]] std::optional<tls_connection> accept_nonblock(int millidelay = 0) const;

  ~tls_server_socket();

private:
  int port;
  tls_context &ctxt;
  sock_handle handle;
};
}

#endif //DOTCHAT_SERVER_TLS_SERVER_SOCKET_HPP
