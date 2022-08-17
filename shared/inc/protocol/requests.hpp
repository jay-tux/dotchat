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
#include <chrono>
#include <optional>
#include "message.hpp"
#include "either.hpp"

namespace dotchat::proto {
using clock_t = std::chrono::system_clock;

inline auto now() {
  return static_cast<uint32_t>(
    std::chrono::time_point_cast<std::chrono::milliseconds>(
        clock_t::now()
    ).time_since_epoch().count()
  );
}

using now_t = decltype(now());

inline auto from_now(now_t val) {
  using namespace std::chrono_literals;
  auto ms = std::chrono::milliseconds(val);
  auto time = std::chrono::time_point<clock_t>(ms);
  return time;
}

using from_now_t = decltype(from_now(0));

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
  const inline static std::string channel_msg = "channel_msg";
  const inline static std::string send_msg = "msg_send";
  const inline static std::string channel_details = "chan_detail";
  const inline static std::string new_channel = "new_chan";
  const inline static std::string new_user = "new_usr";
  const inline static std::string change_pass = "ch_pass";
  const inline static std::string user_details = "usr_detail";
  const inline static std::string invite_user = "invite";
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

struct channel_msg_request : public token_request {
  int32_t chan_id;

  static channel_msg_request from(const message &m);
  [[nodiscard]] message to() const;
};

struct message_send_request : public token_request {
  int32_t chan_id;
  std::string msg_cnt;

  static message_send_request from(const message &m);
  [[nodiscard]] message to() const;
};

struct channel_details_request : public token_request {
  int32_t chan_id;

  static channel_details_request from(const message &m);
  [[nodiscard]] message to() const;
};

struct new_channel_request : public token_request {
  std::string name;
  std::optional<std::string> desc;

  static new_channel_request from(const message &m);
  [[nodiscard]] message to() const;
};

struct new_user_request {
  std::string name;
  std::string pass;

  static new_user_request from(const message &m);
  [[nodiscard]] message to() const;
};

struct change_pass_request : public token_request {
  std::string new_pass;

  static change_pass_request from(const message &m);
  [[nodiscard]] message to() const;
};

struct user_details_request : public token_request {
  int32_t uid;

  static user_details_request from(const message &m);
  [[nodiscard]] message to() const;
};

struct invite_user_request : public token_request {
  int32_t uid;
  int32_t chan_id;

  static invite_user_request from(const message &m);
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
  [[nodiscard]] message to() const;
};

struct id_response : public okay_response {
  int32_t id;
  static id_response from(const message &m);
  [[nodiscard]] message to() const;
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
  [[nodiscard]] message to() const;
};

struct channel_msg_response : public okay_response {
  struct message {
    int32_t sender;
    uint32_t when;
    std::string cnt;
  };

  std::vector<message> msgs;

  static channel_msg_response from(const proto::message &m);
  [[nodiscard]] proto::message to() const;
};

using message_send_response = okay_response;

struct channel_details_response : public okay_response {
  int32_t id;
  std::string name;
  int32_t owner_id;
  std::optional<std::string> desc;
  std::vector<int32_t> members;

  static channel_details_response from(const message &m);
  [[nodiscard]] message to() const;
};

using new_channel_response = id_response;
using new_user_response = okay_response;
using change_pass_response = okay_response;

struct user_details_response : okay_response {
  int32_t id;
  std::string name;
  std::vector<int32_t> mutual_channels;

  static user_details_response from(const message &m);
  [[nodiscard]] message to() const;
};

using invite_user_response = okay_response;
}
}

#endif //DOTCHAT_RESPONSES_HPP
