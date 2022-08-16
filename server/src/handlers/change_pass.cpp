/////////////////////////////////////////////////////////////////////////////
// Name:        change_pass.cpp
// Purpose:     The handlers for the commands (change password; impl)
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

handlers::callback_t handlers::change_pass = [](const message &m) -> message {
  return reply_to<change_pass_request, change_pass_response>(m,
    [](const change_pass_request &req) -> change_pass_response {
      auto user = check_session_key(req.token);

      db::database().update(db::user{ .id = user.id, .name = user.name, .pass = req.new_pass });
      return {};
    }
  );
};