#include <string>
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

void run_get_channels(tls_connection &conn, int32_t token) {
  log << init << "Attempting to retrieve channel list..." << endl;
  bytestream strm;
  strm << channel_list_request{ { .token = token } }.to();
  conn.send(strm);

  strm = conn.read();
  from_message_convertible auto res = dispatch<channel_list_response>(message(strm));
  for(const auto &chan: res.data) {
    log << init << "  -> Channel #" << chan.id << ": " << chan.name << endl;
  }
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
