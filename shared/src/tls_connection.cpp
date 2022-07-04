/////////////////////////////////////////////////////////////////////////////
// Name:        tls_connection.cpp
// Purpose:     Ease-of-use wrapper around SSL (impl)
// Author:      jay-tux
// Created:     July 04, 2022 1:06 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "tls_connection.hpp"
#include "tls_error.hpp"
#include "logger.hpp"
#include "openssl/ssl.h"
#include <sys/socket.h>

using namespace dotchat::server;
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
  log << init << "Sending message of " << buffer.str().size() << " bytes." << endl;
  if(SSL_write(ssl, buffer.str().c_str(), static_cast<int>(buffer.str().size())) < 0)
    throw tls_error("Can't send message `" + buffer.str() + "`");
  buffer.str("");
}

std::stringstream tls_connection::read() {
  log << init << "Attempting to read from TLS socket..." << endl;
  std::stringstream rdbuffer;
  std::array<char, 1024> buf = {};
  auto got = SSL_read(ssl, buf.data(), 1023);
  if(got == 0) connected = false;
  else if(got < 0) throw tls_error("Can't read from SSL/TLS.");
  else {
    buf[got] = '\0';
    std::string data(buf.data());
    rdbuffer.str(data);
    log << init << "Read " << got << " bytes." << endl;
  }
  return rdbuffer;
}

tls_connection::~tls_connection() {
  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(conn_handle);
}