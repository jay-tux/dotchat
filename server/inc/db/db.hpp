/////////////////////////////////////////////////////////////////////////////
// Name:        db.hpp
// Purpose:     Wrapper around SQLite
// Author:      jay-tux
// Created:     July 04, 2022 8:30 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_DB_HPP
#define DOTCHAT_SERVER_DB_HPP

#include <string>
#include "sqlite3.h"
#include "db_error.hpp"
#include "db_prepare.hpp"
#include "db_row.hpp"
#include "fpgen.hpp"

namespace dotchat::server {
class database {
public:
  database(const database &) = delete;
  database(database &&) = delete;

  database &operator=(const database &) = delete;
  database &operator=(database &&) = delete;

  static database &get();

  // TODO: run with read.
  template <typename ... Ts> fpgen::generator<db_row> run_read(const std::string &query, Ts... params) {
    const char *stmt = query.c_str();
    sqlite3_stmt *comp;
    if(int res = sqlite3_prepare_v2(db, stmt, -1, &comp, nullptr); !is_success(res)) {
      throw db_error(res);
    }
    comp = bind_all(comp, params...);
    int res;
    do {
      res = sqlite3_step(comp);
      if(res == SQLITE_ROW) {

      }
      else if(res != SQLITE_DONE) {
        throw db_error(res);
      }
    } while(res != SQLITE_DONE);
    sqlite3_finalize(comp);
  }

  template <typename ... Ts> void run_no_read(const std::string &query, Ts... params) {
    const char *stmt = query.c_str();
    sqlite3_stmt *comp;
    if(int res = sqlite3_prepare_v2(db, stmt, -1, &comp, nullptr); !is_success(res)) {
      throw db_error(res);
    }
    comp = bind_all(comp, params...);
    if(int res = sqlite3_step(comp); !is_success(res)) {
      throw db_error(res);
    }
    sqlite3_finalize(comp);
  }

  ~database();

private:
  enum class sqlite_state {
    ERROR, DONE, DATA, WORKING
  };

  database();

  inline bool is_success(int code) {
    return code == SQLITE_OK || code == SQLITE_ROW || code == SQLITE_DONE;
  }

  inline sqlite_state code_to_state(int code) {
    if(code == SQLITE_ROW) return sqlite_state::DATA;
    else if(is_success(code)) return sqlite_state::DONE;
    else if(code == SQLITE_BUSY) return sqlite_state::WORKING;
    else return sqlite_state::ERROR;
  }

  sqlite3 *db = nullptr;
};
}

#endif //DOTCHAT_SERVER_DB_HPP
