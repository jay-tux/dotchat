/////////////////////////////////////////////////////////////////////////////
// Name:        channel_details.cpp
// Purpose:     The handlers for the commands (channel details; impl)
// Author:      jay-tux
// Created:     August 12, 2022 1:23 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include "handlers/handlers.hpp"
#include "handlers/helpers.hpp"

using namespace sqlite_orm;
using namespace dotchat::server;
using namespace dotchat::proto;
using namespace dotchat::proto::requests;
using namespace dotchat::proto::responses;

handlers::callback_t handlers::channel_details = [](const message &m) -> message {
  return reply_to<channel_details_request, channel_details_response>(m,
      [](const channel_details_request &req) -> channel_details_response {
        auto user = check_session_key(req.token);

        auto members = db::database().select(
            columns(&db::channel_member::user),                   // SELECT user FROM channel_member
            where(c(&db::channel_member::channel) == req.chan_id) // WHERE channel = X
        );

        bool found = false;
        decltype(channel_details_response::members) m_res;
        for(const auto &[id]: members) {
          if(id == user.id) [[unlikely]] found = true;
          m_res.push_back(id);
        }

        if(!found)
          throw proto_error("You can't access that channel.");

        auto channel = db::database().get_optional<db::channel>(req.chan_id);
        if(!channel.has_value())
          throw proto_error("That channel doesn't exist.");

        return {
            {},
            channel.value().id,
            channel.value().name,
            channel.value().owner_id,
            channel.value().desc,
            m_res
        };
      }
  );
};