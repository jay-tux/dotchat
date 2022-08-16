/////////////////////////////////////////////////////////////////////////////
// Name:        new_channel.cpp
// Purpose:     The handlers for the commands (new channel; impl)
// Author:      jay-tux
// Created:     August 12, 2022 1:24 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "handlers/handlers.hpp"
#include "handlers/helpers.hpp"

using namespace sqlite_orm;
using namespace dotchat::server;
using namespace dotchat::proto;
using namespace dotchat::proto::requests;
using namespace dotchat::proto::responses;

handlers::callback_t handlers::new_channel = [](const message &m) -> message {
  return reply_to<new_channel_request, new_channel_response >(m,
    [](const new_channel_request &req) -> new_channel_response {
      auto user = check_session_key(req.token);

      db::channel created = {
          .id = -1,
          .name = req.name,
          .owner_id = user.id,
          .desc = req.desc
      };

      auto id = db::database().insert(created);
      return {
          {}, id
      };
    }
  );
};