/////////////////////////////////////////////////////////////////////////////
// Name:        responses.cpp
// Purpose:     Message to response converters (impl)
// Author:      jay-tux
// Created:     August 11, 2022 4:23 PM
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

// OKAY RESPONSE
okay_response okay_response::from(const dotchat::proto::message &m) {
  if(m.get_command() != response_commands::okay)
    throw proto_error("Expected command `" + response_commands::okay + "`, but got `" + m.get_command() + "`");
  return {};
}
message okay_response::to() const {
  return message(response_commands::okay);
}

// ERROR RESPONSE
error_response error_response::from(const dotchat::proto::message &m) {
  return {
    .reason = require_arg<std::string>("reason", m.map())
  };
}

error_response error_response::from(const dotchat::proto::proto_error &err) {
  return {
    .reason = err.what()
  };
}

message error_response::to() const {
  return message(
      response_commands::error,
      paired("reason", reason)
  );
}

// TOKEN RESPONSE
token_response token_response::from(const dotchat::proto::message &m) {
  return token_response{
    okay_response::from(m),
    require_arg<decltype(token)>("token", m.map()) // token
  };
}

message token_response::to() const {
  return {
      (*this).okay_response::to(),
      paired("token", token)
  };
}

// ID RESPONSE
id_response id_response::from(const dotchat::proto::message &m) {
  return {
    okay_response::from(m),
    require_arg<decltype(id)>("id", m.map())
  };
}

message id_response::to() const {
  return {
      (*this).okay_response::to(),
      paired("id", id)
  };
}

// CHANNEL LIST RESPONSE
channel_list_response channel_list_response::from(const dotchat::proto::message &m) {
  auto data = require_arg<message::arg_list>("data", m.map());
  std::vector<channel_short> vec;

  for(const auto &val: data) {
    if(val.type() != _intl_::matching_enum<message::arg_obj>::val)
      throw proto_error("Invalid contained type in channel_list_response.data");

    auto obj = static_cast<message::arg_obj>(val);
    channel_short ch = {
        .id = require_arg<int32_t>("id", obj),
        .name = require_arg<std::string>("name", obj)
    };
    vec.push_back(ch);
  }

  return { {}, vec /* data */ };
}

message channel_list_response::to() const {
  message::arg_list lst;
  for(const auto &chan: data) {
    message::arg_obj obj;
    obj.set(paired("id", chan.id));
    obj.set(paired("name", chan.name));
    lst.push_back(obj);
  }

  return {
      (*this).okay_response::to(),
      paired("data", lst)
  };
}

// CHANNEL MESSAGE RESPONSE
channel_msg_response channel_msg_response::from(const proto::message &m) {
  auto msgs = require_arg<proto::message::arg_list>("msgs", m.map());
  std::vector<message> res;

  for(const auto &val: msgs) {
    if (val.type() != _intl_::matching_enum<proto::message::arg_obj>::val)
      throw proto_error("Invalid contained type in channel_msg_response.msgs");

    auto obj = static_cast<proto::message::arg_obj>(val);
    message msg {
      .sender = require_arg<decltype(message::sender)>("sender", obj),
      .when = require_arg<decltype(message::when)>("when", obj),
      .cnt = require_arg<decltype(message::cnt)>("cnt", obj)
    };

    res.push_back(msg);
  }

  return { {}, res };
}

message channel_msg_response::to() const {
  proto::message::arg_list lst;
  for(const auto &msg: msgs) {
    proto::message::arg_obj obj;
    obj.set(paired("sender", msg.sender));
    obj.set(paired("when", msg.when));
    obj.set(paired("cnt", msg.cnt));
    lst.push_back(obj);
  }

  return {
      (*this).okay_response::to(),
      paired("msgs", lst)
  };
}

// CHANNEL DETAILS RESPONSE
channel_details_response channel_details_response::from(const dotchat::proto::message &m) {
  auto cid = require_arg<decltype(id)>("id", m.map());
  auto cname = require_arg<decltype(name)>("name", m.map());
  auto cowner = require_arg<decltype(owner_id)>("owner_id", m.map());
  auto cdesc = require_arg<std::string>("desc", m.map());

  auto lst = require_arg<message::arg_list>("members", m.map());
  decltype(members) res;
  for(const auto &val: lst) {
    if(val.type() != _intl_::matching_enum<decltype(res)::value_type>::val)
      throw proto_error("Invalid contained type in channel_details_response.members");

    res.push_back(static_cast<decltype(res)::value_type>(val));
  }

  return {
      {},
      cid, cname, cowner, cdesc.empty() ? std::nullopt : decltype(desc){cdesc}, res
  };
}

message channel_details_response::to() const {
  message::arg_list lst;
  for(const auto &mem: members) lst.push_back(mem);

  return {
      (*this).okay_response::to(),
      paired("id", id),
      paired("name", name),
      paired("owner_id", owner_id),
      paired("desc", desc.has_value() ? desc.value() : ""),
      paired("members", lst)
  };
}

// USER DETAILS RESPONSE
user_details_response user_details_response::from(const dotchat::proto::message &m) {
  auto _id = require_arg<decltype(id)>("id", m.map());
  auto _name = require_arg<decltype(name)>("name", m.map());

  auto lst = require_arg<message::arg_list>("mutual_channels", m.map());
  decltype(mutual_channels) _mutual;
  for(const auto &val: lst) {
    if(val.type() != _intl_::matching_enum<decltype(_mutual)::value_type>::val)
      throw proto_error("Invalid contained type in user_details_response.mutual_channels");

    _mutual.push_back(static_cast<decltype(_mutual)::value_type>(val));
  }

  return {
      {},
      _id, _name, _mutual
  };
}

message user_details_response::to() const {
  message::arg_list lst;
  for(const auto &mut: mutual_channels) lst.push_back(mut);

  return {
      (*this).okay_response::to(),
      paired("id", id),
      paired("name", name),
      paired("mutual_channels", lst)
  };
}