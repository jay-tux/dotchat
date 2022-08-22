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
/**
 * \short Singleton class representing the thread pool manager.
 */
class thread_mgr {
public:
  /**
   * \short Type alias around a collection of connections on their own thread
   * (`std::list<dotchat::server::thread_conn>`).
   */
  using thread_set_t = std::list<thread_conn>;
  /**
   * \short The thread manager is a singleton, so it doesn't support copying.
   */
  thread_mgr(const thread_mgr &) = delete;
  /**
   * \short The thread manager is a singleton, so it doesn't support moving.
   */
  thread_mgr(thread_mgr &&) = delete;

  /**
   * \short The thread manager is a singleton, so it doesn't support copying.
   */
  thread_mgr &operator=(const thread_mgr &) = delete;
  /**
   * \short The thread manager is a singleton, so it doesn't support moving.
   */
  thread_mgr &operator=(thread_mgr &&) = delete;

  /**
   * \short Gets the instance.
   * \returns The singleton instance.
   */
  static thread_mgr &manager();
  /**
   * \short Adds a new connection to the set.
   * \param conn The `dotchat::tls::tls_connection` on which a new thread should be created.
   */
  void enlist(tls::tls_connection &&conn);

  /**
   * \short Gets an iterator to the beginning of the thread set.
   * \returns An iterator to the beginning of the thread set.
   */
  inline thread_set_t::iterator begin() { return threads.begin(); }
  /**
   * \short Gets an iterator to the end of the thread set.
   * \returns An iterator to the end of the thread set.
   */
  inline thread_set_t::iterator end() { return threads.end(); }

  /**
   * \short Cleans up all resources, including stopping any running threads.
   */
  ~thread_mgr() = default;

  /**
   * \short Gets or sets the delay for the cleanup-thread (in ms).
   * \returns A reference to the internal value.
   */
  inline size_t &cleanup_ms_delay() { return delay; };

private:
  /**
   * \short The thread manager is a singleton, so it doesn't support constructing.
   */
  thread_mgr() = default;
  /**
   * \short The internal cleanup function.
   */
  void cleanup(const std::stop_token &st);

  /**
   * \short A mutex to avoid race conditions between enlist/cleanup.
   */
  std::mutex protector;
  /**
   * \short The cleanup thread.
   */
  std::jthread cleaner = std::jthread([this](const std::stop_token &st){ this->cleanup(st); });
  /**
   * \short The thread set.
   */
  thread_set_t threads;
  /**
   * \short The delay for the cleanup thread (in ms).
   */
  size_t delay = 100;
};
}

#endif //DOTCHAT_SERVER_THREAD_MGR_HPP
