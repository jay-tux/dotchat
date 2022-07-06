/////////////////////////////////////////////////////////////////////////////
// Name:        tls_connection.hpp
// Purpose:     Ease-of-use wrapper around SSL
// Author:      jay-tux
// Created:     July 04, 2022 12:57 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_TLS_CONNECTION_HPP
#define DOTCHAT_SERVER_TLS_CONNECTION_HPP

// forward declares
namespace dotchat::server {
class tls_connection;
}

#include "tls_server_socket.hpp"
#include "tls_client_socket.hpp"
#include "tls_context.hpp"
#include "openssl/ssl.h"
#include <sstream>

namespace dotchat::server {
class tls_connection {
public:
  struct end_of_msg {};

  tls_connection(const tls_connection &) = delete;
  tls_connection(tls_connection &&) = delete;

  tls_connection &operator=(const tls_connection &) = delete;
  tls_connection &operator=(tls_connection &&) = delete;

  void operator<<(const end_of_msg);
  inline bool is_connected() const { return connected; }

  template <typename T>
  tls_connection &operator<<(const T &val) {
    buffer << val;
    return *this;
  }

  std::stringstream read();

  ~tls_connection();

private:
  tls_connection(const tls_context &ctxt, int conn_handle);

  std::stringstream buffer = std::stringstream();
  SSL *ssl;
  int conn_handle;
  bool connected = false;
  friend tls_server_socket;
  friend tls_client_socket;
};
}

#endif //DOTCHAT_SERVER_TLS_CONNECTION_HPP