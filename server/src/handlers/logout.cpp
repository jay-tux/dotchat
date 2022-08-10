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
using namespace sqlite_orm;

/*
 *  --- LOGOUT MESSAGE ---
 *  Command: logout
 *  Arguments:
 *   - token: int32 ~ token
 */

handlers::callback_t handlers::logout = [](const arg_obj_t &args) -> message {
  auto token = require_arg<int32_t>("token", args);
  auto user = check_session_key(token);

  db::database().remove_all<db::session_key>(sqlite_orm::where(c(&db::session_key::user) == user.id));
  return message("ok");
};