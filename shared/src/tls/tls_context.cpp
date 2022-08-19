/////////////////////////////////////////////////////////////////////////////
// Name:        tls_context.cpp
// Purpose:     Wrapper around SSL_CTX (impl)
// Author:      jay-tux
// Created:     July 04, 2022 12:55 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "tls/tls_context.hpp"
#include "tls/tls_error.hpp"
#include <sstream>

using namespace dotchat;
using namespace dotchat::tls;

const SSL_METHOD *tls_context::server_method = TLS_server_method();
const SSL_METHOD *tls_context::client_method = TLS_client_method();

SSL_CTX *server_setup(const std::string &key, const std::string &cert) {
  auto ptr = SSL_CTX_new(tls_context::server_method);
  if (!ptr) {
    throw tls_error("Failed to create SSL/TLS context.");
  }

  if (SSL_CTX_use_certificate_file(ptr, cert.c_str(), SSL_FILETYPE_PEM) <= 0) {
    throw tls_error("Failed to select certificate.");
  }
  if (SSL_CTX_use_PrivateKey_file(ptr, key.c_str(), SSL_FILETYPE_PEM) <= 0) {
    throw tls_error("Failed to select private key.");
  }
  return ptr;
}

SSL_CTX *client_setup(const std::string &cert) {
  auto ptr = SSL_CTX_new(tls_context::client_method);
  if (!ptr) {
    throw tls_error("Failed to create SSL/TLS context.");
  }
  SSL_CTX_set_verify(ptr, SSL_VERIFY_PEER, nullptr);
  if (SSL_CTX_load_verify_locations(ptr, cert.c_str(), nullptr) <= 0) {
    throw tls_error("Failed to load certificate.");
  }
  return ptr;
}

tls_context::tls_context(const std::string &cert_file) : operation{mode::CLIENT}, cert{cert_file} {
  internal = client_setup(cert_file);
}

tls_context::tls_context(const std::string &key_file, const std::string &cert_file) : operation{mode::SERVER},
                                                                                               key{key_file},
                                                                                               cert{cert_file} {
  internal = server_setup(key_file, cert_file);
}

tls_context::tls_context(const tls_context &other) : internal{nullptr}, operation{other.operation} {
  *this = other;
}

tls_context::tls_context(tls_context &&other) noexcept : internal{nullptr}, operation{other.operation} {
  *this = std::move(other);
}

tls_context &tls_context::operator=(const tls_context &other) {
  if (this == &other) return *this;
  operation = other.operation;
  key = other.key;
  cert = other.cert;
  if(internal != nullptr) SSL_CTX_free(internal);
  internal = (operation == mode::SERVER) ? server_setup(other.key, other.cert) : client_setup(other.cert);
  return *this;
}

tls_context &tls_context::operator=(tls_context &&other) noexcept {
  auto tmp = other.internal;
  other.internal = internal;
  internal = tmp;
  std::swap(key, other.key);
  std::swap(cert, other.cert);
  return *this;
}

tls_context::~tls_context() {
  if(internal != nullptr)
    SSL_CTX_free(internal);
  internal = nullptr;
}