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
using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::proto;
using namespace dotchat::proto::requests;
using namespace dotchat::proto::responses;
using namespace dotchat::server;

int gen_key() {
  std::array<bytestream::byte, sizeof(int)> data = {};
  RAND_bytes(data.data(), sizeof(int));
  return std::bit_cast<int>(data);
}

handlers::callback_t handlers::login = [](const message &m) -> message {
  using namespace std::chrono_literals;

  return reply_to<login_request, login_response>(m,
      [](const login_request &l) -> login_response {
        auto res = db::database().get_all<db::user>(where(c(&db::user::name) == l.user));
        if(res.empty()) throw proto_error("User `" + l.user + "` doesn't exist.");
        if(res[0].pass != l.pass) throw proto_error("Password for `" + l.user + "` incorrect.");

        int uid = res[0].id;
        int32_t key = gen_key();
        bool okay = false;

        while(!okay) {
          try {
            check_session_key(key);
            gen_key(); // aka this key already exists
          }
          catch(const proto_error &) {
            okay = true; // aka this key is unique and isn't yet in the DB
          }
        }

        db::database().replace(db::session_key{ key, uid, db::now_plus_uncut(24h) });
        return login_response{ {}, key /* token */ };
      }
  );
};