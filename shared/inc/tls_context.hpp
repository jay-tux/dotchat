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
namespace dotchat::server {
class tls_context;
}

#include <string>
#include "openssl/ssl.h"

namespace dotchat::server {
class tls_context {
public:
  tls_context(const std::string &key_file, const std::string &cert_file);
  tls_context(const tls_context &other);
  tls_context(tls_context &&other) noexcept;

  tls_context &operator=(const tls_context &other);
  tls_context &operator=(tls_context &&other) noexcept;

  [[nodiscard]] inline SSL_CTX *get() const { return internal; }

  ~tls_context();

  const static SSL_METHOD *method;

private:
  SSL_CTX *internal;
  std::string key;
  std::string cert;
};
}

#endif //DOTCHAT_SERVER_TLS_CONTEXT_HPP
