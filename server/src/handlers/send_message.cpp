/////////////////////////////////////////////////////////////////////////////
// Name:        send_message.cpp
// Purpose:     The handlers for the commands (send message; impl)
// Author:      jay-tux
// Created:     August 11, 2022 6:32 PM
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

handlers::callback_t handlers::send_msg = [](const message &m) -> message {
  return reply_to<message_send_request, message_send_response>(m,
        [](const message_send_request &msg) -> message_send_response {
          auto user = check_session_key(msg.token);
          if(!user_can_access(user.id, msg.chan_id))
            throw proto_error("You are not permitted to send messages in that channel.");

          db::message add = {
              .id = -1,
              .sender = user.id,
              .channel = msg.chan_id,
              .content = msg.msg_cnt,
              .when = db::now(),
              .replies_to = std::nullopt
          };
          db::database().insert(add);

          return {};
        }
  );
};