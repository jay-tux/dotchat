/////////////////////////////////////////////////////////////////////////////
// Name:        user_details.cpp
// Purpose:     The handlers for the commands (user details; impl)
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

handlers::callback_t handlers::user_details = [](const message &m) -> message {
  return reply_to<user_details_request, user_details_response>(m,
    [](const user_details_request &req) -> user_details_response {
      check_session_key(req.token); // check session key, we don't need the caller

      auto user = db::database().get_optional<db::user>(req.uid);
      if(!user.has_value())
        throw proto_error("User with ID `" + std::to_string(req.uid) + "` doesn't exist.");

      // Query:
      // SELECT channel FROM channel_member
      // WHERE user == X
      // AND channel IN (
      //    SELECT channel FROM channel_member
      //    WHERE user == Y
      // )

      auto mut_chan = db::database().select(
          &db::channel_member::channel,
          where(
              c(&db::channel_member::user) == req.uid
              and
              in(
                &db::channel_member::channel,
                select(
                    &db::channel_member::channel,
                    where(c(&db::channel_member::user) == user->id)
                )
              )
          )
      );

      decltype(user_details_response::mutual_channels) chan;
      for(const auto &c: mut_chan) chan.push_back(c);

      return {
          {},
          user->id,
          user->name,
          chan
      };
    }
  );
};