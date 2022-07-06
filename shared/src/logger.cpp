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

using namespace dotchat;

decltype(logger::start) logger::start;

logger &logger::get() noexcept {
  static logger l;
  logger::start = std::chrono::high_resolution_clock::now();
  return l;
}

std::string &logger::banner_server() noexcept {
  static std::string banner = R"BANNER(
        +=============================+
        |  Welcome to dotchat Server  |
        +=============================+
)BANNER";
  return banner;
}

std::string &logger::banner_client() noexcept {
  static std::string banner = R"BANNER(
        +=============================+
        |  Welcome to dotchat Client  |
        +=============================+
)BANNER";
  return banner;
}