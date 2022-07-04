/////////////////////////////////////////////////////////////////////////////
// Name:        tls_server_socket.cpp
// Purpose:     Wrapper around socket + ssl context (impl)
// Author:      jay-tux
// Created:     June 29, 2022 8:15 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "tls_server_socket.hpp"
#include "logger.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace dotchat::server;
using namespace dotchat::values;

const logger::log_source init{"TLS_SOCK", magenta};

tls_server_socket::tls_server_socket(uint16_t port, tls_context &ctxt) : port{port}, ctxt{ctxt} {
  log << init << "Starting socket on port " << port << "..." << endl;
   sockaddr_in addr = {
       .sin_family = AF_INET,
       .sin_port = htons(port),
       .sin_addr = {
           .s_addr = htonl(INADDR_ANY)
       }
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

tls_connection tls_server_socket::accept() {
  sockaddr_in addr = {};
  uint len = sizeof(addr);
  int client = ::accept(handle, (sockaddr *)&addr, &len);
  if(client < 0) {
    throw socket_error("Unable to accept connection.");
  }
  log << init << "Connected to " << inet_ntoa(addr.sin_addr) << "." << endl;
  return { ctxt, client };
}

tls_server_socket::~tls_server_socket() {
  close(handle);
}