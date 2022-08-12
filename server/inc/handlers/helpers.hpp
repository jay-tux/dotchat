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

#include "logger.hpp"

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

inline bool user_can_access(int uid, int chan_id) {
  return db::database().get_optional<db::channel_member>(uid, chan_id).has_value();
}

inline void dump_keys() {
  using namespace dotchat;
  using namespace dotchat::values;
  using namespace std::literals::chrono_literals;

  const static logger::log_source init { "LOGIN", green };
  auto all_keys = db::database().get_all<db::session_key>();
  auto now = db::now();
  log << init << bold << underline << "All keys (timestamp: " << now << "): " << reset << endl;
  for(const auto &found: all_keys) {
    std::chrono::high_resolution_clock::duration diff{found.valid_until - now};
    auto h = std::chrono::duration_cast<std::chrono::hours>(diff).count();
    auto m = std::chrono::duration_cast<std::chrono::minutes>(diff).count() - (h * 60);

    log << init << "Valid key: " << found.key << " for " << found.user
        << "; valid until " << found.valid_until << "; time left: "
        << h << "h" << m << "m" << endl;
  }
  log << endl;
}
}

#endif //DOTCHAT_SERVER_HELPERS_HPP
