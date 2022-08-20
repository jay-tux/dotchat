/////////////////////////////////////////////////////////////////////////////
// Name:        responses.hpp
// Purpose:     Message to request/response converters
// Author:      jay-tux
// Created:     August 11, 2022 3:06 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short Message to request/response converters.
 */

#ifndef DOTCHAT_RESPONSES_HPP
#define DOTCHAT_RESPONSES_HPP

#include <concepts>
#include <string>
#include <chrono>
#include <optional>
#include "message.hpp"
#include "either.hpp"

/**
 * \short Namespace containing all code related to the dotchat protocol.
 */
namespace dotchat::proto {
/**
 * \short The type of clock used when sending timestamps (`std::chrono::system_clock`).
 */
using clock_t = std::chrono::system_clock;

/**
 * \short Gets the current time stamp (according to `dotchat::proto::clock_t`) as a representable value.
 * \returns The current time stamp.
 *
 * In practice, this value will be a `uint32_t`.
 */
inline auto now() {
  return static_cast<uint32_t>(
    std::chrono::time_point_cast<std::chrono::milliseconds>(
        clock_t::now()
    ).time_since_epoch().count()
  );
}

/**
 * \short Type alias for the representable "now" type (`uint32_t`).
 */
using now_t = decltype(now());

/**
 * \short Converts a value representing a time stamp back to the C++ `std::time_point`.
 * \param val The value to convert.
 * \returns The `std::time_point` corresponding to the given value.
 *
 * In practice, this value will be an `std::chrono::time_point<std::chrono::system_clock, std::chrono::duration>`.
 */
inline auto from_now(now_t val) {
  using namespace std::chrono_literals;
  auto ms = std::chrono::milliseconds(val);
  auto time = std::chrono::time_point<clock_t>(ms);
  return time;
}

/**
 * \short Type alias for the time point type used (
 * `std::chrono::time_point<std::chrono::system_clock, std::chrono::duration>`).
 */
using from_now_t = decltype(from_now(0));

/**
 * \short Concept relaying the meaning of a type which is convertible from a message.
 * \tparam T The type to check.
 *
 * A type satisfies this concept if it has a static member function `T T::from(const dotchat::proto::message &)`.
 */
template <typename T>
concept from_message_convertible = requires(const message &m) {
  { T::from(m) } -> std::same_as<T>; // static from method
};

/**
 * \short Concept relaying the meaning of a type which is constant-convertible to a message.
 * \tparam T The type to check.
 *
 * A type satisfies this concept if it has a member function `dotchat::proto::message T::to() const`.
 */
template <typename T>
concept to_message_convertible = requires(const T &t) {
  { t.to() } -> std::same_as<message>; // member to method (const)
};

/**
 * \short Structure representing a failed expectation by the protocol (missing keys, wrong argument types, ...).
 * \see `std::logic_error`
 */
struct proto_error : public std::logic_error {
  using logic_error::logic_error;
};

/**
 * \short Namespace containing the request structures.
 */
namespace requests {
/**
 * \short Structure containing several constant `std::string`s representing the valid commands.
 */
struct request_commands {
  const inline static std::string login = "login";                 /*!< \short The login command. */
  const inline static std::string logout = "logout";               /*!< \short The log out command. */
  const inline static std::string channel_list = "channel_lst";    /*!< \short The channel listing command. */
  const inline static std::string channel_msg = "channel_msg";     /*!< \short The channel message listing command. */
  const inline static std::string send_msg = "msg_send";           /*!< \short The message sending command. */
  const inline static std::string channel_details = "chan_detail"; /*!< \short The channel detail command. */
  const inline static std::string new_channel = "new_chan";        /*!< \short The channel creation command. */
  const inline static std::string new_user = "new_usr";            /*!< \short The signing up command. */
  const inline static std::string change_pass = "ch_pass";         /*!< \short The password change command. */
  const inline static std::string user_details = "usr_detail";     /*!< \short The user detail command. */
  const inline static std::string invite_user = "invite";          /*!< \short The user invite command. */
};

/**
 * \short Structure representing a request containing only a token (base class).
 */
struct token_request {
  /**
   * \short The contained token.
   */
  int32_t token;

  /**
   * \short Converts a message into a token request.
   * \param m The message to convert.
   * \returns A new token request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static token_request from(const message &m);
protected:
  /**
   * \short Converts a token request to a message.
   * \param command The command for the request.
   * \returns A new message.
   */
  [[nodiscard]] message to_intl(const message::command &command) const;
};

/**
 * \short Structure representing a login request.
 */
struct login_request {
  /**
   * \short The username to log in with.
   */
  std::string user;
  /**
   * \short The password to log in with.
   */
  std::string pass;

  /**
   * \short Converts a message into a login request.
   * \param m The message to convert.
   * \returns A new login request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static login_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a logout request.
 * \see dotchat::proto::requests::token_request
 */
struct logout_request : public token_request {
  /**
   * \short Converts a message into a logout request.
   * \param m The message to convert.
   * \returns A new logout request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static logout_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a channel listing request.
 * \see dotchat::proto::requests::token_request
 */
struct channel_list_request : public token_request {
  /**
   * \short Converts a message into a channel listing request.
   * \param m The message to convert.
   * \returns A new channel listing request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static channel_list_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a channel message listing request.
 * \see dotchat::proto::requests::token_request
 */
struct channel_msg_request : public token_request {
  /**
   * \short The ID of the channel whose messages to request.
   */
  int32_t chan_id;

  /**
   * \short Converts a message into a channel message listing request.
   * \param m The message to convert.
   * \returns A new channel message listing request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static channel_msg_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a message sending request.
 * \see dotchat::proto::requests::token_request
 */
struct message_send_request : public token_request {
  /**
   * \short The ID of the channel in which to send a message.
   */
  int32_t chan_id;
  /**
   * \short The content of the message.
   */
  std::string msg_cnt;

  /**
   * \short Converts a message into a message sending request.
   * \param m The message to convert.
   * \returns A new message sending request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static message_send_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a channel details request.
 * \see dotchat::proto::requests::token_request
 */
struct channel_details_request : public token_request {
  /**
   * \short The ID of the channel whose details to request.
   */
  int32_t chan_id;

  /**
   * \short Converts a message into a channel details request.
   * \param m The message to convert.
   * \returns A new channel details request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static channel_details_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a channel creation request.
 * \see dotchat::proto::requests::token_request
 */
struct new_channel_request : public token_request {
  /**
   * \short The name for the new channel.
   */
  std::string name;
  /**
   * \short An optional description of the channel.
   */
  std::optional<std::string> desc;

  /**
   * \short Converts a message into a channel creation request.
   * \param m The message to convert.
   * \returns A new channel creation request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static new_channel_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a sign-up request.
 */
struct new_user_request {
  /**
   * \short The name for the new user.
   */
  std::string name;
  /**
   * \short The password for the new user.
   */
  std::string pass;
 /**
   * \short Converts a message into a sign-up request.
   * \param m The message to convert.
   * \returns A new sign-up request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static new_user_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a password change request.
 * \see dotchat::proto::requests::token_request
 */
struct change_pass_request : public token_request {
  /**
   * \short The new password for this user.
   */
  std::string new_pass;

  /**
   * \short Converts a message into a password change request.
   * \param m The message to convert.
   * \returns A new password change request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static change_pass_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a user details request.
 * \see dotchat::proto::requests::token_request
 */
struct user_details_request : public token_request {
  /**
   * \short The ID of the user whose details to request.
   */
  int32_t uid;

  /**
   * \short Converts a message into a user details request.
   * \param m The message to convert.
   * \returns A new user details request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static user_details_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a user invite request.
 * \see dotchat::proto::requests::token_request
 */
struct invite_user_request : public token_request {
  /**
   * \short The ID of the user who is to join the channel.
   */
  int32_t uid;
  /**
   * \short The ID of the channel to join.
   */
  int32_t chan_id;

  /**
   * \short Converts a message into a user invite request.
   * \param m The message to convert.
   * \returns A new user invite request.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static invite_user_request from(const message &m);
  /**
   * \short Converts this request to a message.
   * \returns A new message, equivalent to this request.
   */
  [[nodiscard]] message to() const;
};
}

/**
 * \short Namespace containing the response structures.
 */
namespace responses {
/**
 * \short Structure containing several constant std::strings representing the valid commands.
 */
struct response_commands {
  const inline static std::string okay = "ok";    /*!< \short Command indicating a success response. */
  const inline static std::string error = "err";  /*!< \short Command indicating a failure response. */
};

/**
 * \short Structure representing a success response.
 */
struct okay_response {
  /**
   * \short Converts a message into a success response.
   * \param m The message to convert.
   * \returns A new success response.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static okay_response from(const message &m);
  /**
   * \short Converts this response into a message.
   * \returns A new message, equivalent to this response.
   */
  [[nodiscard]] virtual message to() const;
  /**
   * \short Cleans up all resources used by this response.
   */
  virtual ~okay_response() = default;
};

/**
 * \short Structure representing an erroneous response.
 */
struct error_response {
  /**
   * \short The reason for this error.
   */
  std::string reason;
  /**
   * \short Converts a message into an erroneous response.
   * \param m The message to convert.
   * \returns A new error response.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static error_response from(const message &m);
  /**
   * \short Converts an error into an error response.
   * \param err The error to convert.
   * \returns A new error response.
   *
   * This method is mostly intended for using in try-catch blocks (where `err.what()` is used as reason.
   */
  static error_response from(const proto_error &err);
  /**
   * \short Converts this response into a message.
   * \returns A new message, equivalent to this response.
   */
  [[nodiscard]] message to() const;
};

/**
 * \short Structure representing a response holding only a token.
 */
struct token_response : public okay_response {
  /**
   * \short The token.
   */
  int32_t token = 0;

  /**
   * \short Constructs a default token response.
   */
  constexpr token_response() = default;
  /**
   * \short Constructs a token response from all required values.
   * \param o The `dotchat::proto::responses::okay_response` this response is based on.
   * \param token The token to include.
   */
  constexpr token_response(const okay_response &o, int32_t token): okay_response(o), token{token} {}

  /**
   * \short Converts a message into a token response.
   * \param m The message to convert.
   * \returns A new token response.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static token_response from(const message &m);
  /**
   * \short Converts this response into a message.
   * \returns A new message, equivalent to this response.
   */
  [[nodiscard]] message to() const override;
};

/**
 * \short Structure representing a response holding only an ID.
 * \see `dotchat::proto::responses::okay_response`
 */
struct id_response : public okay_response {
  /**
   * \short The ID.
   */
  int32_t id = 0;

  /**
   * \short Constructs a default ID response.
   */
  constexpr id_response() = default;
  /**
   * \short Constructs an ID response from all required values.
   * \param o The `dotchat::proto::responses::okay_response` this response is based on.
   * \param id The ID to include.
   */
  constexpr id_response(const okay_response &o, int32_t id): okay_response(o), id{id} {}

  /**
   * \short Converts a message into an ID response.
   * \param m The message to convert.
   * \returns A new ID response.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static id_response from(const message &m);
  /**
   * \short Converts this response into a message.
   * \returns A new message, equivalent to this response.
   */
  [[nodiscard]] message to() const override;
};

/**
 * \short Type alias for a `dotchat::proto::responses::token_response` (because this kind of response only holds a
 * token).
 */
using login_response = token_response;
/**
 * \short Type alias for a `dotchat::proto::responses::okay_response` (because this kind of response holds no data).
 */
using logout_response = okay_response;

/**
 * \short Structure representing a response holding a list of channels (by ID and name).
 * \see `dotchat::proto::responses::okay_response`
 */
struct channel_list_response : public okay_response {
  /**
   * \short Structure representing a short channel description (ID and name).
   */
  struct channel_short {
    /**
     * \short The ID.
     */
    int32_t id;
    /**
     * \short The channel name.
     */
    std::string name;
  };
  /**
   * \short The actual data sent with the request.
   */
  std::vector<channel_short> data;

  /**
   * \short Constructs a default channel listing response.
   */
  constexpr channel_list_response() = default;
  /**
   * \short Constructs a token response from all required values.
   * \param o The `dotchat::proto::responses::okay_response` this response is based on.
   * \param data The short channel descriptions to include.
   */
  constexpr channel_list_response(const okay_response &o, decltype(data) data): okay_response(o), data{std::move(data)} {}

  /**
   * \short Converts a message into a channel listing response.
   * \param m The message to convert.
   * \returns A new channel listing response.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static channel_list_response from(const message &m);
  /**
   * \short Converts this response into a message.
   * \returns A new message, equivalent to this response.
   */
  [[nodiscard]] message to() const override;
};

/**
 * \short Structure representing a response holding a set of messages in the same channel.
 * \see `dotchat::proto::responses::okay_response`
 */
struct channel_msg_response : public okay_response {
  /**
   * \short Structure representing a single message.
   */
  struct message {
    /**
     * \short The user ID of the sender.
     */
    int32_t sender;
    /**
     * \short (Representable) time stamp when the message was sent.
     */
    uint32_t when;
    /**
     * \short The content of the message.
     */
    std::string cnt;
  };

  /**
   * \short Actual container with the messages.
   */
  std::vector<message> msgs;

  /**
   * \short Constructs a default channel message listing response.
   */
  constexpr channel_msg_response() = default;
  /**
   * \short Constructs a token response from all required values.
   * \param o The `dotchat::proto::responses::okay_response` this response is based on.
   * \param msgs The messages to include.
   */
  constexpr channel_msg_response(const okay_response &o, decltype(msgs) msgs): okay_response(o), msgs{std::move(msgs)} {}

  /**
   * \short Converts a message into a channel message listing response.
   * \param m The message to convert.
   * \returns A new channel message listing response.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static channel_msg_response from(const proto::message &m);
  /**
   * \short Converts this response into a message.
   * \returns A new message, equivalent to this response.
   */
  [[nodiscard]] proto::message to() const override;
};

/**
 * \short Type alias for `dotchat::proto::responses::okay_response` (because this kind of response holds no data).
 */
using message_send_response = okay_response;

/**
 * \short Structure representing a response holding the details about a channel.
 * \see `dotchat::proto::responses::okay_response`
 */
struct channel_details_response : public okay_response {
  /**
   * \short The channel ID.
   */
  int32_t id = 0;
  /**
   * \short The channel's name.
   */
  std::string name;
  /**
   * \short The ID of the channel's owner.
   */
  int32_t owner_id = 0;
  /**
   * \short The (optional) description for the channel.
   */
  std::optional<std::string> desc;
  /**
   * \short The user IDs for the members of the channel.
   */
  std::vector<int32_t> members;

  /**
   * \short Constructs a default channel details response.
   */
  constexpr channel_details_response() = default;
  /**
   * \short Constructs a channel details response from all required values.
   * \param o The `dotchat::proto::responses::okay_response` this response is based on.
   * \param id The channel ID to include.
   * \param name The channel name to include.
   * \param owner_id The ID of the owner to include.
   * \param desc The (optional) description to include.
   * \param members The IDs of the channel members to include.
   */
  constexpr channel_details_response(const okay_response &o, int32_t id, std::string name, int32_t owner_id,
                                  decltype(desc) desc, decltype(members) members): okay_response(o), id{id},
                                  name{std::move(name)}, owner_id{owner_id}, desc{std::move(desc)},
                                  members{std::move(members)} {}

  /**
   * \short Converts a message into a channel details response.
   * \param m The message to convert.
   * \returns A new channel details response.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static channel_details_response from(const message &m);
  /**
   * \short Converts this response into a message.
   * \returns A new message, equivalent to this response.
   */
  [[nodiscard]] message to() const override;
};

/**
 * Type alias for `dotchat::proto::responses::id_response` (because this kind of response only holds an ID).
 */
using new_channel_response = id_response;
/**
 * \short Type alias for `dotchat::proto::responses::okay_response` (because this kind of response holds no data).
 */
using new_user_response = okay_response;
/**
 * \short Type alias for `dotchat::proto::responses::okay_response` (because this kind of response holds no data).
 */
using change_pass_response = okay_response;

/**
 * \short Structure representing a response holding the (public) details about a user.
 * \see `dotchat::proto::responses::okay_response`
 */
struct user_details_response : okay_response {
  /**
   * \short The user's ID.
   */
  int32_t id = 0;
  /**
   * \short The user's name.
   */
  std::string name;
  /**
   * \short All channels you have in common with this user.
   */
  std::vector<int32_t> mutual_channels;

  /**
   * \short Constructs a default user details response.
   */
  constexpr user_details_response() = default;
  /**
   * \short Constructs a token response from all required values.
   * \param o The `dotchat::proto::responses::okay_response` this response is based on.
   * \param id The user ID to include.
   * \param name The username to include.
   * \param mutuals The IDs of the mutual channels to include.
   */
  constexpr user_details_response(const okay_response &o, int32_t id, std::string name, decltype(mutual_channels) mutuals):
      okay_response(o), id{id}, name{std::move(name)}, mutual_channels{std::move(mutuals)} {}

  /**
   * \short Converts a message into a user details response.
   * \param m The message to convert.
   * \returns A new user details response.
   * \throws `dotchat::proto::proto_error` if a key is missing or has the wrong type.
   * \throws `dotchat::proto::proto_error` if the command is incorrect.
   */
  static user_details_response from(const message &m);
  /**
   * \short Converts this response into a message.
   * \returns A new message, equivalent to this response.
   */
  [[nodiscard]] message to() const override;
};

/**
 * \short Type alias for `dotchat::proto::responses::okay_response` (because this kind of response holds no data).
 */
using invite_user_response = okay_response;
}
}

#endif //DOTCHAT_RESPONSES_HPP
