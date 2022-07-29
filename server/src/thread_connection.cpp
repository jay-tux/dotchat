/////////////////////////////////////////////////////////////////////////////
// Name:        thread_connection.cpp
// Purpose:     Threaded worker for the server (impl)
// Author:      jay-tux
// Created:     July 20, 2022 6:45 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <tls/tls_error.hpp>
#include "logger.hpp"
#include "thread_connection.hpp"
#include "handle.hpp"

using namespace dotchat::server;
using namespace dotchat;
using namespace dotchat::values;
using namespace dotchat::tls;

const logger::log_source init{"THRD_CONN", green};
const logger::log_source error { "ERROR", red };

std::atomic<size_t> thread_conn::thread_id_next = 0;

void thread_conn::callback() {
  log << init << "Thread has been forked (ID: " << id << "). Starting conversation..." << endl;
  state = thread_state::RUNNING;

  try {
    while (conn && is_running()) {
      log << init << " -> reading from stream..." << endl;
      auto stream = conn.read();
      log << init << " -> stream size is " << stream.size() << endl;
      if (stream.size() == 0) {
        log << init << " -> empty stream; closing connection..." << endl;
        conn.close();
        state = thread_state::FINISHED;
      } else {
        log << init << " -> handling message..." << endl;
        auto res = handle(stream);
        log << init << " -> sending response..." << endl;
        conn << res.as_string() << tls_connection::end_of_msg{};

        if (state == thread_state::STOPPING) {
          conn.close();
          state = thread_state::STOPPED;
        }
      }
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

  log << init << "Thread (ID: " << id << ") finished." << endl;
}