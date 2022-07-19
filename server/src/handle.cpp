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
#include "data.cpp" // BAD PRACTICE
#include "openssl/rand.h"
#include <sstream>
#include <iomanip>

using namespace dotchat;
using namespace dotchat::values;
using namespace dotchat::tls;
using namespace dotchat::server;
using namespace dotchat::proto;
const logger::log_source init { "HANDLER", grey };
const logger::log_source error { "ERROR", red };

using arg_coll = decltype(message::args);
using fptr_t = message (*)(const arg_coll &);

template <typename T>
concept is_arg_type = std::same_as<T, int> || std::same_as<T, std::string> || std::same_as<T, std::vector<int>>;

std::variant<message, std::vector<int>> parse_vec(const std::string &val, const message &err) {
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
std::variant<message, T> require_arg(char key, arg_coll &args) {
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
    if(std::holds_alternative<int>(val)) return std::get<int>(val);
    else return m;
  }
  else if constexpr(std::same_as<T, std::string>) {
    if(std::holds_alternative<std::string>(val)) return std::get<std::string>(val);
    else return m;
  }
  else {
    if(std::holds_alternative<std::string>(val)) {
      return parse_vec(std::get<std::string>(val), m);
    } else {
      return m;
    }
  }
}

std::string gen_key() {
  std::array<bytestream::byte, 256> data = {};
  RAND_bytes(data.data(), 256);
  std::stringstream res;
  res << 7; // TODO REMOVE
  for(int i = 0; i < 256; i++) {
    res << std::hex << (int)(data[i]);
  }
  return res.str();
}

#define or_get(val, out) \
  if(std::holds_alternative<decltype(out)>(val)) \
    (out) = std::get<decltype(out)>(val); \
  else \
    return std::get<message>(val)

message login(arg_coll &args) {
  std::string uname;
  std::string pass;
  auto got = require_arg<std::string>('u', args);
  or_get(got, uname);
  got = require_arg<std::string>('p', args);
  or_get(got, pass);

  if(auto user = find_user(uname); user.has_value()) {
    if(user.value().pass == pass) {
      std::string key = gen_key();
      add_key(user.value().id, key);
      message ok;
      ok.command = command_type::OK;
      ok.set_arg('t', key);
      log << init << "[LOGIN] succeeded. Generated key is " << key << endl;
      return ok;
    } else {
      message err;
      err.command = command_type::ERR;
      err.set_arg('r', "Invalid pass");
      log << init << "[LOGIN] failed. Expected pass " << user.value().pass << ", got " << pass << endl;
      return err;
    }
  }

  message err;
  err.command = command_type::ERR;
  err.set_arg('r', "No such db_user");
  log << init << "[LOGIN] failed. User " << uname << " does not exist." << endl;
  return err;
}

message logout(arg_coll &args) {
  std::string token;
  auto got = require_arg<std::string>('t', args);
  or_get(got, token);

  if(!session_keys.contains(token)) {
    message err;
    err.command = command_type::ERR;
    err.set_arg('r', "Invalid token");
    return err;
  }
  else {
    rm_keys_for(session_keys[token].user);
    message ok;
    ok.command = command_type::OK;
    return ok;
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