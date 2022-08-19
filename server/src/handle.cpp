/////////////////////////////////////////////////////////////////////////////
// Name:        handle.cpp
// Purpose:     Abstraction to handle commands/messages
// Author:      jay-tux
// Created:     July 12, 2022 11:04 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "handle.hpp"
#include "handlers/handlers.hpp"
#include "handlers/helpers.hpp"
#include <string>

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::server;
using namespace dotchat::proto;
using namespace sqlite_orm;

message invalid_command(const std::string &cmnd) {
  return exc_to_message(proto_error("Command `" + cmnd + "` is invalid."));
}

message dotchat::server::handle(bytestream &in) {
  message got(in);

  if(handlers::switcher.contains(got.get_command())) {
    try {
      return handlers::switcher.at(got.get_command())(got);
    }
    catch(const proto_error &e) {
      return exc_to_message(e);
    }
  }
  return invalid_command(got.get_command());
}