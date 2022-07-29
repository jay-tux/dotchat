/////////////////////////////////////////////////////////////////////////////
// Name:        tls_server_socket.cpp
// Purpose:     Wrapper around socket + ssl context (impl)
// Author:      jay-tux
// Created:     June 29, 2022 8:15 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "tls/tls_server_socket.hpp"
#include "logger.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::values;

const logger::log_source init{"TLS_SOCK", magenta};

tls_server_socket::tls_server_socket(uint16_t port, tls_context &ctxt) : port{port}, ctxt{ctxt} {
  log << init << "Starting socket on port " << port << "..." << endl;
   sockaddr_in addr = {
       .sin_family = AF_INET,
       .sin_port = htons(port),
       .sin_addr = {
           .s_addr = htonl(INADDR_ANY)
       },
       .sin_zero = {}
   };
   handle = socket(AF_INET, SOCK_STREAM, 0);
   if(handle < 0) {
     throw socket_error("Can't create socket for port " + std::to_string(port) + ".");
   }

   if(bind(handle, (sockaddr *)(&addr), sizeof(addr)) < 0) {
     throw socket_error("Can't bind socket to port " + std::to_string(port) + ".");
   }

   if(listen(handle, 1) < 0) {
     throw socket_error("Unable to listen to socket.");
   }
}

tls_connection tls_server_socket::accept() const {
  sockaddr_in addr = {};
  uint len = sizeof(addr);
  int client = ::accept(handle, (sockaddr *)&addr, &len);
  if(client < 0) {
    throw socket_error("Unable to accept connection.");
  }
  log << init << "Connected to " << inet_ntoa(addr.sin_addr) << "." << endl;
  return { ctxt, client };
}

std::optional<tls_connection> tls_server_socket::accept_nonblock(int millidelay) const {
  pollfd fd = { .fd = handle, .events = POLLIN, .revents = 0 };
  if(auto res = poll(&fd, 1, millidelay); res > 0 && (fd.revents & POLLIN) != 0) return accept();
  return std::nullopt;
}

tls_server_socket::~tls_server_socket() {
  close(handle);
}