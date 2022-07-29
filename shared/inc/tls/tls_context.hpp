/////////////////////////////////////////////////////////////////////////////
// Name:        tls_context.hpp
// Purpose:     Wrapper around SSL_CTX
// Author:      jay-tux
// Created:     July 04, 2022 12:48 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_TLS_CONTEXT_HPP
#define DOTCHAT_SERVER_TLS_CONTEXT_HPP

// forward declares
namespace dotchat::tls {
class tls_context;
}

#include <string>
#include "openssl/err.h"
#include "openssl/ssl.h"

namespace dotchat::tls {

template <typename Fun>
concept error_dump_callable = requires(Fun &f) {
  { f() } -> std::same_as<void>;
};

template <typename Str>
concept error_dump_streamable = requires(Str &s, const char *m, int i) {
  { s << m } -> std::convertible_to<Str &>;
  { s << i } -> std::convertible_to<Str &>;
};

class tls_context {
public:
  enum class mode { CLIENT, SERVER };
  explicit tls_context(const std::string &cert_file);
  tls_context(const std::string &key_file, const std::string &cert_file);
  tls_context(const tls_context &other);
  tls_context(tls_context &&other) noexcept;

  [[nodiscard]] inline mode get_mode() const { return operation; }

  tls_context &operator=(const tls_context &other);
  tls_context &operator=(tls_context &&other) noexcept;

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

  [[nodiscard]] inline SSL_CTX *get() const { return internal; }

  ~tls_context();

  const static SSL_METHOD *server_method;
  const static SSL_METHOD *client_method;

private:
  SSL_CTX *internal;
  mode operation;
  std::string key;
  std::string cert;
};
}

#endif //DOTCHAT_SERVER_TLS_CONTEXT_HPP
