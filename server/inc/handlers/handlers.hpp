/////////////////////////////////////////////////////////////////////////////
// Name:        handlers.hpp
// Purpose:     The handlers for the commands
// Author:      jay-tux
// Created:     August 10, 2022 10:14 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_CLIENT_HANDLERS_HPP
#define DOTCHAT_CLIENT_HANDLERS_HPP

#include "protocol/message.hpp"
#include "protocol/requests.hpp"

namespace dotchat::server {
using dotchat::proto::proto_error;

struct handlers {
  using callback_t = proto::message (*) (const proto::message &);
  using pair_t = std::pair<std::string, callback_t>;
  using cmd_coll = proto::requests::request_commands;

  static callback_t login;
  static callback_t logout;
  static callback_t channel_list;
  static callback_t channel_msg;
  static callback_t send_msg;
  static callback_t channel_details;
  static callback_t new_channel;
  static callback_t new_user;
  static callback_t change_pass;
  static callback_t user_details;
  static callback_t invite_user;

  const static inline std::map<std::string, callback_t, std::less<>> switcher {
#define ADD(command) pair_t{ cmd_coll::command, command }
      ADD(login), ADD(logout), ADD(channel_list), ADD(channel_msg),
      ADD(send_msg), ADD(channel_details), ADD(new_channel),
      ADD(new_user), ADD(change_pass), ADD(user_details), ADD(invite_user)
#undef ADD
  };
};
}

#endif //DOTCHAT_CLIENT_HANDLERS_HPP
