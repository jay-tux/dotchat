/////////////////////////////////////////////////////////////////////////////
// Name:        channel_messages.cpp
// Purpose:     The handlers for the commands (channel message list; impl)
// Author:      jay-tux
// Created:     August 11, 2022 6:14 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "tls/tls_bytestream.hpp"
#include "handlers/handlers.hpp"
#include "db/database.hpp"
#include "handlers/helpers.hpp"

using namespace sqlite_orm;
using namespace dotchat::tls;
using namespace dotchat::proto;
using namespace dotchat::proto::requests;
using namespace dotchat::proto::responses;
using namespace dotchat::server;

/*
 *  --- LOGIN MESSAGE ---
 *  Command: channel_msg
 *  Arguments:
 *   - token: int32_t ~ session key
 *   - chan_id: int32_t ~ channel ID
 */

/*
 *  --- LOGIN SUCCESS RESPONSE ---
 *  Command: ok
 *  Arguments:
 *   - token: int32_t ~ session key
 */
handlers::callback_t handlers::channel_msg = [](const message &m) -> message {
  return reply_to<channel_msg_request, channel_msg_response>(m,
      [](const channel_msg_request &req) -> channel_msg_response {
        if(auto user = check_session_key(req.token); !user_can_access(user.id, req.chan_id))
          throw proto_error("You can't access that channel, or that channel doesn't exist.");

        auto res = db::database().get_all<db::message>(
            where(c(&db::message::channel) == req.chan_id), order_by(&db::message::when)
        );

        std::vector<channel_msg_response::message> msgs;
        for(const auto &msg: res) {
          channel_msg_response::message add = {
              .sender = msg.sender, .when = msg.when, .cnt = msg.content
          };
          msgs.push_back(add);
        }

        return { {}, msgs };
      }
  );
};