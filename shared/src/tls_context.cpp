/////////////////////////////////////////////////////////////////////////////
// Name:        tls_context.cpp
// Purpose:     Wrapper around SSL_CTX (impl)
// Author:      jay-tux
// Created:     July 04, 2022 12:55 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "tls_context.hpp"
#include "logger.hpp"
#include "tls_error.hpp"
#include <sstream>

using namespace dotchat::server;
using namespace dotchat::values;

const SSL_METHOD *tls_context::method = TLS_server_method();
const logger::log_source init{"TLS_CTXT", yellow};

SSL_CTX *_setup(const std::string &key, const std::string &cert) {
  auto ptr = SSL_CTX_new(tls_context::method);
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

tls_context::tls_context(const std::string &key_file, const std::string &cert_file) : key{key_file}, cert{cert_file} {
  log << init << "Starting from key file " << key_file << " and certificate " << cert_file << endl;
  internal = _setup(key_file, cert_file);
}

tls_context::tls_context(const tls_context &other) : internal{nullptr} {
  log << init << "Copy-constructing TLS context from other(key: " << other.key << ", cert: " << other.cert << ")"
      << endl;
  *this = other;
}

tls_context::tls_context(tls_context &&other) noexcept : internal{nullptr} {
  log << init << "Move-constructing TLS context from other(key: " << other.key << ", cert: " << other.cert << ")"
      << endl;
  *this = std::move(other);
}

tls_context &tls_context::operator=(const tls_context &other) {
  if (this == &other) return *this;
  log << init << "Copying TLS context..." << endl;
  internal = _setup(other.key, other.cert);
  return *this;
}

tls_context &tls_context::operator=(tls_context &&other) noexcept {
  log << init << "Moving TLS context..." << endl;
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