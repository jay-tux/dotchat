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
namespace dotchat::tls {
class tls_client_socket;
}

#include <stdexcept>
#include <utility>
#include "tls_context.hpp"
#include "tls_connection.hpp"

/**
 * \short Namespace containing all code related to the dotchat OpenSSL TLS wrappers.
 */
namespace dotchat::tls {
/**
 * \short Class representing a client socket in a TLS context.
 */
class tls_client_socket {
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
   * \short Constructs a client socket using a given TLS context.
   * \param ctxt A reference to the context to use.
   * \throws `dotchat::tls::tls_client_socket::socket_error` if a socket can't be created.
   */
  explicit tls_client_socket(tls_context &ctxt);
  /**
   * \short TLS client sockets can't be copy-initialized.
   */
  tls_client_socket(const tls_client_socket &) = delete;
  /**
   * \short TLS client sockets can't be move-initialized.
   */
  tls_client_socket(tls_client_socket &&other) = delete;

  /**
   * \short TLS client sockets can't be copy-assigned.
   */
  tls_client_socket &operator=(const tls_client_socket &) = delete;
  /**
   * \short TLS client sockets can't be move-assigned.
   */
  tls_client_socket &operator=(tls_client_socket &&other) = delete;

  /**
   * \short Attempts to initialize a connection to a TLS server socket.
   * \param ip The IP address of the machine on which the server runs.
   * \param port The port number for the server socket.
   * \returns A new `dotchat::tls::tls_connection` for this connection.
   * \throws `dotchat::tls::tls_client_socket::socket_error` if the IP address can't be parsed.
   * \throws `dotchat::tls::tls_client_socket::socket_error` if the connection can't be established.
   */
  [[nodiscard]] tls_connection connect(const std::string &ip, uint16_t port) const;

  /**
   * \short Destroys the client socket, terminating the connection (if any).
   */
  ~tls_client_socket();

private:
  /**
   * \short A reference to the context in which this socket operates.
   */
  tls_context &ctxt;
  /**
   * \short The socket handle.
   */
  sock_handle handle;
};
}

#endif //DOTCHAT_SERVER_TLS_CLIENT_SOCKET_HPP
