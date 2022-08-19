#include <string>
#include <algorithm>
#include "protocol/message.hpp"
#include "cli.hpp"
#include "tls/tls_client_socket.hpp"

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::proto;
using namespace dotchat::proto::requests;
using namespace dotchat::proto::responses;

void help(const char *invoker) {
  std::cerr << "Usage: " << invoker << " <certificate PEM file> <IP address> <port number>" << std::endl;
}

int main(int argc, const char **argv) {
  if((argc == 2 && std::string(argv[1]) == "-h") || argc != 4) {
    help(argv[0]);
    return 0;
  }

  std::cerr << "Attempting to connect..." << std::endl;

  uint16_t portno = std::stoi(std::string(argv[3]));

  try {
    auto context = tls_context(std::string(argv[1]));
    auto socket = tls_client_socket(context);
    auto conn = socket.connect(std::string(argv[2]), portno);
    client::cli sender{conn};
    sender();
  }
  catch(const std::exception &exc) {
    std::cerr << "An error occurred:" << std::endl;
    std::cerr << "  " << exc.what() << std::endl;
  }
  return 0;
}
