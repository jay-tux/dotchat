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

  static callback_t login;
  static callback_t logout;
  static callback_t channels;

  const static inline std::map<std::string, callback_t, std::less<>> switcher {
      pair_t{ "login", login },
      pair_t{ "logout", logout },
      pair_t{ "channel_lst", channels }
  };
};
}

#endif //DOTCHAT_CLIENT_HANDLERS_HPP
