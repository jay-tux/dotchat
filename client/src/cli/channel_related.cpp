/////////////////////////////////////////////////////////////////////////////
// Name:        channel_related.cpp
// Purpose:     CLI interface for the dotchat client (channel related impl)
// Author:      jay-tux
// Created:     August 16, 2022 6:50 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include "cli.hpp"

using namespace dotchat::proto;
using namespace dotchat::proto::requests;
using namespace dotchat::proto::responses;
using namespace dotchat::tls;
using namespace dotchat::client;

channel_list_response cli::send_channel_list(int32_t token) {
  return run_boilerplate<channel_list_response>(
      channel_list_request{ { .token = token } }
  );
}

channel_msg_response cli::send_channel_message_list(int32_t token, int32_t chan_id) {
  return run_boilerplate<channel_msg_response>(
      channel_msg_request{ { .token = token }, chan_id }
  );
}

channel_details_response cli::send_channel_details(int32_t token, int32_t chan_id) {
  return run_boilerplate<channel_details_response>(
      channel_details_request{ { .token = token }, chan_id }
  );
}

void cli::send_send_message(int32_t token, int32_t chan_id) {
  std::string msg;
  std::cout << "Message to send: ";
  std::getline(std::cin, msg);

  run_boilerplate<message_send_response>(
      message_send_request{ { .token = token }, chan_id, msg }
  );
}

void cli::send_create_channel(int32_t token) {
  std::string name;
  std::string desc;
  std::cout << "Name for the new channel? ";
  std::cin >> name;
  cli::flush();
  if(name[0] != '#') name = "#" + name;
  std::cout << "Will create " << name << std::endl;
  std::cout << "Description for the channel (optional)? ";
  std::getline(std::cin, desc);

  run_boilerplate<new_channel_response>(
      new_channel_request{
        { .token = token },
        name,
        desc.empty() ? decltype(new_channel_request::desc)(std::nullopt) : desc
      }
  );
}