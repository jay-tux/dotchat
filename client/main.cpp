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

void run_login(tls_connection &conn) {
  std::cout << "Username: ";
  std::string name;
  std::cin >> name;
  std::cout << "Password: ";
  std::string pass;
  std::cin >> pass;

  bytestream strm;
  strm << message("login",
                  std::pair<std::string, std::string>{ "user", name },
                  std::pair<std::string, std::string>{ "pass", pass }
                  );
  conn.send(strm);

  strm = conn.read();
  if(message resp(strm); resp.get_command() == "ok") {
    log << init << "Attempting to extract token..." << endl;
    if(resp.map().contains("token") && resp.map().type("token") == dotchat::proto::_intl_::matching_enum<int32_t>::val) {
      int32_t token = resp.map().as<int32_t>("token");
      log << init << "Token: " << token << endl;
      log << init << "Attempting logout." << endl;
      bytestream logout;
      logout << message("logout",
                        std::pair<std::string, int32_t>{ "token", token }
                        );
      conn.send(logout);
      logout = conn.read();
      resp = message(logout);
      if(resp.get_command() == "ok") {
        log << init << "Success! Bye" << endl;
      }
      else {
        log << init << "Such a fail! They didn't log out :(" << endl;
      }
    }
    else {
      log << init << "Oops. Couldn't get token :(" << endl;
    }
  }
  else {
    log << init << "Got error response :(" << endl;
  }

  conn.close();
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
