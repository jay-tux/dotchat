#include <string>
#include <vector>
#include "tls/tls_server_socket.hpp"
#include "thread_connection.hpp"
#include "tls/tls_error.hpp"
#include "db/database.hpp"
#include <csignal>
#include <atomic>
#include <iostream>

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::server;

volatile std::sig_atomic_t flag = 0;
const static int milli_delay = 100;

void help(const char *invoker) {
  std::cerr << "Usage: " << invoker << " <private key PEM file> <certificate PEM file>" << std::endl;
}

extern "C" void sig_int(int sig) {
  std::cerr << std::endl << "Signal " << sig << " thrown... Shutting down server..." << std::endl;
  flag = 1;
}

int main(int argc, const char **argv) {
  if(argc < 3 || (argc == 2 && std::string(argv[1]) == "-h")) {
    help(argv[0]);
    return 0;
  }

  std::cerr << "Starting server..." << std::endl;
  std::cerr << "Setting up signal handler..." << std::endl;
  struct sigaction params{};
  params.sa_flags = 0;
  params.sa_handler = sig_int;
  sigemptyset(&params.sa_mask);
  if(sigaction(SIGINT, &params, nullptr) == -1) {
    std::cerr << "Failed to install signal handler... Continuing without handler..." << std::endl;
  }

  std::cerr << "Starting database service..." << std::endl;
  db::database();

  try {
    auto context = tls_context(std::string(argv[1]), std::string(argv[2]));
    auto socket = tls_server_socket(42069, context);
    std::vector<thread_conn> open; // TODO remove on join
    std::cerr << "Waiting for connections..." << std::endl;
    while(flag == 0) {
      if(auto is_ready = socket.accept_nonblock(milli_delay); is_ready.has_value())
        open.emplace_back(std::move(is_ready.value()));
    }
    std::cerr << "Detected shutdown request..." << std::endl;
    std::cerr << "  -> Closing " << open.size() << " running connection(s)/thread(s)..." << std::endl;
    size_t i = 0;
    for(auto &v: open) {
      v.request_stop(); // request politely
      std::cerr << "    -> Requested thread #" << i << " to stop..." << std::endl;
      i++;
    }
    i = 0;
    for(auto &v: open) {
      std::cerr << "    -> Waiting for thread #" << i << " to die..." << std::endl;
      v.stop_sync(); // kill.
      std::cerr << "    -> Thread #" << i << " died." << std::endl;
      i++;
    }
  }
  catch(const tls::tls_error &err) {
    std::cerr << "An error occurred:" << std::endl;
    std::cerr << "  " << err.what() << std::endl;
    std::cerr << "OpenSSL error queue: ";
    tls_context::dump_error_queue([](){ std::cerr << std::endl << "  "; }, std::cerr);
    std::cerr << std::endl;
  }
  catch(const std::exception &exc) {
    std::cerr << "An error occurred:" << std::endl;
    std::cerr << "  " << exc.what() << std::endl;
  }
  return 0;
}
