/////////////////////////////////////////////////////////////////////////////
// Name:        logger.cpp
// Purpose:     Simple but effective logger (impl)
// Author:      jay-tux
// Created:     June 29, 2022 9:01 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include "logger.hpp"

using namespace dotchat::server;

decltype(logger::start) logger::start;

logger &logger::get() noexcept {
  static logger l;
  logger::start = std::chrono::high_resolution_clock::now();
  return l;
}

std::string &logger::banner() noexcept {
  static std::string banner = R"BANNER(
+=============================+
|  Welcome to dotchat Server  |
+=============================+
)BANNER";
  return banner;
}