/////////////////////////////////////////////////////////////////////////////
// Name:        login_related.cpp
// Purpose:     CLI interface for the dotchat client (login related impl)
// Author:      jay-tux
// Created:     August 16, 2022 2:55 PM
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

int32_t cli::send_login() {
  std::string name;
  std::string pass;
  std::cout << "Username: ";
  std::cin >> name;
  cli::flush();
  std::cout << "Password: ";
  std::cin >> pass;
  cli::flush();

  return run_boilerplate<login_response>(
      login_request{.user = name, .pass = pass}
  ).token;
}

void cli::send_logout(int32_t token) {
  run_boilerplate<logout_response>(
      logout_request{ { .token = token } }
  );
}

void cli::send_new_user() {
  std::string name;
  std::string pass1;
  std::string pass2;

  std::cout << "Username: ";
  std::cin >> name;
  cli::flush();
  do {
    if(pass1 != pass2) std::cout << "Passwords didn't match. Try again." << std::endl;

    std::cout << "Password: ";
    std::cin >> pass1;
    cli::flush();
    std::cout << "Repeat password: ";
    std::cin >> pass2;
    cli::flush();
  } while(pass1 != pass2);

  run_boilerplate<new_user_response>(
      new_user_request{ .name = name, .pass = pass1 }
  );
}