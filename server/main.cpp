#include <string>
#include <vector>
#include "logger.hpp"
#include "tls/tls_server_socket.hpp"
#include "thread_connection.hpp"
#include "tls/tls_error.hpp"
#include "db/database.hpp"
#include <csignal>
#include <atomic>

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::server;
using namespace dotchat::values;

const logger::log_source init { "MAIN", cyan };
const logger::log_source error { "ERROR", red };

volatile std::sig_atomic_t flag = 0;
const static int milli_delay = 100;

void help(const char *invoker) {
  log << "Usage: " << invoker << " <private key PEM file> <certificate PEM file>" << endl;
}

extern "C" void sig_int(int sig) {
  log << endl << init << "Signal " << sig << " thrown... Shutting down server..." << endl;
  flag = 1;
}

int main(int argc, const char **argv) {
  log << reset;
  log << logger::banner_t<true>{} << endl;
  if(argc < 3 || (argc == 2 && std::string(argv[1]) == "-h")) {
    help(argv[0]);
    return 0;
  }

  log << init << "Starting server..." << endl;
  log << init << "Setting up signal handler..." << endl;
  struct sigaction params{};
  params.sa_flags = 0;
  params.sa_handler = sig_int;
  sigemptyset(&params.sa_mask);
  if(sigaction(SIGINT, &params, nullptr) == -1) {
    log << error << "Failed to install signal handler... Continuing without handler..." << endl;
  }

  log << init << "Starting database service..." << endl;
  db::database();

  try {
    auto context = tls_context(std::string(argv[1]), std::string(argv[2]));
    auto socket = tls_server_socket(42069, context);
    std::vector<thread_conn> open; // TODO remove on join
    log << init << "Waiting for connections..." << endl;
    while(flag == 0) {
      if(auto is_ready = socket.accept_nonblock(milli_delay); is_ready.has_value())
        open.emplace_back(std::move(is_ready.value()));
    }
    log << init << "Detected shutdown request..." << endl;
    log << init << "  -> Closing " << open.size() << " running connection(s)/thread(s)..." << endl;
    size_t i = 0;
    for(auto &v: open) {
      v.request_stop(); // request politely
      log << init << "    -> Requested thread #" << i << " to stop..." << endl;
      i++;
    }
    i = 0;
    for(auto &v: open) {
      log << init << "    -> Waiting for thread #" << i << " to die..." << endl;
      v.stop_sync(); // kill.
      log << init << "    -> Thread #" << i << " died." << endl;
      i++;
    }
  }
  catch(const tls::tls_error &err) {
    log << error << red << "An error occurred:" << endl;
    log << error << red << "  " << err.what() << endl;
    log << error << red << "OpenSSL error queue: ";
    tls_context::dump_error_queue([](){ log << endl << error << red << "  "; }, log);
    log << endl;
  }
  catch(const std::exception &exc) {
    log << error << red << "An error occurred:" << endl;
    log << error << red << "  " << exc.what() << endl;
  }
  return 0;
}
