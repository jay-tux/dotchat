/////////////////////////////////////////////////////////////////////////////
// Name:        db_prepare.hpp
// Purpose:     Wrapper around sqlite3_bind
// Author:      jay-tux
// Created:     July 04, 2022 10:11 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_DB_PREPARE_HPP
#define DOTCHAT_SERVER_DB_PREPARE_HPP

#include <string>
#include "sqlite3.h"
#include "db_error.hpp"

namespace dotchat::server {
namespace _intl_ {
inline sqlite3_stmt *_actual_bind(sqlite3_stmt *p, int idx, int v) {
  if(int res = sqlite3_bind_int(p, idx, v); res != SQLITE_OK) {
    throw db_error(res);
  }
  return p;
}

inline sqlite3_stmt  *_actual_bind(sqlite3_stmt *p, int idx, const std::string &data) {
  sqlite3_bind_text(p, idx, data.c_str(), -1, SQLITE_TRANSIENT);
  return p;
}

template <typename T1, typename ... Rest> sqlite3_stmt *_run_binds(sqlite3_stmt *p, int idx, T1 v, Rest... rest) {
  if constexpr(sizeof...(Rest) == 0) return _actual_bind(p, idx, v);
  else return _run_binds(_intl_::_actual_bind(p, idx, v), idx + 1, rest...);
}
}

template <typename ... Ts>
sqlite3_stmt *bind_all(sqlite3_stmt *stmt, Ts... vs) {
  if constexpr(sizeof...(Ts) == 0) return stmt;
  else return _intl_::_run_binds(stmt, 1, vs...);
}
}

#endif //DOTCHAT_SERVER_DB_PREPARE_HPP
