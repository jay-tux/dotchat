/////////////////////////////////////////////////////////////////////////////
// Name:        data.cpp
// Purpose:     Temporary data storage - will be replaced by SQLite
// Author:      jay-tux
// Created:     July 12, 2022 11:46 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <string>
#include <map>
#include <optional>

namespace helpers {
template <typename C, typename Pred>
void erase_if(C &cont, const Pred &p) {
  for(auto it = cont.begin(); it != cont.end();) {
    if(p(*it)) it = cont.erase(it);
    else ++it;
  }
}
}

struct db_user {
  size_t id;
  std::string username;
  std::string pass;
};

struct db_keys {
  std::string session_key;
  size_t user;
  size_t valid_until;
};

struct db_channel {
  size_t id;
  std::string name;
  size_t owner;
  std::optional<std::string> description;
};

struct db_channel_user {
  size_t user_id;
  size_t channel_id;
};

struct db_message {
  size_t id;
  size_t sender;
  size_t channel;
  std::string content;
  size_t date;
};

std::map<size_t, db_user> users = {
    { 1, { 1, "master", "pass" } }
};

std::map<std::string, db_keys> session_keys = {};

std::map<size_t, db_channel> channels = {
    { 1, { 1, "general", 1, "The main lobby" } }
};

std::map<size_t, db_channel_user> channels_users = {
    { 1, { 1, 1 } }
};

std::map<size_t, db_message> messages = {};

std::optional<db_user> find_user(const std::string_view &name) {
  for(const auto &[key, val] : users) {
    if(val.username == name) return val;
  }
  return std::nullopt;
}

void add_key(size_t userid, const std::string &key) {
  session_keys[key] = { key, userid, 0 }; // TODO: set duration
}

void rm_keys_for(size_t userid) {
  helpers::erase_if(session_keys, [userid](decltype(*(session_keys.begin())) const &val){
    return val.second.user == userid;
  });
}