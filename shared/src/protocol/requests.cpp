/////////////////////////////////////////////////////////////////////////////
// Name:        responses.cpp
// Purpose:     Message to request converters (impl)
// Author:      jay-tux
// Created:     August 11, 2022 4:00 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <string>
#include "protocol/message.hpp"
#include "protocol/requests.hpp"
#include "protocol/helpers.hpp"

using namespace dotchat::proto;
using namespace dotchat::proto::requests;
using namespace dotchat::proto::responses;

template <typename T>
std::pair<std::string, T> paired(std::string key, T val) {
  return std::make_pair(key, val);
}

// TOKEN REQUEST
token_request token_request::from(const dotchat::proto::message &m) {
  return {
    .token = require_arg<decltype(token)>("token", m.map())
  };
}

message token_request::to_intl(const message::command &command) const {
  return message(
      command,
      paired("token", token)
  );
}

// LOGIN REQUEST
login_request login_request::from(const dotchat::proto::message &m) {
  return {
    .user = require_arg<decltype(user)>("user", m.map()),
    .pass = require_arg<decltype(pass)>("pass", m.map())
  };
}

message login_request::to() const {
  return message(
      request_commands::login,
      paired("user", user), paired(pass, pass)
  );
}

// LOGOUT REQUEST
logout_request logout_request::from(const dotchat::proto::message &m) {
  return logout_request{ token_request::from(m) };
}

message logout_request::to() const {
  return token_request::to_intl(request_commands::logout);
}

// CHANNEL LIST REQUEST
channel_list_request channel_list_request::from(const dotchat::proto::message &m) {
  return channel_list_request{ token_request::from(m) };
}

message channel_list_request::to() const {
  return token_request::to_intl(request_commands::channel_list);
}