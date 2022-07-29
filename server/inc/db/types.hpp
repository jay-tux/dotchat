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
#include <optional>

namespace dotchat::server::db {
inline auto now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

template <typename R, typename P>
inline auto now_plus(std::chrono::duration<R, P> dur) {
  return (std::chrono::high_resolution_clock::now().time_since_epoch() + dur).count();
}

struct user {
  int id;
  std::string name;
  std::string pass;
};

struct session_key {
  int key;
  int user;
  decltype(now()) valid_until;
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
  int when;
  std::optional<int> replies_to;
};
}

#endif //DOTCHAT_SERVER_TYPES_HPP
