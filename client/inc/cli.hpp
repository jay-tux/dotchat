/////////////////////////////////////////////////////////////////////////////
// Name:        cli.hpp
// Purpose:     CLI interface for the dotchat client.
// Author:      jay-tux
// Created:     August 16, 2022 12:39 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short CLI interface for the dotchat client.
 */

#ifndef DOTCHAT_CLIENT_CLI_HPP
#define DOTCHAT_CLIENT_CLI_HPP

#include "tls/tls_connection.hpp"
#include "tls/tls_bytestream.hpp"
#include "protocol/requests.hpp"
#include <stdexcept>
#include <iostream>

/**
 * \short Namespace containing code for the dotchat client.
 */
namespace dotchat::client {
/**
 * \short Structure containing the CLI interactions for a given TLS connection (`dotchat::tls::tls_connection`).
 */
struct cli {
  /**
   * \short Structure representing an error during CLI operations.
   */
  struct cli_error : std::logic_error {
    using logic_error::logic_error;

    /**
     * \short `dotchat::client::cli::cli_error` to be thrown when a response was not a success response.
     * \returns A new CLI error.
     */
    static inline auto non_okay() { return cli_error("Non-okay response"); }
    /**
     * \short `dotchat::client::cli::cli_error` to be thrown when a response was not of the expected kind.
     * \returns A new CLI error.
     */
    static inline auto unparsable() { return cli_error("Unparsable response"); }
  };

  /**
   * \short Constructs a new CLI from a TLS connection.
   * \param conn A reference to the connection to work with.
   */
  inline explicit cli(tls::tls_connection &conn) : conn{conn} {}
  /**
   * \short Copy-initialization of a CLI is not supported.
   */
  cli(const cli &other) = delete;
  /**
   * \short Move-initialization of a CLI is not supported.
   */
  cli(cli && other) = delete;

  /**
   * \short Copy-assignment of a CLI is not supported.
   */
  cli &operator=(const cli &other) = delete;
  /**
   * \short Move-assignment of a CLI is not supported.
   */
  cli &operator=(cli &&other) = delete;

  /**
   * \short Boilerplate code for message sending. Sends the message and attempts to parse its response.
   * \tparam Res The (expected) response type. Should satisfy `dotchat::proto::from_message_convertible<Res>`.
   * \tparam Req The request type. Should satisfy `dotchat::proto::to_message_convertible<Req>`.
   * \param r The request to send.
   * \returns The response from the server.
   * \throws `dotchat::client::cli::cli_error` if the response couldn't be parsed.
   * \throws `dotchat::client::cli::cli_error` if the response was not a success response.
   */
  template <proto::from_message_convertible Res, proto::to_message_convertible Req>
  Res run_boilerplate(const Req &r) {
    tls::bytestream strm;
    strm << r.to();
    conn.send(strm);
    strm = conn.read();
    proto::message resp(strm);

    try {
      if(resp.get_command() == proto::responses::response_commands::okay) {
        return Res::from(resp);
      }
      else {
        std::cout << "Action failed!" << std::endl;
        auto err = proto::responses::error_response::from(resp);
        std::cout << "  Reason: " << err.reason;
        throw cli_error::non_okay();
      }
    }
    catch(const proto::proto_error &err) {
      std::cout << "Failed to parse response correctly!" << std::endl
                << "  Reason: " << err.what() << std::endl;
      throw cli_error::unparsable();
    }
  }

  /**
   * \short Runs the event/interface loop.
   */
  inline void operator()() { run_wait_loop(); }
  /**
   * \short Runs the event/interface loop.
   */
  void run_wait_loop();

  /**
   * \short Interface and functional code to send a login request.
   * \returns The token obtained from the login process.
   */
  int32_t send_login();
  /**
   * \short Interface and functional code to send a log-out request.
   * \param token The token to use in the request.
   */
  void send_logout(int32_t token);
  /**
   * \short Interface and functional code to send a channel listing request.
   * \param token The token to use in the request.
   * \returns The parsed response message.
   */
  proto::responses::channel_list_response send_channel_list(int32_t token);
  /**
   * \short Interface and functional code to send a channel message listing request.
   * \param token The token to use in the request.
   * \returns The parsed response message.
   */
  proto::responses::channel_msg_response send_channel_message_list(int32_t token, int32_t chan_id);
  /**
   * \short Interface and functional code to send a channel details request.
   * \param token The token to use in the request.
   * \returns The parsed response message.
   */
  proto::responses::channel_details_response send_channel_details(int32_t token, int32_t chan_id);
  /**
   * \short Interface and functional code to send a message to a channel.
   * \param token The token to use in the request.
   * \param chan_id The ID of the channel to send the message in.
   */
  void send_send_message(int32_t token, int32_t chan_id);
  /**
   * \short Interface and functional code to send a request to create a new channel.
   * \param token The token to use in the request.
   */
  void send_create_channel(int32_t token);
  /**
   * \short Interface and functional code to send a signup request.
   */
  void send_new_user();
  /**
   * \short Interface and functional code to send a user details request.
   * \param token The token to use in the request.
   * \param uid The ID of the user whose details to request.
   * \returns The parsed response message.
   */
  proto::responses::user_details_response send_user_details(int32_t token, int32_t uid);
  /**
   * \short Interface and functional code to send a change password request.
   * \param token The token to use in the request.
   */
  void send_change_pass(int32_t token);
  /**
   * \short Interface and functional code to invite a user to a channel.
   * \param token The token to use in the request.
   * \param uid The ID of the user to invite.
   * \param chan_id The ID of the channel to invite to.
   * \returns The parsed response message.
   */
  void send_user_invite(int32_t token, int32_t uid, int32_t chan_id);

  /**
   * \short Cleans up all resources used by the CLI.
   */
  constexpr ~cli() = default;

  /**
   * \short Flushes the stdin buffer.
   */
  inline static void flush() {
    std::string _;
    std::getline(std::cin, _);
  }

  /**
   * \short The internal TLS connection.
   */
  tls::tls_connection &conn;
};
}

#endif //DOTCHAT_CLIENT_CLI_HPP
