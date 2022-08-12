#include <string>
#include <algorithm>
#include "logger.hpp"
#include "protocol/message.hpp"
#include "protocol/requests.hpp"
#include "protocol/helpers.hpp"
#include "tls/tls_client_socket.hpp"

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::proto;
using namespace dotchat::proto::requests;
using namespace dotchat::proto::responses;
using namespace dotchat::values;

const logger::log_source init { "MAIN", cyan };
const logger::log_source error { "ERROR", red };

void help(const char *invoker) {
  log << "Usage: " << invoker << " <certificate PEM file> <IP address> <port number>" << endl;
}

void run_logout(tls_connection &conn, int32_t token) {
  log << init << "Attempting logout..." << endl;
  bytestream strm;
  strm << logout_request{ { .token = token } }.to();
  conn.send(strm);

  strm = conn.read();
  dispatch<logout_response>(message(strm));
  log << init << "Logged out." << endl;
}

void run_get_channels(tls_connection &conn, int32_t token);

void run_get_messages(tls_connection &conn, int32_t token, int32_t channel) {
  while(true) {
    log << init << "Attempting to retrieve messages in channel #" << channel << endl;
    bytestream strm;
    strm << channel_msg_request{{.token = token}, channel}.to();
    conn.send(strm);

    strm = conn.read();
    from_message_convertible auto res = dispatch<channel_msg_response>(message(strm));
    log << init << bold << underline << "Messages in this channel:" << reset << endl;
    for (const auto &msg: res.msgs) {
      log << init << "<@" << msg.sender << " at " << msg.when << ">: " << msg.cnt << endl;
    }

    std::string reply;
    std::cout << "Send a reply (.q to exit, .c to choose a channel)? ";
    std::getline(std::cin, reply);

    if (reply == ".q") break;
    else if (reply == ".c") {
      run_get_channels(conn, token);
      break;
    } else {
      bytestream msg_strm;
      msg_strm << message_send_request{
          {.token = token}, channel, reply
      }.to();
      conn.send(msg_strm);

      msg_strm = conn.read();
      if (message(msg_strm).get_command() != response_commands::okay) {
        log << error << "Failed to send message!" << endl;
      }
    }
  }
}

void run_get_channels(tls_connection &conn, int32_t token) {
  log << init << "Attempting to retrieve channel list..." << endl;
  bytestream strm;
  strm << channel_list_request{ { .token = token } }.to();
  conn.send(strm);

  strm = conn.read();
  from_message_convertible auto res = dispatch<channel_list_response>(message(strm));
  std::vector<int32_t> channels;
  log << init << bold << underline << "Channel list:" << reset << endl;
  for(const auto &chan: res.data) {
    log << init << "  -> Channel #" << chan.id << ": " << chan.name << endl;
    channels.push_back(chan.id);
  }

  int32_t channel = -1;
  do {
    std::cout << "Choose channel ID: ";
    std::cin >> channel;
    if(std::find(channels.begin(), channels.end(), channel) == channels.end()) {
      log << init << "Invalid channel ID." << endl;
      channel = -1;
    }
  } while(channel == -1);
  run_get_messages(conn, token, channel);

  log << endl;
}

void run_login(tls_connection &conn) {
  std::cout << "Username: ";
  std::string name;
  std::cin >> name;
  std::cout << "Password: ";
  std::string pass;
  std::cin >> pass;

  bytestream strm;
  strm << login_request{ .user = name, .pass = pass }.to();
  conn.send(strm);

  strm = conn.read();
  from_message_convertible auto res = dispatch<login_response>(message(strm));
  log << init << "Token: " << res.token << endl;

  run_get_channels(conn, res.token);

  run_logout(conn, res.token);
  conn.close();
}

int main(int argc, const char **argv) {
  log << reset;
  log << logger::banner_t<false>{} << endl;
  if((argc == 2 && std::string(argv[1]) == "-h") || argc != 4) {
    help(argv[0]);
    return 0;
  }

  log << init << "Attempting to connect..." << endl;

  uint16_t portno = std::stoi(std::string(argv[3]));

  try {
    auto context = tls_context(std::string(argv[1]));
    auto socket = tls_client_socket(context);
    auto conn = socket.connect(std::string(argv[2]), portno);
    run_login(conn);
  }
  catch(const std::exception &exc) {
    log << error << red << "An error occurred:" << endl;
    log << error << red << "  " << exc.what() << endl;
  }
  return 0;
}
