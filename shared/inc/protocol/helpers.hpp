/////////////////////////////////////////////////////////////////////////////
// Name:        helpers.hpp
// Purpose:     Helpers for message to request/response parsers
// Author:      jay-tux
// Created:     August 11, 2022 3:20 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_HELPERS_HPP
#define DOTCHAT_HELPERS_HPP

#include <string>
#include "protocol/message.hpp"
#include "protocol/requests.hpp"

namespace dotchat::proto {
template <typename Fun, typename In, typename Out>
concept response_fun = requires(Fun &&f, const In &in) {
  { f(in) } -> std::same_as<Out>;
};

template <typename T>
static T require_arg(const std::string &key, const message::arg_obj &source) {
  if (!source.contains(key)) {
    throw proto_error("Key `" + key + "` not present.");
  }
  if (source.type(key) != proto::_intl_::matching_enum<T>::val) {
    throw proto_error("Key `" + key + "` doesn't have the correct type.");
  }
  return source[key].get<proto::_intl_::matching_enum<T>::val>();
}

template <from_message_convertible Req, to_message_convertible Res, typename Fun>
message reply_to(const message &m, Fun &&f) requires(response_fun<Fun, Req, Res>) {
  try {
    Req req = Req::from(m);
    Res res = f(req);
    return res.to();
  }
  catch(const proto_error &e) {
    return responses::error_response{ .reason = e.what() }.to();
  }
}

template <from_message_convertible Res>
Res dispatch(const message &m) {
  if(m.get_command() == responses::response_commands::okay) return Res::from(m);
  else if(m.get_command() == responses::response_commands::error)
    throw proto_error("Got a non-successful response from the server.");
  throw proto_error("Message with invalid command `" + m.get_command() + "`. Expected `ok` or `err`.");
}
}

#endif //DOTCHAT_HELPERS_HPP
