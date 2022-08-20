/////////////////////////////////////////////////////////////////////////////
// Name:        tls_context.hpp
// Purpose:     Wrapper around SSL_CTX
// Author:      jay-tux
// Created:     July 04, 2022 12:48 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short Wrapper around SSL_CTX.
 */

#ifndef DOTCHAT_SERVER_TLS_CONTEXT_HPP
#define DOTCHAT_SERVER_TLS_CONTEXT_HPP

// forward declares
namespace dotchat::tls {
class tls_context;
}

#include <string>
#include "openssl/err.h"
#include "openssl/ssl.h"

/**
 * \short Namespace containing all code related to the dotchat OpenSSL TLS wrappers.
 */
namespace dotchat::tls {
/**
 * \short Concept relaying the meaning of a function type which can be used to "prepare" output streams for error
 * messages.
 * \tparam Fun The function type.
 *
 * A function type (lambda, function pointer, ...) `Fun` satisfies `dotchat::tls::error_dump_callable<Fun>` if it has a
 * signature `void f()` (no arguments, no return value).
 */
template <typename Fun>
concept error_dump_callable = requires(Fun &f) {
  { f() } -> std::same_as<void>;
};

/**
 * \short Concept relaying the meaning of a function type which supports streaming all error message parts.
 * \tparam Str The stream type.
 *
 * To support error part streaming, the following should hold for any `Str &s`:
 * - for any `const char *m`, `s << m` should be a valid expression returning a `Str &`; and
 * - for any `int i`, `s << i` should be a valid expression returning a `Str &`.
 */
template <typename Str>
concept error_dump_streamable = requires(Str &s, const char *m, int i) {
  { s << m } -> std::convertible_to<Str &>;
  { s << i } -> std::convertible_to<Str &>;
};

/**
 * \short Class representing the context in which TLS connections are made.
 */
class tls_context {
public:
  /**
   * \short Enumeration containing the two modes for the context.
   */
  enum class mode {
    CLIENT, /*!< \short The context is run in client mode. */
    SERVER  /*!< \short The context is run in server mode. */
  };
  /**
   * \short Opens the given certificate file and uses it to initialize the context in client mode.
   * \param cert_file The certificate file.
   * \throws `dotchat::tls::tls_error` if the context can not be created by OpenSSL.
   * \throws `dotchat::tls::tls_error` if the certificate file can not be read.
   *
   * The certificate should be a PEM file. It may, however, be self-signed.
   * To be able to connect to another instance, the same certificate should be used on the server.
   */
  explicit tls_context(const std::string &cert_file);
  /**
   * \short Opens the given private key and certificate file and uses them to initialize the context in server mode.
   * \param key_file The key file.
   * \param cert_file The certificate file.
   * \throws `dotchat::tls::tls_error` if the context can not be created by OpenSSL.
   * \throws `dotchat::tls::tls_error` if the certificate file can not be read.
   * \throws `dotchat::tls::tls_error` if the private key file can not be read.
   *
   * The key and certificate should be PEM files. The certificate may, however, be self-signed.
   * To be able to connect to another instance, the same certificate should be used on the server.
   */
  tls_context(const std::string &key_file, const std::string &cert_file);
  /**
   * \short Copies all data from another TLS context.
   * \param other The context to copy from.
   *
   * Operation, key (if any) and certificate file are copied, then a new context is created by OpenSSL.
   */
  tls_context(const tls_context &other);
  /**
   * \short Moves all data from another TLS context.
   * \param other The context to move from.
   *
   * All data is stolen from the other context, which is left in an unusable state.
   */
  tls_context(tls_context &&other) noexcept;

  /**
   * \short Gets the current mode of operation.
   * \returns The current mode of operation.
   */
  [[nodiscard]] inline mode get_mode() const { return operation; }

  /**
   * \short Copies all data from another TLS context.
   * \param other The context to copy from.
   *
   * Operation, key (if any) and certificate file are copied, then a new context is created by OpenSSL.
   * If this context was initialized already, it is cleaned up (removed) first.
   */
  tls_context &operator=(const tls_context &other);
  /**
   * \short Moves all data from another TLS context.
   * \param other The context to move from.
   *
   * All data is stolen from the other context, which then holds the data this context used to hold.
   */
  tls_context &operator=(tls_context &&other) noexcept;

  /**
   * \short Dumps the error queue to the given output stream.
   * \tparam Fun The type of function to call before each error is dumped.
   * \tparam Stream The stream type.
   * \param run_before The function to call before each error is dumped.
   * \param out The stream to write to.
   *
   * Iterates over all OpenSSL errors in their error queue, sending the message `no errors in queue` to the stream
   * if no errors are found; otherwise, does the following for each error in the queue:
   * - Calls `run_before()` to prepare;
   * - Writes to the output stream `[<file>::<function>, on line <line no>]: <error message>`, where everything between
   * angle brackets is replaced by the actual values obtained.
   */
  template <error_dump_callable Fun, error_dump_streamable Stream>
  static void dump_error_queue(Fun &&run_before, Stream &out) {
    const char *file;
    int line;
    const char *func;
    const char *data;
    if(ERR_peek_error() == 0) {
      run_before();
      out << "no errors in queue";
    }
    while(ERR_get_error_all(&file, &line, &func, &data, nullptr) != 0) {
      run_before();
      out << "[" << file << "::" << func << ", on line " << line << "]: " << data;
    }
  }

  /**
   * \short Gets a pointer to the internal OpenSSL context.
   * \returns A pointer to the internal OpenSSL context.
   */
  [[nodiscard]] inline SSL_CTX *get() const { return internal; }

  /**
   * \short Closes the context, cleaning up any resources used.
   */
  ~tls_context();

  /**
   * \short Server method for OpenSSL.
   */
  const static SSL_METHOD *server_method;
  /**
   * \short Client method for OpenSSL.
   */
  const static SSL_METHOD *client_method;

private:
  /**
   * \short The pointer to the internal OpenSSL context.
   */
  SSL_CTX *internal;
  /**
   * \short The mode of operation for this context.
   */
  mode operation;
  /**
   * \short The private key file (if any).
   */
  std::string key;
  /**
   * \short The certificate file.
   */
  std::string cert;
};
}

#endif //DOTCHAT_SERVER_TLS_CONTEXT_HPP
