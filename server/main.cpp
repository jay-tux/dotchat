#include <string>
#include "logger.hpp"
#include "tls/tls_server_socket.hpp"
#include "db/db.hpp"

// TODO (general): protocol implementation

using namespace dotchat::server;
using namespace dotchat::values;

const logger::log_source init { "MAIN", cyan };
const logger::log_source error { "ERROR", red };

void help(const char *invoker) {
  log << "Usage: " << invoker << " <private key PEM file> <certificate PEM file>" << endl;
}

int main(int argc, const char **argv) {
  log << reset;
  log << logger::banner_t<true>{} << endl;
  if(argc < 3 || (argc == 2 && std::string(argv[1]) == "-h")) {
    help(argv[0]);
    return 0;
  }

  log << init << "Starting server..." << endl;

  try {
    auto context = tls_context(std::string(argv[1]), std::string(argv[2]));
    auto socket = tls_server_socket(42069, context);
    log << init << "Waiting for connections..." << endl;
    // TODO: loop
    auto conn = socket.accept();
    conn << "Hellooooo!\n" << tls_connection::end_of_msg{};
    conn << "And hello again\n" << tls_connection::end_of_msg{};

    log << init << "Server shutting down..." << endl;
  }
  catch(const std::exception &exc) {
    log << error << red << "An error occurred:" << endl;
    log << error << red << exc.what() << endl;
  }
  return 0;
}
