/////////////////////////////////////////////////////////////////////////////
// Name:        handlers.hpp
// Purpose:     The handlers for the commands
// Author:      jay-tux
// Created:     August 10, 2022 10:14 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short The handlers for the commands.
 */

#ifndef DOTCHAT_CLIENT_HANDLERS_HPP
#define DOTCHAT_CLIENT_HANDLERS_HPP

#include "protocol/message.hpp"
#include "protocol/requests.hpp"

/**
 * \short Namespace for all code related to the server.
 */
namespace dotchat::server {
/**
 * \short Type alias for the error type (`dotchat::proto::proto_error`).
 */
using dotchat::proto::proto_error;

/**
 * \short Structure containing the handlers.
 */
struct handlers {
  /**
   * \short The handler callback type (functions of type `const dotchat::proto::message & -> dotchat::proto::message`).
   */
  using callback_t = proto::message (*) (const proto::message &);
  /**
   * \short The pair type used in the multiplexer (`std::pair<std::string, dotchat::server::handlers::callback_t`>).
   */
  using pair_t = std::pair<std::string, callback_t>;
  /**
   * \short Type alias for the request command collection (`dotchat::proto::requests::request_commands`).
   */
  using cmd_coll = proto::requests::request_commands;

  /**
   * \short Callback for login requests.
   */
  static callback_t login;
  /**
   * \short Callback for logout requests.
   */
  static callback_t logout;
  /**
   * \short Callback for channel listing requests.
   */
  static callback_t channel_list;
  /**
   * \short Callback for channel message listing requests.
   */
  static callback_t channel_msg;
  /**
   * \short Callback for message sending requests.
   */
  static callback_t send_msg;
  /**
   * \short Callback for channel detail requests.
   */
  static callback_t channel_details;
  /**
   * \short Callback for channel creation requests.
   */
  static callback_t new_channel;
  /**
   * \short Callback for signup requests.
   */
  static callback_t new_user;
  /**
   * \short Callback for password change requests.
   */
  static callback_t change_pass;
  /**
   * \short Callback for user detail requests.
   */
  static callback_t user_details;
  /**
   * \short Callback for user invitation requests.
   */
  static callback_t invite_user;

  /**
   * \short Multiplexer mapping commands to their correct callbacks.
   */
  const static inline std::map<std::string, callback_t, std::less<>> switcher {
#define ADD(command) pair_t{ cmd_coll::command, command }
      ADD(login), ADD(logout), ADD(channel_list), ADD(channel_msg),
      ADD(send_msg), ADD(channel_details), ADD(new_channel),
      ADD(new_user), ADD(change_pass), ADD(user_details), ADD(invite_user)
#undef ADD
  };
};
}

#endif //DOTCHAT_CLIENT_HANDLERS_HPP
