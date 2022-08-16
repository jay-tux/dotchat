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

// CHANNEL MESSAGE REQUEST
channel_msg_request channel_msg_request::from(const message &m) {
  return {
    token_request::from(m),
    require_arg<decltype(chan_id)>("chan_id", m.map()) // channel id
  };
}

message channel_msg_request::to() const {
  return {
      token_request::to_intl(request_commands::channel_msg),
      paired("chan_id", chan_id)
  };
}

// MESSAGE SEND REQUEST
message_send_request message_send_request::from(const message &m) {
  return {
    token_request::from(m),
    require_arg<decltype(chan_id)>("chan_id", m.map()),
    require_arg<decltype(msg_cnt)>("msg_cnt", m.map())
  };
}

message message_send_request::to() const {
  return {
      token_request::to_intl(request_commands::send_msg),
      paired("chan_id", chan_id),
      paired("msg_cnt", msg_cnt)
  };
}

// CHANNEL DETAILS REQUEST
channel_details_request channel_details_request::from(const message &m) {
  return {
    token_request::from(m),
    require_arg<decltype(chan_id)>("chan_id", m.map())
  };
}

message channel_details_request::to() const {
  return {
    token_request::to_intl(request_commands::channel_details),
    paired("chan_id", chan_id)
  };
}

// NEW CHANNEL REQUEST
new_channel_request new_channel_request::from(const message &m) {
  auto base = token_request::from(m);
  auto desc = require_arg<std::string>("desc", m.map());

  return {
    base,
    require_arg<decltype(name)>("name", m.map()),
    desc.empty() ? std::nullopt : decltype(new_channel_request::desc){desc}
  };
}

message new_channel_request::to() const {
  return {
    token_request::to_intl(request_commands::new_channel),
    paired("name", name),
    paired("desc", desc.has_value() ? desc.value() : "")
  };
}

// NEW USER REQUEST
new_user_request new_user_request::from(const message &m) {
  return {
    .name = require_arg<decltype(name)>("name", m.map()),
    .pass = require_arg<decltype(pass)>("pass", m.map())
  };
}

message new_user_request::to() const {
  return message{
    request_commands::new_user,
    paired("name", name),
    paired("pass", pass)
  };
}

// CHANGE PASS REQUEST
change_pass_request change_pass_request::from(const message &m) {
  return {
    token_request::from(m),
    require_arg<decltype(new_pass)>("new_pass", m.map())
  };
}

message change_pass_request::to() const {
  return {
    token_request::to_intl(request_commands::change_pass),
    paired("new_pass", new_pass)
  };
}

// USER DETAILS REQUEST
user_details_request user_details_request::from(const message &m) {
  return {
    token_request::from(m),
    require_arg<decltype(uid)>("uid", m.map())
  };
}

message user_details_request::to() const {
  return {
    token_request::to_intl(request_commands::user_details),
    paired("uid", uid)
  };
}

// INVITE USER REQUEST
invite_user_request invite_user_request::from(const message &m) {
  return {
    token_request::from(m),
    require_arg<decltype(uid)>("uid", m.map()),
    require_arg<decltype(chan_id)>("chan_id", m.map())
  };
}

message invite_user_request::to() const {
  return {
    token_request::to_intl(request_commands::invite_user),
    paired("uid", uid),
    paired("chan_id", chan_id)
  };
}