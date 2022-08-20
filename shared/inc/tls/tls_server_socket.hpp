/////////////////////////////////////////////////////////////////////////////
// Name:        tls_server_socket.hpp
// Purpose:     Wrapper around socket + ssl context
// Author:      jay-tux
// Created:     June 29, 2022 8:14 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short Wrapper around socket + ssl context.
 */

#ifndef DOTCHAT_SERVER_TLS_SERVER_SOCKET_HPP
#define DOTCHAT_SERVER_TLS_SERVER_SOCKET_HPP

// forward declares
namespace dotchat::tls {
class tls_server_socket;
}

#include <stdexcept>
#include <optional>
#include <utility>
#include "tls_context.hpp"
#include "tls_connection.hpp"

/**
 * \short Namespace containing all code related to the dotchat OpenSSL TLS wrappers.
 */
namespace dotchat::tls {
/**
 * \short Class representing a server socket in a TLS context.
 */
class tls_server_socket {
public:
  /**
   * \short Structure representing an error when using sockets.
   * \see `std::logic_error`
   */
  struct socket_error : std::logic_error {
    using std::logic_error::logic_error;
  };

  /**
   * \short Type alias for the socket handle type (`int`).
   */
  using sock_handle = int;

  /**
   * \short Constructs a server socket using a given TLS context on a certain port.
   * \param port The port number to connect to.
   * \param ctxt A reference to the context to use.
   * \throws `dotchat::tls::tls_server_socket::socket_error` if a UNIX socket can't be created for the port number.
   * \throws `dotchat::tls::tls_server_socket::socket_error` if the UNIX socket can't be bound to the port.
   * \throws `dotchat::tls::tls_server_socket::socket_error` if it's not possible to listen on the UNIX socket.
   */
  tls_server_socket(uint16_t port, tls_context &ctxt);
  /**
   * \short TLS server sockets can't be copy-initialized.
   */
  tls_server_socket(const tls_server_socket &) = delete;
  /**
   * \short TLS server sockets can't be move-initialized.
   */
  tls_server_socket(tls_server_socket &&) = delete;

  /**
   * \short TLS server sockets can't be copy-assigned.
   */
  tls_server_socket &operator=(const tls_server_socket &) = delete;
  /**
   * \short TLS server sockets can't be move-assigned.
   */
  tls_server_socket &operator=(tls_server_socket &&) = delete;

  /**
   * \short Waits for an incoming connection, and accepts it.
   * \returns A new, accepted TLS connection.
   * \throws `dotchat::tls::tls_server_socket::socket_error` if the connection can't be accepted.
   * \throws `dotchat::tls::tls_error` if the TLS connection can't be accepted.
   * \throws `dotchat::tls::tls_error` if the TLS handshake can't be completed.
   */
  [[nodiscard]] tls_connection accept() const;
  /**
   * \short Waits for a certain amount of time, accepting a connection if one is available.
   * \param millidelay The delay, in milliseconds.
   * \returns A new, accepted TLS connection if one was available. Otherwise, `std::nullopt`.
   * \throws `dotchat::tls::tls_server_socket::socket_error` if the connection can't be accepted.
   * \throws `dotchat::tls::tls_error` if the TLS connection can't be accepted.
   * \throws `dotchat::tls::tls_error` if the TLS handshake can't be completed.
   *
   * Uses polling to check if a connection is available during the duration. If one is, it will be accepted and the
   * method will return early (before the delay has passed). If not, the function waits until the delay has passed, then
   * returns an empty response.
   */
  [[nodiscard]] std::optional<tls_connection> accept_nonblock(int millidelay = 0) const;

  /**
   * \short Destroys the socket, cleaning up any resources.
   */
  ~tls_server_socket();

private:
  /**
   * \short The port number.
   */
  int port;
  /**
   * \short The reference to the used TLS context.
   */
  tls_context &ctxt;
  /**
   * \short The socket handle (file descriptor).
   */
  sock_handle handle;
};
}

#endif //DOTCHAT_SERVER_TLS_SERVER_SOCKET_HPP
