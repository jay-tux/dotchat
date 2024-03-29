/////////////////////////////////////////////////////////////////////////////
// Name:        tls_server_socket.cpp
// Purpose:     Wrapper around socket + ssl context (impl)
// Author:      jay-tux
// Created:     June 29, 2022 8:15 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "tls/tls_client_socket.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace dotchat;
using namespace dotchat::tls;

tls_client_socket::tls_client_socket(tls_context &ctxt) : ctxt{ctxt} {
  handle = socket(AF_INET, SOCK_STREAM, 0);
  if(handle < 0) {
    throw socket_error("Can't create socket.");
  }
}

tls_connection tls_client_socket::connect(const std::string &ip, uint16_t port) const {
  sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = {},
      .sin_zero = {}
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