/////////////////////////////////////////////////////////////////////////////
// Name:        database.hpp
// Purpose:     Database setup and teardown wrappers
// Author:      jay-tux
// Created:     July 23, 2022 10:20 AM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short Database setup and teardown wrappers.
 */

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

// TODO clean up using pimpl

/**
 * \short Namespace for all code related to the database.
 */
namespace dotchat::server::db {
/**
 * \short Namespace for internal helper code.
 */
namespace _intl_ {
/**
 * \short Holds the path to the database file.
 */
static const char *const path = "db.dotchat.sqlite";

/**
 * \short Column for the user IDs.
 */
const inline auto col_user_id = sqlite_orm::make_column("id", &user::id, sqlite_orm::autoincrement(),
                                                 sqlite_orm::primary_key());
/**
 * \short Column for the usernames.
 */
const inline auto col_user_name = sqlite_orm::make_column("name", &user::name, sqlite_orm::unique());
/**
 * \short Column for the user passwords.
 */
const inline auto col_user_pass = sqlite_orm::make_column("pass", &user::pass);
/**
 * \short Table for the users.
 */
const inline auto tbl_user = sqlite_orm::make_table(
    "user",
    col_user_id, col_user_name, col_user_pass
);

/**
 * \short Column for the session keys.
 */
const inline auto col_key_key = sqlite_orm::make_column("key", &session_key::key, sqlite_orm::primary_key());
/**
 * \short Column for the user IDs associated with the session keys.
 */
const inline auto col_key_user = sqlite_orm::make_column("user", &session_key::user);
/**
 * \short Column for the validity of the session keys.
 */
const inline auto col_key_valid = sqlite_orm::make_column("valid_until", &session_key::valid_until);
/**
 * \short Table for the session keys.
 */
const inline auto tbl_key = sqlite_orm::make_table(
    "session_key",
    col_key_key, col_key_user, col_key_valid,
    sqlite_orm::foreign_key(&session_key::user).references(&user::id)
);

/**
 * \short Column for the channel IDs.
 */
const inline auto col_chan_id = sqlite_orm::make_column("id", &channel::id, sqlite_orm::primary_key());
/**
 * \short Column for the channel names.
 */
const inline auto col_chan_name = sqlite_orm::make_column("name", &channel::name, sqlite_orm::unique());
/**
 * \short Column for the channel owner IDs.
 */
const inline auto col_chan_owner = sqlite_orm::make_column("owner_id", &channel::owner_id);
/**
 * \short Column for the channel descriptions.
 */
const inline auto col_chan_desc = sqlite_orm::make_column("desc", &channel::desc);
/**
 * \short Table for the channels.
 */
const inline auto tbl_chan = sqlite_orm::make_table(
    "channel",
    col_chan_id, col_chan_name, col_chan_owner, col_chan_desc,
    sqlite_orm::foreign_key(&channel::owner_id).references(&user::id)
);

/**
 * \short Column for the channel members' user IDs.
 */
const inline auto col_ch_mem_user = sqlite_orm::make_column("user", &channel_member::user);
/**
 * \short Column for the channel members' channel IDs.
 */
const inline auto col_ch_mem_chan = sqlite_orm::make_column("channel", &channel_member::channel);
/**
 * \short Table for the channel members.
 */
const inline auto tbl_ch_mem = sqlite_orm::make_table(
    "channel_member",
    col_ch_mem_user, col_ch_mem_chan,
    sqlite_orm::primary_key(&channel_member::user, &channel_member::channel),
    sqlite_orm::foreign_key(&channel_member::user).references(&user::id),
    sqlite_orm::foreign_key(&channel_member::channel).references(&channel::id)
);

/**
 * \short Column for the message IDs.
 */
const inline auto col_msg_id = sqlite_orm::make_column("id", &message::id, sqlite_orm::primary_key());
/**
 * \short Column for the message sender IDs.
 */
const inline auto col_msg_sender = sqlite_orm::make_column("sender", &message::sender);
/**
 * \short Column for the message channel IDs.
 */
const inline auto col_msg_channel = sqlite_orm::make_column("channel", &message::channel);
/**
 * \short Column for the message contents.
 */
const inline auto col_msg_content = sqlite_orm::make_column("content", &message::content);
/**
 * \short Column for the message timestamps.
 */
const inline auto col_msg_when = sqlite_orm::make_column("when", &message::when);
/**
 * \short Column for the message reply IDs.
 */
const inline auto col_msg_reply = sqlite_orm::make_column("replies_to", &message::replies_to);
/**
 * \short Table for the messages.
 */
const inline auto tbl_msg = sqlite_orm::make_table(
    "message",
    col_msg_id, col_msg_sender, col_msg_channel, col_msg_content, col_msg_when, col_msg_reply,
    sqlite_orm::foreign_key(&message::sender).references(&user::id),
    sqlite_orm::foreign_key(&message::channel).references(&channel::id),
    sqlite_orm::foreign_key(&message::replies_to).references(&message::id)
);

/**
 * \short Structure representing the database.
 */
struct db {
  /**
   * \short The actual internal sqlite_orm storage.
   */
  inline static auto storage = sqlite_orm::make_storage(
      path,
      tbl_user, tbl_key, tbl_chan, tbl_ch_mem, tbl_msg
  );
  /**
   * \short Whether or not the initialization has been run already.
   */
  inline static bool init_ran = false;
};
}

/**
 * \short Gets the database; initializing it the first time this method is called.
 * \returns A reference to the database.
 */
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
