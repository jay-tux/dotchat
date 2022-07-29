/////////////////////////////////////////////////////////////////////////////
// Name:        database.hpp
// Purpose:     Database setup and teardown wrappers
// Author:      jay-tux
// Created:     July 23, 2022 10:20 AM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_DATABASE_HPP
#define DOTCHAT_SERVER_DATABASE_HPP

#include <string>
#include <optional>
#include <type_traits>
#include <filesystem>
#include "types.hpp"
#include "sqlite_orm/sqlite_orm.h"

#ifndef SQLITE_ORM_OPTIONAL_SUPPORTED
namespace sqlite_orm {
template <typename T>
struct type_is_nullable<std::optional<T>> : public std::true_type {
  bool operator()(const std::optional<T> &v) const {
    return v.has_value();
  }
};
}
#endif

// TODO create wrapper struct to always use std::optional/catches & rethrow with stack-aware exception

namespace dotchat::server::db {
namespace _intl_ {
static const char *const path = "db.dotchat.sqlite";

const inline auto col_user_id = sqlite_orm::make_column("id", &user::id, sqlite_orm::autoincrement(),
                                                 sqlite_orm::primary_key());
const inline auto col_user_name = sqlite_orm::make_column("name", &user::name, sqlite_orm::unique());
const inline auto col_user_pass = sqlite_orm::make_column("pass", &user::pass);
const inline auto tbl_user = sqlite_orm::make_table(
    "user",
    col_user_id, col_user_name, col_user_pass
);

const inline auto col_key_key = sqlite_orm::make_column("key", &session_key::key, sqlite_orm::primary_key());
const inline auto col_key_user = sqlite_orm::make_column("user", &session_key::user);
const inline auto col_key_valid = sqlite_orm::make_column("valid_until", &session_key::valid_until);
const inline auto tbl_key = sqlite_orm::make_table(
    "session_key",
    col_key_key, col_key_user, col_key_valid,
    sqlite_orm::foreign_key(&session_key::user).references(&user::id)
);

const inline auto col_chan_id = sqlite_orm::make_column("id", &channel::id, sqlite_orm::primary_key());
const inline auto col_chan_name = sqlite_orm::make_column("name", &channel::name, sqlite_orm::unique());
const inline auto col_chan_owner = sqlite_orm::make_column("owner_id", &channel::owner_id);
const inline auto col_chan_desc = sqlite_orm::make_column("desc", &channel::desc);
const inline auto tbl_chan = sqlite_orm::make_table(
    "channel",
    col_chan_id, col_chan_name, col_chan_owner, col_chan_desc,
    sqlite_orm::foreign_key(&channel::owner_id).references(&user::id)
);

const inline auto col_ch_mem_user = sqlite_orm::make_column("user", &channel_member::user);
const inline auto col_ch_mem_chan = sqlite_orm::make_column("channel", &channel_member::channel);
const inline auto tbl_ch_mem = sqlite_orm::make_table(
    "channel_member",
    col_ch_mem_user, col_ch_mem_chan,
    sqlite_orm::primary_key(&channel_member::user, &channel_member::channel),
    sqlite_orm::foreign_key(&channel_member::user).references(&user::id),
    sqlite_orm::foreign_key(&channel_member::channel).references(&channel::id)
);

const inline auto col_msg_id = sqlite_orm::make_column("id", &message::id, sqlite_orm::primary_key());
const inline auto col_msg_sender = sqlite_orm::make_column("sender", &message::sender);
const inline auto col_msg_channel = sqlite_orm::make_column("channel", &message::channel);
const inline auto col_msg_content = sqlite_orm::make_column("content", &message::content);
const inline auto col_msg_when = sqlite_orm::make_column("when", &message::when);
const inline auto col_msg_reply = sqlite_orm::make_column("replies_to", &message::replies_to);
const inline auto tbl_msg = sqlite_orm::make_table(
    "message",
    col_msg_id, col_msg_sender, col_msg_channel, col_msg_content, col_msg_when, col_msg_reply,
    sqlite_orm::foreign_key(&message::sender).references(&user::id),
    sqlite_orm::foreign_key(&message::channel).references(&channel::id),
    sqlite_orm::foreign_key(&message::replies_to).references(&message::id)
);

struct db {
  inline static auto storage = sqlite_orm::make_storage(
      path,
      tbl_user, tbl_key, tbl_chan, tbl_ch_mem, tbl_msg
  );
  inline static bool init_ran = false;
};

}

inline decltype(_intl_::db::storage) &database() {
  using namespace _intl_;
  decltype(db::storage) &res = db::storage;

  if(!db::init_ran) [[unlikely]] {
    if(!std::filesystem::exists(std::filesystem::path{path})) {
      db::storage.sync_schema();
      int user_id = res.insert(user{-1, "master", "pass"});
      int chan_id = res.insert(channel{-1, "general", user_id, "general main room"});
      res.replace(channel_member{.user = user_id, .channel = chan_id});
    }
    db::init_ran = true;
  }

  return res;
}
}

#endif //DOTCHAT_SERVER_DATABASE_HPP
