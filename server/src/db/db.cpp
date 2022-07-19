/////////////////////////////////////////////////////////////////////////////
// Name:        db.cpp
// Purpose:     Wrapper around SQLite (impl)
// Author:      jay-tux
// Created:     July 04, 2022 8:30 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include "db/db.hpp"
#include "logger.hpp"

#define DB_FILE "dotchat.sql_db"

using namespace dotchat;
using namespace dotchat::server;
using namespace dotchat::values;

const logger::log_source init { "DB", grey };
const char * const init_sql =
#include "dbinit.sql"
const std::array<const char *, 8> statement_desc = {
    "table: users",
    "table: db_keys",
    "table: channels",
    "table: db_channel_user",
    "table: messages",
    "db_user: master",
    "db_channel: general",
    "user_channel: master@general"
};
const int statement_count = 8;

database::database() {
  if(int res = sqlite3_open_v2(DB_FILE, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr); !is_success(res))
    throw db_error(res);
  log << init << "Database " DB_FILE " opened." << endl;
  const char *curr = init_sql;
  const char *nxt;
  sqlite3_stmt *comp;
  for(int i = 0; i < statement_count; i++) {
    log << init << "  -> running statement " << i << ": insert/create " << statement_desc[i] << endl;
    if (int res = sqlite3_prepare_v2(db, curr, -1, &comp, &nxt); !is_success(res))
      throw db_error(res);
    if(int res = sqlite3_step(comp); !is_success(res))
      throw db_error(res);
    sqlite3_finalize(comp);
    log << init << "  -> successfully ran statement." << endl;
  }
  log << init << "Database setup finished!" << endl;
}

database &database::get() {
  static database db;
  return db;
}

database::~database() {
  if(db != nullptr) sqlite3_close(db);
  db = nullptr;
}