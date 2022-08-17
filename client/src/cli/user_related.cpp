/////////////////////////////////////////////////////////////////////////////
// Name:        user_related.cpp
// Purpose:     CLI interface for the dotchat client (user related impl)
// Author:      jay-tux
// Created:     August 16, 2022 7:03 PM
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

user_details_response cli::send_user_details(int32_t token, int32_t uid) {
  return run_boilerplate<user_details_response>(
      user_details_request{ { .token = token }, uid }
  );
}

void cli::send_change_pass(int32_t token) {
  std::string pass1;
  std::string pass2;
  do {
    if(pass1 != pass2)
      std::cout << "Passwords don't match. Please try again." << std::endl;
    std::cout << "New password: ";
    std::cin >> pass1;
    cli::flush();
    std::cout << "Repeat new password: ";
    std::cin >> pass2;
    cli::flush();
  } while(pass1 != pass2);

  run_boilerplate<change_pass_response>(
      change_pass_request{ { .token = token }, pass1 }
  );
}

void cli::send_user_invite(int32_t token, int32_t uid, int32_t chan_id) {
  run_boilerplate<invite_user_response>(
      invite_user_request{ { .token = token }, uid, chan_id }
  );
}