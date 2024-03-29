/////////////////////////////////////////////////////////////////////////////
// Name:        tls_connection.cpp
// Purpose:     Ease-of-use wrapper around SSL (impl)
// Author:      jay-tux
// Created:     July 04, 2022 1:06 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "tls/tls_connection.hpp"
#include "tls/tls_error.hpp"
#include "openssl/ssl.h"
#include <iostream>
#include <unistd.h>

#if __unix__
#include <cerrno>
#endif

using namespace dotchat;
using namespace dotchat::tls;

void dump_err(const SSL *ssl, int err) {
  auto code = SSL_get_error(ssl, err);
#define X(x) case (x): std::cerr << "Error " << (x) << ": " << #x << std::endl; break;
  switch(code) {
    X(SSL_ERROR_NONE)
    X(SSL_ERROR_ZERO_RETURN)
    X(SSL_ERROR_WANT_READ)
    X(SSL_ERROR_WANT_WRITE)
    X(SSL_ERROR_WANT_CONNECT)
    X(SSL_ERROR_WANT_ACCEPT)
    X(SSL_ERROR_WANT_X509_LOOKUP)
    X(SSL_ERROR_WANT_ASYNC)
    X(SSL_ERROR_WANT_ASYNC_JOB)
    X(SSL_ERROR_WANT_CLIENT_HELLO_CB)
    X(SSL_ERROR_SYSCALL)
    X(SSL_ERROR_SSL)
    default: std::cerr << "Error " << code << ": unknown?" << std::endl; break;
  }

#if __unix__
  if(code == SSL_ERROR_SYSCALL) {
    std::cerr << "  -> inspecting *nix errno..." << std::endl;
    std::cerr << "  -> *nix errno information: " << errno << " ~> " << strerror(errno) << std::endl;
  }
#endif
}

tls_connection::tls_connection(const tls_context &ctxt, int conn_handle) : ssl{SSL_new(ctxt.get())}, conn_handle{conn_handle} {
  SSL_set_fd(ssl, conn_handle);
  if(ctxt.get_mode() == tls_context::mode::SERVER) {
    if (SSL_accept(ssl) < 0) {
      throw tls_error("Can't accept SSL/TLS connection.");
    }
  }
  else {
    // no host name verification. No idea if we need it?
    if(SSL_connect(ssl) <= 0) {
      throw tls_error("Can't connect using SSL/TLS.");
    }
  }
}

void tls_connection::operator<<(const end_of_msg) {
  if(SSL_write(ssl, buffer.buffer(), static_cast<int>(buffer.size())) < 0)
    throw tls_error("Can't send message");
  buffer.cleanse();
}

bytestream tls_connection::read() { // TODO: refactor to allow messages of > 1024 bytes
  std::vector<byte> buf;
  buf.reserve(1024);
  bytestream res;
  if(auto got = SSL_read(ssl, buf.data(), 1023); got <= 0) {
    connected = false;
    dump_err(ssl, got);
    throw tls_error("Can't read from SSL/TLS.");
  }
  else {
    std::span subset(buf.begin(), got);
    res.overwrite(subset);
  }
  return res;
}

void tls_connection::send(bytestream &strm) {
  buffer = std::move(strm);
  (*this) << end_of_msg{};
}

void tls_connection::close() {
  if(ssl != nullptr) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    ::close(conn_handle);
    ssl = nullptr;
  }
}

tls_connection::~tls_connection() {
  if(ssl != nullptr) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    ::close(conn_handle);
    ssl = nullptr;
  }
}