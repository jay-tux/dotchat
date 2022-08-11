/////////////////////////////////////////////////////////////////////////////
// Name:        responses.hpp
// Purpose:     Message to request/response converters
// Author:      jay-tux
// Created:     August 11, 2022 3:06 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_RESPONSES_HPP
#define DOTCHAT_RESPONSES_HPP

#include <concepts>
#include <string>
#include "message.hpp"
#include "either.hpp"

namespace dotchat::proto {
template <typename T>
concept from_message_convertible = requires(const message &m) {
  { T::from(m) } -> std::same_as<T>; // static from method
};

template <typename T>
concept to_message_convertible = requires(const T &t) {
  { t.to() } -> std::same_as<message>; // member to method (const)
};

struct proto_error : public std::logic_error {
  using logic_error::logic_error;
};

namespace requests {
struct request_commands {
  const inline static std::string login = "login";
  const inline static std::string logout = "logout";
  const inline static std::string channel_list = "channel_lst";
};

struct token_request {
  int32_t token;

  static token_request from(const message &m);
protected:
  [[nodiscard]] message to_intl(const message::command &command) const;
};

struct login_request {
  std::string user;
  std::string pass;

  static login_request from(const message &m);
  [[nodiscard]] message to() const;
};

struct logout_request : public token_request {
  static logout_request from(const message &m);
  [[nodiscard]] message to() const;
};

struct channel_list_request : public token_request {
  static channel_list_request from(const message &m);
  [[nodiscard]] message to() const;
};
}

namespace responses {
struct response_commands {
  const inline static std::string okay = "ok";
  const inline static std::string error = "err";
};

struct okay_response {
  static okay_response from(const message &m);
  [[nodiscard]] message to() const;
};

struct error_response {
  std::string reason;
  static error_response from(const message &m);
  static error_response from(const proto_error &err);
  [[nodiscard]] message to() const;
};

struct token_response : public okay_response {
  int32_t token;
  static token_response from(const message &m);
  message to() const;
};

using login_response = token_response;
using logout_response = okay_response;

struct channel_list_response : public okay_response {
  struct channel_short {
    int32_t id;
    std::string name;
  };
  std::vector<channel_short> data;

  static channel_list_response from(const message &m);
  message to() const;
};
}
}

#endif //DOTCHAT_RESPONSES_HPP
