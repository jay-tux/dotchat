/////////////////////////////////////////////////////////////////////////////
// Name:        thread_mgr.hpp
// Purpose:     Thread manager for the server
// Author:      jay-tux
// Created:     August 20, 2022 9:40 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short Thread manager for the server.
 */

#ifndef DOTCHAT_SERVER_THREAD_MGR_HPP
#define DOTCHAT_SERVER_THREAD_MGR_HPP

#include <list>
#include <queue>
#include <mutex>
#include "threading/thread_connection.hpp"
#include "tls/tls_connection.hpp"

/**
 * \short Namespace for all code related to the server.
 */
namespace dotchat::server {
class thread_mgr {
public:
  using thread_set_t = std::list<thread_conn>;
  thread_mgr(const thread_mgr &) = delete;
  thread_mgr(thread_mgr &&) = delete;

  thread_mgr &operator=(const thread_mgr &) = delete;
  thread_mgr &operator=(thread_mgr &&) = delete;

  static thread_mgr &manager();
  void enlist(tls::tls_connection &&conn);

  inline thread_set_t::iterator begin() { return threads.begin(); }
  inline thread_set_t::iterator end() { return threads.end(); }

  ~thread_mgr() = default;

  inline size_t &cleanup_ms_delay() { return delay; };

private:
  thread_mgr() = default;
  void cleanup(const std::stop_token &st);

  std::mutex protector;
  std::jthread cleaner = std::jthread([this](const std::stop_token &st){ this->cleanup(st); });
  thread_set_t threads;
  size_t delay = 100;
};
}

#endif //DOTCHAT_SERVER_THREAD_MGR_HPP
