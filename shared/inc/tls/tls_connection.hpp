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

/**
 * \short Namespace containing all code related to the dotchat OpenSSL TLS wrappers.
 */
namespace dotchat::tls {
/**
 * Type alias for the byte type used (`uint8_t`).
 */
using byte = bytestream::byte;

/**
 * \short Class representing a TLS connection.
 *
 * TLS connections can't be created explicitly, only by using the `connect` method from a client socket or the `accept`
 * method of a server socket.
 */
class tls_connection {
public:
  /**
   * \short Structure indicating the end of a message.
   *
   * This structure shouldn't be used directly, but
   * `void dotchat::tls::tls_connection::send(dotchat::tls::bytrestream &)` should be called instead.
   */
  struct end_of_msg {};

  /**
   * \short TLS connections can't be copy-initialized.
   */
  tls_connection(const tls_connection &) = delete;
  /**
   * \short Moves all data from the other connection into this one.
   * \param other The TLS connection whose data to move into this one.
   */
  inline tls_connection(tls_connection &&other) noexcept { *this = std::move(other); }

  /**
   * \short TLS connections can't be copy-assigned.
   */
  tls_connection &operator=(const tls_connection &) = delete;
  /**
   * \short Move-assigns this connection from another one.
   * \param other The connection to move from.
   * \returns A reference to this connection.
   *
   * All data (buffer, OpenSSL data, connection handle and state) are swapped between both objects. The other object,
   * which is an rvalue reference, is assumed to be destroyed afterwards.
   */
  inline tls_connection &operator=(tls_connection &&other) noexcept {
    std::swap(buffer, other.buffer);
    std::swap(ssl, other.ssl);
    std::swap(conn_handle, other.conn_handle);
    std::swap(connected, other.connected);
    return *this;
  }

  /**
   * \short Makes the connection sends all the data in its buffer.
   * \param _ This parameter is ignored and is only for overload resolution.
   * \throws `dotchat::tls::tls_error` if writing to the underlying OpenSSL structures failed.
   */
  void operator<<(end_of_msg _);
  /**
   * \short Returns the connection state of this connection.
   * \returns True if this connection is established; otherwise false.
   */
  [[nodiscard]] inline bool is_connected() const { return connected; }

  /**
   * \short Adds a single value to the connection buffer.
   * \tparam T The type of value to add.
   * \param val The value to add.
   * \returns A reference to this connection after modification.
   * \deprecated Use `dotchat::tls::bytestream` instead, then use the `send` member function to send a byte-stream.
   */
  template <typename T>
  [[deprecated("Use byte-streams instead (dotchat::tls::bytestream)")]] tls_connection &operator<<(const T &val) {
    buffer << val;
    return *this;
  }

  /**
   * \short Sends the contents of a byte-stream through the connection to the other end.
   * \param strm The stream to send.
   */
  void send(bytestream &strm);
  /**
   * \short Starts reading from the connection.
   * \returns A new byte-stream which contains the data read.
   * \attention The current implementation only supports reading messages of at most 1024 bytes. This will be changed
   * in the future to allow messages of arbitrary lengths.
   */
  bytestream read();

  /**
   * \short Checks the internal state to determine if the connection is still opened.
   * \returns True if the underlying connection has not been shut down yet, otherwise false.
   */
  [[nodiscard]] inline bool is_open() const {
    return ssl != nullptr && SSL_get_shutdown(ssl) == 0;
  }
  /**
   * \short Checks the internal state to determine if the connection is still opened.
   * \returns True if the underlying connection has not been shut down yet, otherwise false.
   */
  inline explicit operator bool() const { return is_open(); }

  /**
   * \short If the connection is still opened, closes it. Otherwise, this is a no-op.
   */
  void close();

  /**
   * \short Destroys this connection, closing the connection if it's still opened.
   */
  ~tls_connection();

private:
  /**
   * \short Constructs a new connection from a handle, in a certain TLS context.
   * \param ctxt A reference to the context to use.
   * \param conn_handle The connection handle.
   * \throws `dotchat::tls::tls_error` if the connection is server-side and the TLS connection can't be accepted.
   * \throws `dotchat::tls::tls_error` if the TLS handshake can't be completed.
   */
  tls_connection(const tls_context &ctxt, int conn_handle);

  /**
   * The internal buffer to send or read from.
   */
  bytestream buffer = bytestream();
  /**
   * The wrapped OpenSSL connection.
   */
  SSL *ssl = nullptr;
  /**
   * The connection handle.
   */
  int conn_handle = -1;
  /**
   * Whether or not a connection has been established.
   */
  bool connected = false;
  friend tls_server_socket;
  friend tls_client_socket;
};
}

#endif //DOTCHAT_SERVER_TLS_CONNECTION_HPP