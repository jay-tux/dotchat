/////////////////////////////////////////////////////////////////////////////
// Name:        tls_server_socket.cpp
// Purpose:     Wrapper around socket + ssl context (impl)
// Author:      jay-tux
// Created:     June 29, 2022 8:15 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "tls_client_socket.hpp"
#include "logger.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace dotchat::server;
using namespace dotchat::values;

const logger::log_source init{"TLS_SOCK", magenta};

tls_client_socket::tls_client_socket(tls_context &ctxt) : ctxt{ctxt} {
  log << init << "Creating client socket..." << endl;
  handle = socket(AF_INET, SOCK_STREAM, 0);
  if(handle < 0) {
    throw socket_error("Can't create socket.");
  }
}

tls_connection tls_client_socket::connect(const std::string &ip, uint16_t port) const {
  log << init << "Attempting connection to " << ip << ":" << port << endl;
  sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port)
  };
  if(inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
    throw socket_error("Can't parse IP address...");
  }
  if(::connect(handle, (sockaddr *)&addr, sizeof(addr)) < 0) {
    throw socket_error("Can't connect to server...");
  }
  return {ctxt, handle};
}

tls_client_socket::~tls_client_socket() {
  close(handle);
}