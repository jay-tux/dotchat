/////////////////////////////////////////////////////////////////////////////
// Name:        db_error.hpp
// Purpose:     std::exception subtype for database errors
// Author:      jay-tux
// Created:     July 06, 2022 11:55 AM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_DB_ERROR_HPP
#define DOTCHAT_SERVER_DB_ERROR_HPP

#ifndef SQLITE_WRAPPED_INVALID_TYPE
#define SQLITE_WRAPPED_INVALID_TYPE -1
#endif

#include <exception>
#include <string>

namespace dotchat::server {
struct db_error : std::exception {
  explicit db_error(int code);
  [[nodiscard]] inline const char * what() const noexcept override { return msg.c_str(); }
  std::string msg;
};
}

#endif //DOTCHAT_SERVER_DB_ERROR_HPP
