/////////////////////////////////////////////////////////////////////////////
// Name:        thread_mgr.cpp
// Purpose:     Thread manager for the server
// Author:      jay-tux
// Created:     August 20, 2022 10:26 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "threading/thread_mgr.hpp"
#include "threading/thread_connection.hpp"
#include "tls/tls_connection.hpp"
#include <mutex>
#include <chrono>
#include <algorithm>

using namespace dotchat::tls;
using namespace dotchat::server;
using namespace std::chrono_literals;

thread_mgr &thread_mgr::manager() {
  static thread_mgr mgr;
  return mgr;
}

void thread_mgr::enlist(tls::tls_connection &&conn) {
  std::unique_lock lock { protector };
  threads.emplace_back(std::move(conn));
}

bool is_stopped(const thread_conn &c) {
  return (thread_state)c == thread_state::STOPPED || (thread_state)c == thread_state::FINISHED;
}

void actual_cleanup(thread_mgr::thread_set_t &threads, std::mutex &mutex) {
  std::unique_lock lock { mutex };
  threads.remove_if([](const auto &thread){
    return is_stopped(thread);
  });
}

void thread_mgr::cleanup(const std::stop_token &st) {
  while(!st.stop_requested()) {
    std::this_thread::sleep_for(delay * 1ms);
    actual_cleanup(threads, protector);
  }
}