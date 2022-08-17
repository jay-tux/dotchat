/////////////////////////////////////////////////////////////////////////////
// Name:        cli.hpp
// Purpose:     CLI interface for the dotchat client.
// Author:      jay-tux
// Created:     August 16, 2022 12:39 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_CLIENT_CLI_HPP
#define DOTCHAT_CLIENT_CLI_HPP

#include "tls/tls_connection.hpp"
#include "tls/tls_bytestream.hpp"
#include "protocol/requests.hpp"
#include <stdexcept>
#include <iostream>

namespace dotchat::client {
struct cli {
  struct cli_error : std::logic_error {
    using logic_error::logic_error;

    static inline cli_error non_okay() { return cli_error("Non-okay response"); }
    static inline cli_error unprasable() { return cli_error("Unparsable response"); }
  };

  inline explicit cli(tls::tls_connection &conn) : conn{conn} {}
  cli(const cli &other) = delete;
  cli(cli && other) = delete;

  cli &operator=(const cli &other) = delete;
  cli &operator=(cli &&other) = delete;

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
        std::cout << "Failed to log out!" << std::endl;
        auto err = proto::responses::error_response::from(resp);
        std::cout << "  Reason: " << err.reason;
        throw cli_error::non_okay();
      }
    }
    catch(const proto::proto_error &err) {
      std::cout << "Failed to parse response correctly!" << std::endl
                << "  Reason: " << err.what() << std::endl;
      throw cli_error::unprasable();
    }
  }

  inline void operator()() { run_wait_loop(); }
  void run_wait_loop();
  int32_t send_login();
  void send_logout(int32_t token);
  proto::responses::channel_list_response send_channel_list(int32_t token);
  proto::responses::channel_msg_response send_channel_message_list(int32_t token, int32_t chan_id);
  proto::responses::channel_details_response send_channel_details(int32_t token, int32_t chan_id);
  void send_send_message(int32_t token, int32_t chan_id);
  void send_create_channel(int32_t token);
  void send_new_user();
  proto::responses::user_details_response send_user_details(int32_t token, int32_t uid);
  void send_change_pass(int32_t token);
  void send_user_invite(int32_t token, int32_t uid, int32_t chan_id);

  constexpr ~cli() = default;

  inline static void flush() {
    std::string _;
    std::getline(std::cin, _);
  }

  tls::tls_connection &conn;
};
}

#endif //DOTCHAT_CLIENT_CLI_HPP
