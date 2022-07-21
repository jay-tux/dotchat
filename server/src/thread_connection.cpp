/////////////////////////////////////////////////////////////////////////////
// Name:        thread_connection.cpp
// Purpose:     Threaded worker for the server (impl)
// Author:      jay-tux
// Created:     July 20, 2022 6:45 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "logger.hpp"
#include "thread_connection.hpp"
#include "handle.hpp"

using namespace dotchat::server;
using namespace dotchat;
using namespace dotchat::values;
using namespace dotchat::tls;

const logger::log_source init{"THRD_CONN", green};

void thread_conn::callback() {
  log << init << "Thread has been forked. Starting conversation..." << endl;
  state = thread_state::RUNNING;

  while(conn && is_running()) {
    log << init << " -> reading from stream..." << endl;
    auto stream = conn.read();
    log << init << " -> stream size is " << stream.size() << endl;
    if(stream.size() == 0) {
      log << init << " -> empty stream; closing connection..." << endl;
      conn.close();
      state = thread_state::FINISHED;
    }
    else {
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