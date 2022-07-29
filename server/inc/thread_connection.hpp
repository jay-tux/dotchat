/////////////////////////////////////////////////////////////////////////////
// Name:        thread_connection.hpp
// Purpose:     Threaded worker for the server
// Author:      jay-tux
// Created:     July 20, 2022 6:45 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_THREAD_CONNECTION_HPP
#define DOTCHAT_SERVER_THREAD_CONNECTION_HPP

#include <thread>
#include "tls/tls_connection.hpp"

namespace dotchat::server {
enum class thread_state {
  WAITING, RUNNING, STOPPING, FINISHED, STOPPED
};

class thread_conn {
public:
  inline explicit thread_conn(tls::tls_connection &&conn) : conn{std::move(conn)} {
    id = thread_id_next++;
  }
  thread_conn(const thread_conn &other) = delete;
  thread_conn(thread_conn &&other) = default;

  thread_conn &operator=(const thread_conn &other) = delete;
  thread_conn &operator=(thread_conn &&other) = default;

  [[nodiscard]] inline bool is_running() const {
    return state == thread_state::RUNNING || state == thread_state::STOPPING;
  }
  inline void request_stop() {
    if(is_running()) state = thread_state::STOPPING;
  }

  inline explicit operator bool() const {  return is_running(); }
  inline explicit operator thread_state() const {  return state; }

  inline void wait_for() {  if(is_running()) runner.join(); }

  inline void stop_sync() {
    
    request_stop();
    wait_for();
  }

  ~thread_conn() = default;

private:
  void callback();

  static std::atomic<size_t> thread_id_next;
  tls::tls_connection conn;
  std::jthread runner = std::jthread([this](){ this->callback(); });
  thread_state state = thread_state::WAITING;
  size_t id = 0;
};
}

#endif //DOTCHAT_SERVER_THREAD_CONNECTION_HPP
