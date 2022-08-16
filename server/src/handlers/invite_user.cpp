/////////////////////////////////////////////////////////////////////////////
// Name:        invite_user.cpp
// Purpose:     The handlers for the commands (invite user to channel; impl)
// Author:      jay-tux
// Created:     August 12, 2022 1:26 PM
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

handlers::callback_t handlers::invite_user = [](const message &m) -> message {
  return reply_to<invite_user_request, invite_user_response>(m,
    [](const invite_user_request &req) -> invite_user_response {
        auto user = check_session_key(req.token);
        auto pre_chan = db::database().get_optional<db::channel>(req.chan_id);

        if(!pre_chan.has_value())
          throw proto_error("There is no channel with ID " + std::to_string(req.chan_id) + ".");
        const auto &chan = pre_chan.value();
        if(chan.owner_id != user.id)
          throw proto_error("Only the creator of a channel can add users to that channel.");

        auto pre_other = db::database().get_optional<db::user>(req.uid);
        if(!pre_other.has_value())
          throw proto_error("There is no user with ID " + std::to_string(req.uid) + ".");
        const auto &other = pre_other.value();

        auto already_joined = db::database().get_all<db::channel_member>(
            where(
                c(&db::channel_member::channel) == req.chan_id
                and
                c(&db::channel_member::user) == req.uid)
        );
        if(!already_joined.empty())
          throw proto_error("That user has already joined that channel.");

        db::database().replace(db::channel_member{ .user = other.id, .channel = chan.id });

        return {};
    }
  );
};