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

/**
 * \short Namespace for all code related to the server.
 */
namespace dotchat::server {
/**
 * \short Enumeration with all threaded connection thread states.
 */
enum class thread_state {
  WAITING,    /*!< \short The thread is waiting to start. */
  RUNNING,    /*!< \short The thread is running. */
  STOPPING,   /*!< \short The thread is stopping on request. */
  FINISHED,   /*!< \short The thread has finished. */
  STOPPED     /*!< \short The thread has been stopped. */
};

/**
 * \short Class representing a connection running on a separate thread.
 */
class thread_conn {
public:
  /**
   * \short Constructs a new threaded connection from a normal connection.
   * \param conn The connection to work with.
   */
  inline explicit thread_conn(tls::tls_connection &&conn) : conn{std::move(conn)} {
    id = thread_id_next++;
  }
  /**
   * \short Threaded connections can't be copy-constructed.
   */
  thread_conn(const thread_conn &other) = delete;
  /**
   * \short Threaded connections can't be move-constructed.
   */
  thread_conn(thread_conn &&other) = default;

  /**
   * \short Threaded connections can't be copy-assigned.
   */
  thread_conn &operator=(const thread_conn &other) = delete;
  /**
   * \short Threaded connections can't be move-assigned.
   */
  thread_conn &operator=(thread_conn &&other) = default;

  /**
   * \short Returns whether the thread is running or not.
   * \returns True if the thread is running (states `RUNNING` and `STOPPING`), otherwise false.
   */
  [[nodiscard]] inline bool is_running() const {
    return state == thread_state::RUNNING || state == thread_state::STOPPING;
  }
  /**
   * \short Request the thread to stop running (asynchronously).
   */
  inline void request_stop() {
    if(is_running()) state = thread_state::STOPPING;
  }

  /**
   * \short Returns whether the thread is running or not.
   * \returns True if the thread is running (states `RUNNING` and `STOPPING`), otherwise false.
   */
  inline explicit operator bool() const {  return is_running(); }
  /**
   * \short Gets the thread's state.
   * \returns The thread's state.
   */
  inline explicit operator thread_state() const {  return state; }

  /**
   * \short Wait for the thread to finish running.
   */
  inline void wait_for() {  if(is_running()) runner.join(); }

  /**
   * \short Request the thread to stop running (synchronously).
   */
  inline void stop_sync() {
    request_stop();
    wait_for();
  }

  /**
   * \short Cleans up the threaded and frees all resources associated with it.
   */
  ~thread_conn() = default;

private:
  /**
   * \short The callback for the connection.
   */
  void callback();

  /**
   * \short The next thread ID to be given.
   */
  static std::atomic<size_t> thread_id_next;
  /**
   * \short The TLS connection this threaded connection is running on.
   */
  tls::tls_connection conn;
  /**
   * \short The actual internal thread (`std::jthread`).
   */
  std::jthread runner = std::jthread([this](){ this->callback(); });
  /**
   * \short The current state for this thread.
   */
  thread_state state = thread_state::WAITING;
  /**
   * \short This thread's ID.
   */
  size_t id = 0;
};
}

#endif //DOTCHAT_SERVER_THREAD_CONNECTION_HPP
