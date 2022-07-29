/////////////////////////////////////////////////////////////////////////////
// Name:        handle.cpp
// Purpose:     Abstraction to handle commands/messages
// Author:      jay-tux
// Created:     July 12, 2022 11:04 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "protocol/message_error.hpp"
#include "handle.hpp"
#include "logger.hpp"
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

using arg_coll = decltype(message::args);

template <typename T>
concept is_arg_type = std::same_as<T, int> || std::same_as<T, std::string> || std::same_as<T, std::vector<int>>;

either<message, std::vector<int>> parse_vec(const std::string &val, const message &err) {
  std::vector<int> res;
  std::stringstream buf;
  buf.str(val);
  while(!buf.eof()) {
    if(buf.peek() < '0' || buf.peek() > '9') { return err; }
    int tmp;
    buf >> tmp;
    char c;
    buf >> c;
    if(c != ';') return err;
    res.push_back(tmp);
  }
  return res;
}

template <is_arg_type T>
either<message, T> require_arg(char key, arg_coll &args) {
  if(!args.contains(key)) {
    message m;
    m.command = command_type::ERR;
    m.set_arg('r', "Argument error");
    m.set_arg('s', "Missing");
    m.set_arg('k', std::to_string(key));
    return m;
  }

  message m;
  m.command = command_type::ERR;
  m.set_arg('r', "Argument Error");
  m.set_arg('s', "Data type");
  m.set_arg('k', std::to_string(key));

  const auto &val = args[key];
  if constexpr(std::same_as<T, int>) {
    if(val.holds<int>()) return val.get<int>();
    else return m;
  }
  else if constexpr(std::same_as<T, std::string>) {
    if(val.holds<std::string>()) return val.get<std::string>();
    else return m;
  }
  else {
    if(val.holds<std::string>()) {
      return parse_vec(val.get<std::string>(), m);
    } else {
      return m;
    }
  }
}

int gen_key() {
  std::array<bytestream::byte, sizeof(int)> data = {};
  RAND_bytes(data.data(), sizeof(int));
  return std::bit_cast<int>(data) > 0 ? -std::bit_cast<int>(data) : std::bit_cast<int>(data);
}

#define or_get(val, out) \
  if((val).holds<decltype(out)>()) \
    (out) = (val).get<decltype(out)>(); \
  else \
    return (val).get<message>()

message login(arg_coll &args) {
  std::string uname;
  std::string pass;
  auto got = require_arg<std::string>('u', args);
  or_get(got, uname);
  got = require_arg<std::string>('p', args);
  or_get(got, pass);

  if(auto u = db::database().get_all<db::user>(where(c(&db::user::name) == uname)); u.empty()) {
    return message {
      command_type::ERR,
      std::pair{'r', "No such db_user"}
    };
  }
  else if(u[0].pass == pass) {
    auto key = gen_key();
    using namespace std::chrono_literals;
    db::database().replace(db::session_key{ key, u[0].id, db::now_plus(1h) });
    return message {
      command_type::OK,
      std::pair{'t', key}
    };
  }
  else {
    return message {
      command_type::ERR,
      std::pair{'r', "Invalid password"}
    };
  }
}

std::optional<db::user> check_session_key(int key) {
  if(auto tmp = db::database().get_optional<db::session_key>(key); tmp.has_value() && tmp.value().valid_until >= db::now()) {
    return db::database().get_optional<db::user>(tmp.value().user);
  }

  auto now = db::now();
  log << init << "Failed to match " << key << " (timestamp: " << now << ") with any of:" << endl;
  for(const auto &v: db::database().get_all<db::session_key>()) {
    log << init << "  " << v.key << " (for " << v.user << ";";
    if(v.key == key) log << "; matches";
    log << " valid until " << v.valid_until;
    if(v.valid_until < now) log << "; expired";
    log << ")" << endl;
  }

  return std::nullopt;
}

message logout(arg_coll &args) {
  int token;
  auto got = require_arg<int>('t', args);
  or_get(got, token);

  auto user = check_session_key(token);

  if(user.has_value()) {
    db::database().remove_all<db::session_key>(where(c(&db::session_key::key) = user.value().id));
    return message {
      command_type::OK
    };
  }
  else {
    return message {
      command_type::ERR,
      std::pair{'r', "Invalid token"}
    };
  }
}

message load(arg_coll &args) {
  // TODO
  return {};
}

message load_skipping(arg_coll &args) {
  // TODO
  return {};
}

message load_message(arg_coll &args) {
  // TODO
  return {};
}

message load_channels(arg_coll &args) {
  // TODO
  return {};
}

message load_channel_details(arg_coll &args) {
  // TODO
  return {};
}

message store(arg_coll &args) {
  // TODO
  return {};
}

message invalid_command() {
  message res;
  res.command = command_type::ERR;
  res.set_arg('r', "Invalid command");
  return res;
}

message dotchat::server::handle(bytestream &in) {
  try {
    message m;
    in >> m;

    switch(m.command) {
      case command_type::AUTH: return login(m.args);
      case command_type::EXIT: return logout(m.args);
      case command_type::LOAD: return load(m.args);
      case command_type::LOADO: return load_skipping(m.args);
      case command_type::LOADM: return load_message(m.args);
      case command_type::LOADC: return load_channels(m.args);
      case command_type::LEA: return load_channel_details(m.args);
      case command_type::STORE: return store(m.args);

      default:
        return invalid_command();
    }
  }
  catch(const msg_error &exc) {
    log << init << error << red << "An error occurred:" << endl;
    log << init << error << red << exc.what() << endl;
    message res;
    res.command = proto::command_type::ERR;
    res.set_arg('r', "'Unparsable request'");
    return res;
  }
}