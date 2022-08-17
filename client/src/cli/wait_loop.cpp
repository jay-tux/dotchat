/////////////////////////////////////////////////////////////////////////////
// Name:        wait_loop.cpp
// Purpose:     CLI interface for the dotchat client (wait-loop impl)
// Author:      jay-tux
// Created:     August 16, 2022 12:47 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>
#include <string>
#include <chrono>
#include "cli.hpp"
#include "protocol/requests.hpp"

using namespace dotchat::client;
using namespace dotchat::tls;
using namespace dotchat::proto;
using namespace dotchat::proto::responses;

enum class login_action { LOGIN, SIGNUP, QUIT };
enum class main_action { LOGOUT, CHAN_LIST, NEW_CHAN, CH_PASS, QUIT };
enum class chan_action {
  GET_MSGS, SEND_MSG, GET_USRS, INVITE_USR, BACK, QUIT
};

login_action request_login() {
  std::string resp;
  std::cout << "You are currently not logged in." << std::endl
            << "  -> Use .l to log into an existing account, or" << std::endl
            << "  -> Use .s to create a new account." << std::endl;

  while(true) {
    std::cout << "Your choice? ";

    std::getline(std::cin, resp);
    if (resp == ".l") return login_action::LOGIN;
    else if (resp == ".s") return login_action::SIGNUP;
    else if (resp == ".q") return login_action::QUIT;
    std::cout << "Unrecognized command." << std::endl;
  }
}

std::string format_timestamp(uint32_t when) {
  auto time = from_now(when);
  return std::to_string(time.time_since_epoch().count());
}

#define YNRESPONDER \
    std::getline(std::cin, resp); \
    if(resp == "y") return true;  \
    else if(resp == "n") return false; \
    std::cout << "Please answer with y (yes) or n (no) << std::endl"

bool login_after_signup() {
  while(true) {
    std::string resp;
    std::cout << "You're signed up now. Do you want to log in (y/n)? ";
    YNRESPONDER;
  }
}

bool logout_before_quit() {
  while(true) {
    std::string resp;
    std::cout << "Log out before quitting (y/n)? ";
    YNRESPONDER;
  }
}

bool confirm_user(int32_t uid, std::string_view uname) {
  while(true) {
    std::string resp;
    std::cout << "Confirm adding user #" << uid << " (" << uname << ")(y/n)? ";
    YNRESPONDER;
  }
}

#undef YNRESPONDER

main_action request_action() {
  const static std::map<std::string, main_action, std::less<>> options{
      std::make_pair(".cs", main_action::CHAN_LIST),
      std::make_pair(".cc", main_action::NEW_CHAN),
      std::make_pair(".cp", main_action::CH_PASS),
      std::make_pair(".l", main_action::LOGOUT),
      std::make_pair(".q", main_action::QUIT)
  };

  std::cout << "This is the main menu." << std::endl
            << "  -> Use .cs to get a channel list," << std::endl
            << "  -> Use .cc to create a new channel, or" << std::endl
            << "  -> Use .l to log out." << std::endl;
  while(true) {
    std::string resp;
    std::cout << "What do you want to do? ";
    std::getline(std::cin, resp);
    if(options.contains(resp)) return options.at(resp);
    std::cout << "Unrecognized command. Please try again." << std::endl;
  }
}

chan_action request_chan_action() {
  const static std::map<std::string, chan_action, std::less<>> options{
    std::make_pair(".m", chan_action::GET_MSGS),
    std::make_pair(".s", chan_action::SEND_MSG),
    std::make_pair(".u", chan_action::GET_USRS),
    std::make_pair(".i", chan_action::INVITE_USR),
    std::make_pair(".b", chan_action::BACK),
    std::make_pair(".q", chan_action::QUIT)
  };

  std::cout << "Actions for this channel:" << std::endl
            << "  -> Use .m to get all messages in this channel," << std::endl
            << "  -> Use .s to send a message," << std::endl
            << "  -> Use .u to view the members of this channel," << std::endl
            << "  -> Use .i to invite another user here, or" << std::endl
            << "  -> Use .b to go back." << std::endl;

  while(true) {
    std::string resp;
    std::cout << "What do you want to do? ";
    std::getline(std::cin, resp);

    if(options.contains(resp)) return options.at(resp);
    std::cout << "Unrecognized command. Please try again." << std::endl;
  }
}

int32_t choose_user(cli &cli, int32_t token) {
  int32_t uid;
  bool inv_user = true;
  do {
    std::cout << "Enter user ID: ";
    std::cin >> uid;

    try {
      auto details = cli.send_user_details(token, uid);
      inv_user = !confirm_user(details.id, details.name);
    }
    catch(std::exception &) {
      std::cout << "Invalid user. Please try again." << std::endl;
    }
  } while(inv_user);
  return uid;
}

bool run_in_channel_menu(cli &cli, int32_t token, int32_t chan_id) {
  try {
    auto chan = cli.send_channel_details(token, chan_id);

    while(true) {
      std::cout << "You're now in " << chan.name << "(ID: " << chan.id << ")." << std::endl;
      chan_action act = request_chan_action();

      if(act == chan_action::GET_MSGS) {
        auto resp = cli.send_channel_message_list(token, chan_id);
        std::cout << "Messages in " << chan.name << ":" << std::endl;
        for(const auto &msg: resp.msgs) {
          std::cout << "  <User #" << msg.sender << "> at "
                    << format_timestamp(msg.when) << ": " << msg.cnt << std::endl;
        }
      }
      else if(act == chan_action::SEND_MSG) {
        cli.send_send_message(token, chan_id);
      }
      else if(act == chan_action::GET_USRS) {
        std::cout << "Users in " << chan.name << " (the owner has a * next to their name):" << std::endl;
        for(const auto &uid: chan.members) {
          auto user = cli.send_user_details(token, uid);
          std::cout << "  -> "
                    << (uid == chan.owner_id ? '*' : ' ') << "User #" << user.id
                    << ": " << user.name << std::endl;
        }
        std::cout << std::endl;
      }
      else if(act == chan_action::INVITE_USR) {
        int32_t uid = choose_user(cli, token);
        cli.send_user_invite(token, uid, chan_id);
      }
      else if(act == chan_action::BACK) {
        return false;
      }
      else if(act == chan_action::QUIT) {
        return true;
      }
    }
  }
  catch(cli::cli_error &err) {
    std::cout << "Something happened: " << err.what() << std::endl;
    return false;
  }
}

bool run_channel_menu(cli &cli, int32_t token) {
  try {
    while(true) {
      channel_list_response list = cli.send_channel_list(token);
      std::cout << "Channels available to you:" << std::endl;
      for(const auto &chan: list.data) {
        std::cout << "  -> " << chan.id << ": " << chan.name << std::endl;
      }
      std::cout << "Options: " << std::endl
                << "  -> Use .c to choose and act upon a channel," << std::endl
                << "  -> Use .b to go back, or" << std::endl
                << "  -> Use .r to refresh this list." << std::endl;

      while(true) {
        std::cout << "Your choice? ";

        std::string resp;
        std::getline(std::cin, resp);

        if(resp == ".c") {
          int32_t id;
          std::cout << "Channel ID? ";
          std::cin >> id;
          cli::flush();
          if(run_in_channel_menu(cli, token, id))
            return true;
          break; // break to outer loop
        }
        else if(resp == ".b") {
          return false;
        }
        else if(resp == ".r") {
          break; // break to outer loop
        }
        else if(resp == ".q") {
          return true;
        }

        std::cout << "Unrecognized command. Please try again." << std::endl;
      }
    }
  }
  catch(cli::cli_error &err) {
    std::cout << "Something happened: " << err.what() << std::endl;
    return false;
  }
}

void cli::run_wait_loop() {
  std::cout << "Welcome to dotchat client CLI tool." << std::endl
            << " -> Please prepend all commands with a dot (.)." << std::endl
            << "    Exception: yes/no questions (then please " << std::endl
            << "    answer with y or n)" << std::endl
            << " -> Whenever a command is requested, you can also" << std::endl
            << "    enter `.q` to exit." << std::endl << std::endl;
  while(true) {
    int32_t token;

    // step one: login or signup
    try {
      login_action step_one = request_login();

      if (step_one == login_action::QUIT) return;
      else if (step_one == login_action::LOGIN) token = send_login();
      else {
        send_new_user();
        if (login_after_signup())
          token = send_login();
        else
          continue;
      }
    }
    catch(cli::cli_error &err) {
      std::cout << "Something happened: " << err.what() << std::endl;
      return;
    }

    // step two: run actions
    while(true) {
      try {
        main_action act = request_action();

        if (act == main_action::LOGOUT) {
          send_logout(token);
          break;
        } else if (act == main_action::CHAN_LIST) {
          if(run_channel_menu(*this, token)) {
            return;
          }
        } else if(act == main_action::NEW_CHAN) {
          send_create_channel(token);
        } else if(act == main_action::CH_PASS) {
          send_change_pass(token);
        } else if(act == main_action::QUIT) {
          return;
        }
      }
      catch(cli::cli_error &err) {
        std::cout << "Something happened: " << err.what() << std::endl;
      }
    }
  }
}