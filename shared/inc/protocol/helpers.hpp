/////////////////////////////////////////////////////////////////////////////
// Name:        helpers.hpp
// Purpose:     Helpers for message to request/response parsers
// Author:      jay-tux
// Created:     August 11, 2022 3:20 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short Helpers for message to request/response parsers.
 */

#ifndef DOTCHAT_HELPERS_HPP
#define DOTCHAT_HELPERS_HPP

#include <string>
#include "protocol/message.hpp"
#include "protocol/requests.hpp"

/**
 * \short Namespace containing all code related to the dotchat protocol.
 */
namespace dotchat::proto {
/**
 * \short Concept relaying the meaning of a response function type.
 * \tparam Fun The type of the function.
 * \tparam In The input argument type.
 * \tparam Out The output argument type.
 *
 * The type `Fun` should be callable with an argument of `const In &`, returning a value of type `Out`.
 * In other words, `Fun` should be a `In -> Out` function.
 */
template <typename Fun, typename In, typename Out>
concept response_fun = requires(Fun &&f, const In &in) {
  { f(in) } -> std::same_as<Out>;
};

/**
 * \short Helper function to extract an argument with a certain type from an arg_obj.
 * \tparam T The (expected) type of the argument to extract (must be representable).
 * \param key The key of the argument to extract.
 * \param source The arg_obj to search.
 * \returns The extracted value (if present).
 * \throws `dotchat::proto::proto_error` if the key is not present or it doesn't have the correct type.
 */
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

/**
 * \short Wraps a reply function, converting a `Req -> Res` function to `const message & -> message`.
 * \tparam Req The request type. Should satisfy `dotchat::proto::from_message_convertible<Req>`.
 * \tparam Res The response type. Should satisfy `dotchat::proto::to_message_convertible<Res>`.
 * \tparam Fun The function type. Should satisfy `dotchat::proto::response_fun<Fun, Req, Res>` (be a `Req -> Res` function).
 * \param m The message to reply to.
 * \param f The reply function.
 * \returns The reply of the function, or an error message.
 *
 * This function tries to construct an object of type `Req` by using `static Req::from(const message &)`; which is
 * passed as argument to `f`. The result of the function is returned from the function using `Res::to()`. If any
 * `dotchat::proto::proto_error` occurs, it is caught and an error message is returned instead (using
 * `dotchat::proto::responses::error_response::to()`.
 */
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
}

#endif //DOTCHAT_HELPERS_HPP
