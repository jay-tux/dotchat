/////////////////////////////////////////////////////////////////////////////
// Name:        handle.cpp
// Purpose:     Abstraction to handle commands/messages
// Author:      jay-tux
// Created:     July 12, 2022 11:04 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "handle.hpp"
#include "logger.hpp"
#include "either.hpp"
#include "db/database.hpp"
#include "db/types.hpp"
#include "openssl/rand.h"
#include <sstream>
#include <string>

using namespace dotchat;
using namespace dotchat::values;
using namespace dotchat::tls;
using namespace dotchat::server;
using namespace dotchat::proto;
using namespace sqlite_orm;
const logger::log_source init { "HANDLER", grey };
const logger::log_source error { "ERROR", red };

using arg_coll = const message::arg_obj;

int gen_key() {
  std::array<bytestream::byte, sizeof(int)> data = {};
  RAND_bytes(data.data(), sizeof(int));
  return std::bit_cast<int>(data);
}

struct proto_error : public std::logic_error {
  using logic_error::logic_error;
};

template <typename T>
T require_arg(const std::string &key, const message::arg_obj &source) {
  if (!source.contains(key)) {
    throw proto_error("Key `" + key + "` not present.");
  }

  if (source.type(key) != proto::_intl_::matching_enum<T>::val) {
    throw proto_error("Key `" + key + "` doesn't have the correct type.");
  }

  return source[key].get<proto::_intl_::matching_enum<T>::val>();
}

message exc_to_message(const std::exception &e) {
  return message("err", std::pair<std::string, std::string>{ "reason", std::string(e.what()) });
}

/*
 *  --- LOGIN MESSAGE ---
 *  Command: login
 *  Arguments:
 *   - name: string ~ username
 *   - pass: string ~ password
 */

message login(const arg_coll &args) {
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
}

void dump_session_keys(int key) {
  auto now = db::now();
  for(const auto &v: db::database().get_all<db::session_key>()) {
    log << init << "  " << v.key << " (for " << v.user << ";";
    if(v.key == key) log << "; matches";
    log << " valid until " << v.valid_until;
    if(v.valid_until < now) log << "; expired";
    log << ")" << endl;
  }
}

db::user check_session_key(int key) {
  if(auto tmp = db::database().get_optional<db::session_key>(key); tmp.has_value() && tmp.value().valid_until >= db::now()) {
    return db::database().get_optional<db::user>(tmp.value().user).value();
  }

  auto now = db::now();
  log << init << "Failed to match " << key << " (timestamp: " << now << ") with any of:" << endl;
  dump_session_keys(key);

  throw proto_error("Token `" + std::to_string(key) + "` is invalid or has expired. Please log-in again.");
}

/*
 *  --- LOGOUT MESSAGE ---
 *  Command: logout
 *  Arguments:
 *   - token: int32 ~ token
 */

message logout(const arg_coll &args) {
  auto token = require_arg<int32_t>("token", args);
  auto user = check_session_key(token);

  db::database().remove_all<db::session_key>(sqlite_orm::where(c(&db::session_key::user) == user.id));
  dump_session_keys(0);

  return message("ok");
}

message invalid_command(const std::string &cmnd) {
  return exc_to_message(proto_error("Command `" + cmnd + "` is invalid."));
}

message dotchat::server::handle(bytestream &in) {
  using callback_t = decltype(&login);
  const static std::map<std::string, callback_t, std::less<>> switcher{
      std::make_pair("login", login),
      std::make_pair("logout", logout)
  };
  message got(in);

  if(switcher.contains(got.get_command())) {
    try {
      return switcher.at(got.get_command())(got.map());
    }
    catch(const proto_error &e) {
      return exc_to_message(e);
    }
  }
  return invalid_command(got.get_command());
}