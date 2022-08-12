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
  using arg_obj_t = proto::message::arg_obj;
  using callback_t = proto::message (*) (const proto::message &);
  using pair_t = std::pair<std::string, callback_t>;
  using cmd_coll = proto::requests::request_commands;

  static callback_t login;
  static callback_t logout;
  static callback_t channels;
  static callback_t channel_msg;
  static callback_t send_msg;

  const static inline std::map<std::string, callback_t, std::less<>> switcher {
      pair_t{ cmd_coll::login, login },
      pair_t{ cmd_coll::logout, logout },
      pair_t{ cmd_coll::channel_list, channels },
      pair_t{ cmd_coll::channel_msg, channel_msg },
      pair_t{ cmd_coll::send_msg, send_msg }
  };
};
}

#endif //DOTCHAT_CLIENT_HANDLERS_HPP
