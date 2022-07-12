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
    message msg;
    auto stream = conn.read();
    stream >> msg;
    log << init << "Received a message." << endl;

    log << init << "Client shutting down..." << endl;
  }
  catch(const std::exception &exc) {
    log << error << red << "An error occurred:" << endl;
    log << error << red << exc.what() << endl;
  }
  return 0;
}
