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
namespace dotchat::tls {
class tls_connection;
}

#include "tls_server_socket.hpp"
#include "tls_client_socket.hpp"
#include "tls_context.hpp"
#include "tls_bytestream.hpp"
#include "openssl/ssl.h"
#include <vector>

namespace dotchat::tls {
using byte = uint8_t;

class tls_connection {
public:
  struct end_of_msg {};

  tls_connection(const tls_connection &) = delete;
  inline tls_connection(tls_connection &&other) noexcept { *this = std::move(other); }

  tls_connection &operator=(const tls_connection &) = delete;
  inline tls_connection &operator=(tls_connection &&other) noexcept {
    std::swap(buffer, other.buffer);
    std::swap(ssl, other.ssl);
    std::swap(conn_handle, other.conn_handle);
    std::swap(connected, other.connected);
    return *this;
  }

  void operator<<(end_of_msg);
  [[nodiscard]] inline bool is_connected() const { return connected; }

  template <typename T>
  tls_connection &operator<<(const T &val) {
    buffer << val;
    return *this;
  }

  bytestream read();
  [[nodiscard]] inline bool is_open() const {
    return ssl != nullptr && SSL_get_shutdown(ssl) == 0;
  }
  inline explicit operator bool() const { return is_open(); }
  void close();

  ~tls_connection();

private:
  tls_connection(const tls_context &ctxt, int conn_handle);

  bytestream buffer = bytestream();
  SSL *ssl = nullptr;
  int conn_handle = -1;
  bool connected = false;
  friend tls_server_socket;
  friend tls_client_socket;
};
}

#endif //DOTCHAT_SERVER_TLS_CONNECTION_HPP