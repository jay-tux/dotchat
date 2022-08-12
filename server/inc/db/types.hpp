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

namespace dotchat::server::db {
using clock_t = std::chrono::system_clock;
using uncut_clock_t = std::chrono::steady_clock;

inline auto now() {
  return static_cast<uint32_t>(
    std::chrono::time_point_cast<std::chrono::milliseconds>(clock_t::now())
        .time_since_epoch()
        .count()
  );
}

inline auto now_uncut() {
  return uncut_clock_t::now()
          .time_since_epoch()
          .count();
}

template <typename R, typename P>
inline auto now_plus(std::chrono::duration<R, P> dur) {
  return static_cast<uint32_t>(
      (
          std::chrono::time_point_cast<std::chrono::milliseconds>(clock_t::now())
              .time_since_epoch() + dur
      ).count()
  );
}

template <typename R, typename P>
inline auto now_plus_uncut(std::chrono::duration<R, P> dur) {
  return (
      uncut_clock_t::now().time_since_epoch() + dur
  ).count();
}

struct user {
  int id;
  std::string name;
  std::string pass;
};

struct session_key {
  int key;
  int user;
  decltype(now_uncut()) valid_until;
};

struct channel {
  int id;
  std::string name;
  int owner_id;
  std::optional<std::string> desc;
};

struct channel_member {
  int user;
  int channel;
};

struct message {
  int id;
  int sender;
  int channel;
  std::string content;
  decltype(now()) when;
  std::optional<int> replies_to;
};
}

#endif //DOTCHAT_SERVER_TYPES_HPP
