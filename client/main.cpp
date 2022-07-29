#include <string>
#include "logger.hpp"
#include "protocol/message.hpp"
#include "tls/tls_client_socket.hpp"

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::proto;
using namespace dotchat::values;

const logger::log_source init { "MAIN", cyan };
const logger::log_source error { "ERROR", red };

void help(const char *invoker) {
  log << "Usage: " << invoker << " <certificate PEM file> <IP address> <port number>" << endl;
}

std::string as_str(command_type c) {
  if(c == command_type::OK) return "OK";
  else if(c == command_type::ERR) return "ERR";
  else return "???";
}

void run_login(tls_connection &conn) {
  std::cout << "Username: ";
  std::string name;
  std::cin >> name;
  std::cout << "Password: ";
  std::string pass;
  std::cin >> pass;
  log << init << "Username: `" << name << "` (length " << name.size() << "); hex dump: ";
  for(const auto c : name) log << "0x" << std::hex << (int)c << " ";
  log << endl;
  log << init << "Password: `" << pass << "` (length " << pass.size() << "); hex dump: ";
  for(const auto c : pass) log << "0x" << std::hex << (int)c << " ";
  log << endl;

  message m1;
  m1.command = command_type::AUTH;
  m1.set_arg('u', name);
  m1.set_arg('p', pass);
  conn << m1.as_string() << tls_connection::end_of_msg{};

  message resp;
  bytestream data = conn.read();
  data >> resp;
  log << init << "Response:" << endl << " -> Command: " << as_str(resp.command) << endl;
  for(const auto &[k, v]: resp.args) {
    log << " -> " << k << ": ";
    if(resp.is_int(k)) log << resp.get_int(k) << " (type: int)";
    else log << resp.get_str(k) << " (type: string)";
    log << endl;
  }

  if(resp.command == command_type::OK) {
    log << init << "Login successful. Trying to log out now..." << endl;
    message m2;
    m2.command = command_type::EXIT;
    m2.set_arg('t', resp.get_int('t'));
    conn << m2.as_string() << tls_connection::end_of_msg{};
    data = conn.read();
    data >> resp;
    log << init << "Response: " << endl << " -> Command: " << as_str(resp.command) << endl;
    for(const auto &[k, v]: resp.args) {
      log << " -> " << k << ": ";
      if(resp.is_int(k)) log << resp.get_int(k);
      else log << resp.get_str(k);
      log << endl;
    }
  }
}

int main(int argc, const char **argv) {
  log << reset;
  log << logger::banner_t<false>{} << endl;
  if(argc < 4 || (argc == 2 && std::string(argv[1]) == "-h")) {
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
