/////////////////////////////////////////////////////////////////////////////
// Name:        login.cpp
// Purpose:     The handlers for the commands (login; impl)
// Author:      jay-tux
// Created:     August 10, 2022 10:24 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <openssl/rand.h>
#include <chrono>
#include "tls/tls_bytestream.hpp"
#include "handlers/handlers.hpp"
#include "db/database.hpp"
#include "handlers/helpers.hpp"

using namespace sqlite_orm;
using namespace dotchat::tls;
using namespace dotchat::proto;
using namespace dotchat::server;

int gen_key() {
  std::array<bytestream::byte, sizeof(int)> data = {};
  RAND_bytes(data.data(), sizeof(int));
  return std::bit_cast<int>(data);
}

/*
 *  --- LOGIN MESSAGE ---
 *  Command: login
 *  Arguments:
 *   - name: string ~ username
 *   - pass: string ~ password
 */

handlers::callback_t handlers::login = [](const arg_obj_t &args) -> message {
  using namespace std::chrono_literals;

  auto user = require_arg<std::string>("user", args);
  auto pass = require_arg<std::string>("pass", args);
  auto res = db::database().get_all<db::user>(where(c(&db::user::name) == user));
  if(res.empty()) throw proto_error("User `" + user + "` doesn't exist.");
  if(res[0].pass != pass) throw proto_error("Password for `" + pass + "` incorrect.");

  int uid = res[0].id;
  auto key = gen_key();
  db::database().replace(db::session_key{ key, uid, db::now_plus(1h) });
  return message("ok", std::pair<std::string, int32_t>{ "token", key });
};