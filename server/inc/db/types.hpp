/////////////////////////////////////////////////////////////////////////////
// Name:        types.hpp
// Purpose:     Data types to be stored in the database
// Author:      jay-tux
// Created:     July 23, 2022 10:16 AM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_TYPES_HPP
#define DOTCHAT_SERVER_TYPES_HPP

#include <string>
#include <chrono>
#include <optional>
#include "protocol/requests.hpp"

/**
 * \short Namespace for all code related to the database.
 */
namespace dotchat::server::db {
/**
 * \short Type alias for the clock type (inherited from `dotchat::proto::clock_t`).
 */
using clock_t = proto::clock_t;
using proto::now;

/**
 * \short Type alias for the clock type used in checking validity of session keys (`std::chrono::steady_clock`).
 */
using uncut_clock_t = std::chrono::steady_clock;

/**
 * \short Returns the current timestamp using the `dotchat::server::db::uncut_clock_t`.
 * \returns The current timestamp.
 */
inline auto now_uncut() {
  return uncut_clock_t::now()
          .time_since_epoch()
          .count();
}

/**
 * \short Returns the current timestamp, with an added duration, using the `dotchat::server::db::uncut_clock_t`.
 * \returns Now plus a certain duration.
 */
template <typename R, typename P>
inline auto now_plus_uncut(std::chrono::duration<R, P> dur) {
  return (
      uncut_clock_t::now().time_since_epoch() + dur
  ).count();
}

/**
 * \short Structure representing a user.
 */
struct user {
  /**
   * \short The user's ID.
   */
  int id;
  /**
   * \short The user's username.
   */
  std::string name;
  /**
   * \short The user's password.
   */
  std::string pass;
};

/**
 * \short Structure representing a session key.
 */
struct session_key {
  /**
   * \short The actual session key.
   */
  int key;
  /**
   * \short The user ID associated with this session key.
   */
  int user;
  /**
   * \short The timestamp of the moment this session key becomes invalid.
   */
  decltype(now_uncut()) valid_until;
};

/**
 * \short Structure representing a channel.
 */
struct channel {
  /**
   * \short The channel's ID.
   */
  int id;
  /**
   * \short The channel's name.
   */
  std::string name;
  /**
   * \short The user ID of the channel's owner.
   */
  int owner_id;
  /**
   * \short The channel's description (optional).
   */
  std::optional<std::string> desc;
};

/**
 * \short Structure representing the member of a channel.
 */
struct channel_member {
  /**
   * \short The user's ID.
   */
  int user;
  /**
   * \short The channel's ID.
   */
  int channel;
};

/**
 * \short Structure representing a message in a channel.
 */
struct message {
  /**
   * \short The message's ID.
   */
  int id;
  /**
   * \short The sender's user ID.
   */
  int sender;
  /**
   * \short The ID of the channel this message was sent in.
   */
  int channel;
  /**
   * \short The message's contents.
   */
  std::string content;
  /**
   * \short Time stamp of when the message was sent.
   */
  decltype(now()) when;
  /**
   * \short The message's ID this message was a reply to (optional).
   */
  std::optional<int> replies_to;
};
}

#endif //DOTCHAT_SERVER_TYPES_HPP
