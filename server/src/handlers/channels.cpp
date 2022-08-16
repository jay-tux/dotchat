/////////////////////////////////////////////////////////////////////////////
// Name:        channels.cpp
// Purpose:     The handlers for the commands (channel listing; impl)
// Author:      jay-tux
// Created:     August 10, 2022 10:46 PM
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

handlers::callback_t handlers::channel_list = [](const message &m) -> message {
  return reply_to<channel_list_request, channel_list_response>(m,
      [](const channel_list_request &req) -> channel_list_response {
        auto user = check_session_key(req.token);

        auto channel_data = db::database().select(
            columns(&db::channel::id, &db::channel::name),                // SELECT id, name FROM channel
            join<db::channel_member>(                                     // FULL JOIN channel_member
                on(c(&db::channel_member::channel) == &db::channel::id)   // ON channel_member.channel = id
            ),
            where(c(&db::channel_member::user) == user.id)                // WHERE channel_member.user = X
        );

        channel_list_response resp;
        for(const auto &[id, name]: channel_data) {
          channel_list_response::channel_short item{ .id = id, .name = name };
          resp.data.push_back(item);
        }

        return resp;
      }
  );
};