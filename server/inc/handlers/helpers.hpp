/////////////////////////////////////////////////////////////////////////////
// Name:        helpers.hpp
// Purpose:     Helpers for the handlers for the commands
// Author:      jay-tux
// Created:     August 10, 2022 10:32 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_HELPERS_HPP
#define DOTCHAT_SERVER_HELPERS_HPP

#include <string>
#include <exception>
#include "protocol/message.hpp"
#include "handlers/handlers.hpp"
#include "db/types.hpp"
#include "db/database.hpp"
#include "protocol/helpers.hpp"

namespace dotchat::server {
inline static proto::message exc_to_message(const std::exception &e) {
  return proto::message("err", std::pair<std::string, std::string>{ "reason", std::string(e.what()) });
}

inline db::user check_session_key(int key) {
  if(auto tmp = db::database().get_optional<db::session_key>(key); tmp.has_value() && tmp.value().valid_until >= db::now()) {
    return db::database().get_optional<db::user>(tmp.value().user).value();
  }

  throw proto_error("Token `" + std::to_string(key) + "` is invalid or has expired. Please log-in again.");
}
}

#endif //DOTCHAT_SERVER_HELPERS_HPP
