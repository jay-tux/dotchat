/////////////////////////////////////////////////////////////////////////////
// Name:        db_row.hpp
// Purpose:     Helper struct to extract data from rows
// Author:      jay-tux
// Created:     July 06, 2022 12:12 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_DB_ROW_HPP
#define DOTCHAT_SERVER_DB_ROW_HPP

#include <concepts>
#include "sqlite3.h"
#include "db_error.hpp"

namespace dotchat::server {
namespace _intl_ {
template <typename T>
concept correct_row_type = std::same_as<T, int> || std::same_as<T, std::string>;
}

struct db_row {
  sqlite3_stmt *source;

  enum class col_type {
    COL_INT, COL_STRING
  };

  inline int column_count() const { return sqlite3_column_count(source); }
  inline col_type column_type(int col) const {
    switch(sqlite3_column_type(source, col)) {
      case SQLITE_INTEGER: return col_type::COL_INT;
      case SQLITE_TEXT: return col_type::COL_STRING;
      default: throw db_error(SQLITE_WRAPPED_INVALID_TYPE);
    }
  }

  inline int column_int(int col) const {
    if(sqlite3_column_type(source, col) != SQLITE_INTEGER)
      throw db_error(SQLITE_WRAPPED_INVALID_TYPE);
    return sqlite3_column_int(source, col);
  }

  inline std::string column_str(int col) const {
    if(sqlite3_column_type(source, col) != SQLITE_INTEGER)
      throw db_error(SQLITE_WRAPPED_INVALID_TYPE);
    return { (const char *)(sqlite3_column_text(source, col)) };
  }

  template <typename T1>
  T1 get(int col) requires _intl_::correct_row_type<T1> {
    if constexpr(std::is_same_v<T1, int>) { return column_int(col); }
    else return column_str(col);
  }
};
}

#endif //DOTCHAT_SERVER_DB_ROW_HPP
