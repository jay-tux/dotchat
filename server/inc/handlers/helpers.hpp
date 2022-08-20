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

/**
 * \short Namespace for all code related to the server.
 */
namespace dotchat::server {
/**
 * \short Converts an exception to a protocol message.
 * \param e The exception to convert.
 * \returns An error message for the protocol.
 */
inline proto::message exc_to_message(const std::exception &e) {
  return proto::message("err", std::pair<std::string, std::string>{ "reason", std::string(e.what()) });
}

/**
 * \short Checks the session key.
 * \param key The session key to check.
 * \returns The user associated with the session key.
 * \throws `dotchat::proto::proto_error` if the token is invalid.
 */
inline db::user check_session_key(int key) {
  if(auto tmp = db::database().get_optional<db::session_key>(key); tmp.has_value() && tmp.value().valid_until >= db::now()) {
    return db::database().get_optional<db::user>(tmp.value().user).value();
  }

  throw proto_error("Token `" + std::to_string(key) + "` is invalid or has expired. Please log-in again.");
}

/**
 * \short Checks whether a user can access a certain channel.
 * \param uid The user's ID.
 * \param chan_id The channel's ID.
 * \returns True if the user has access to the channel, otherwise false.
 */
inline bool user_can_access(int uid, int chan_id) {
  return db::database().get_optional<db::channel_member>(uid, chan_id).has_value();
}
}

#endif //DOTCHAT_SERVER_HELPERS_HPP
