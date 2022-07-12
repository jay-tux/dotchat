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
#include "logger.hpp"
#include "openssl/ssl.h"
#include <sys/socket.h>

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::values;

const logger::log_source init{"TLS_CONN", blue};

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
  log << init << "Sending message of " << buffer.size() << " bytes." << endl;
  if(SSL_write(ssl, buffer.buffer(), static_cast<int>(buffer.size())) < 0)
    throw tls_error("Can't send message `" +
    std::string(static_cast<const char *>(buffer.buffer())) + "`");
  buffer.cleanse();
}

bytestream tls_connection::read() {
  log << init << "Attempting to read from TLS socket..." << endl;
  std::vector<char> buf;
  buf.reserve(1024);
  bytestream res;
  auto got = SSL_read(ssl, buf.data(), 1023);
  if(got == 0) connected = false;
  else if(got < 0) throw tls_error("Can't read from SSL/TLS.");
  else {
    std::span subset(buf.begin(), got);
    res << subset;
    log << init << "Read " << got << " bytes." << endl;
    log << init << "Stream size: " << res.size() << endl;
  }
  return res;
}

tls_connection::~tls_connection() {
  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(conn_handle);
}