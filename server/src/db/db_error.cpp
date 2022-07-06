/////////////////////////////////////////////////////////////////////////////
// Name:        db_error.cpp
// Purpose:     std::exception subtype for database errors (impl)
// Author:      jay-tux
// Created:     July 06, 2022 11:57 AM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "db/db_error.hpp"
#include "sqlite3.h"

using namespace dotchat::server;

const char *err_str(int code) {
#define X(s) case s: return #s;
#include "sqlite_error.macro"
  switch(code) {
    case -1: return "Invalid data type.";
    SQLITE_ERRORS
    default: return "UNKNOWN ERROR";
  }
}

db_error::db_error(int code) : msg{"SQLite database error: " + std::string(err_str(code))} {}
