/////////////////////////////////////////////////////////////////////////////
// Name:        new_user.cpp
// Purpose:     The handlers for the commands (new user; impl)
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

handlers::callback_t handlers::new_user = [](const message &m) -> message {
  return reply_to<new_user_request, new_user_response>(m,
    [](const new_user_request &req) -> new_user_response {
        db::database().insert(db::user{ .id = -1, .name = req.name, .pass = req.pass });
        return {};
    }
  );
};