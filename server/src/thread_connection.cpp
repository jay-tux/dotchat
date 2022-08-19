/////////////////////////////////////////////////////////////////////////////
// Name:        thread_connection.cpp
// Purpose:     Threaded worker for the server (impl)
// Author:      jay-tux
// Created:     July 20, 2022 6:45 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <tls/tls_error.hpp>
#include <iostream>
#include "thread_connection.hpp"
#include "handle.hpp"

using namespace dotchat::server;
using namespace dotchat;
using namespace dotchat::tls;

std::atomic<size_t> thread_conn::thread_id_next = 0;

void thread_conn::callback() {
  state = thread_state::RUNNING;

  try {
    while (conn && is_running()) {
      auto stream = conn.read();
      if (stream.size() == 0) {
        conn.close();
        state = thread_state::FINISHED;
      } else {
        auto res = handle(stream);
        bytestream strm;
        strm << res;
        conn.send(strm);

        if (state == thread_state::STOPPING) {
          conn.close();
          state = thread_state::STOPPED;
        }
      }
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
    conn.close();
  }
}