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
okay_response okay_response::from(const dotchat::proto::message &) {
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