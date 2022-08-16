/////////////////////////////////////////////////////////////////////////////
// Name:        logout.cpp
// Purpose:     The handlers for the commands (logout; impl)
// Author:      jay-tux
// Created:     August 10, 2022 10:31 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "handlers/handlers.hpp"
#include "handlers/helpers.hpp"

using namespace dotchat::server;
using namespace dotchat::proto;
using namespace dotchat::proto::requests;
using namespace dotchat::proto::responses;
using namespace sqlite_orm;

handlers::callback_t handlers::logout = [](const message &m) -> message {
  return reply_to<logout_request, logout_response>(m,
      [](const logout_request &req) -> logout_response {
        auto user = check_session_key(req.token);
        db::database().remove_all<db::session_key>(
            sqlite_orm::where(c(&db::session_key::user) == user.id)
        );
        return logout_response{};
      }
  );
};