/////////////////////////////////////////////////////////////////////////////
// Name:        logger.hpp
// Purpose:     Simple but effective logger
// Author:      jay-tux
// Created:     June 29, 2022 8:19 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_LOGGER_HPP
#define DOTCHAT_SERVER_LOGGER_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <utility>
#include <type_traits>
#include <chrono>
#include "termcolor/termcolor.hpp"

namespace dotchat::server {
namespace _colors {
enum class color_e {
  BLUE, GREEN, RED, WHITE, CYAN, YELLOW, MAGENTA, GREY, RESET
};

template<color_e C>
struct color {
};

template<>
struct color<color_e::BLUE> {
  constexpr static const auto val = termcolor::blue;
};

template<>
struct color<color_e::GREEN> {
  constexpr static const auto val = termcolor::green;
};

template<>
struct color<color_e::RED> {
  constexpr static const auto val = termcolor::red;
};

template<>
struct color<color_e::WHITE> {
  constexpr static const auto val = termcolor::white;
};

template<>
struct color<color_e::CYAN> {
  constexpr static const auto val = termcolor::cyan;
};

template<>
struct color<color_e::YELLOW> {
  constexpr static const auto val = termcolor::yellow;
};

template<>
struct color<color_e::MAGENTA> {
  constexpr static const auto val = termcolor::magenta;
};

template<>
struct color<color_e::GREY> {
  constexpr static const auto val = termcolor::grey;
};

template<>
struct color<color_e::RESET> {
  constexpr static const auto val = termcolor::reset;
};
}
namespace _mods {
enum class modifier_e {
  BOLD, ITALIC, UNDERLINE, RESET
};

template<modifier_e M>
struct mod {
};

template<>
struct mod<modifier_e::BOLD> {
  constexpr static const auto val = termcolor::bold;
};

template<>
struct mod<modifier_e::ITALIC> {
  constexpr static const auto val = termcolor::italic;
};

template<>
struct mod<modifier_e::UNDERLINE> {
  constexpr static const auto val = termcolor::underline;
};

template<>
struct mod<modifier_e::RESET> {
  constexpr static const auto val = termcolor::reset;
};
}

struct logger {
  using color_e = _colors::color_e;
  template<color_e C> using color = _colors::color<C>;
  using modifier_e = _mods::modifier_e;
  template<modifier_e M> using mod = _mods::mod<M>;
  using time_t = std::invoke_result_t<decltype(std::chrono::high_resolution_clock::now)>;

  struct endl_t {
  };
  struct banner_t {
  };

  template<typename T, uint W>
  struct with_width {
    explicit with_width(T v) : value{std::move(v)} {}

    const T value;
  };

  template<color_e C>
  struct log_source {
    constexpr log_source(std::string name, color<C> col) noexcept: name{std::move(name)}, col{col} {}

    const std::string name;
    const color<C> col;
  };

  constexpr logger(const logger &) = delete;

  constexpr logger(logger &&) = delete;

  static logger &get() noexcept;

  static std::string &banner() noexcept;

  logger &operator=(const logger &) = delete;

  logger &operator=(logger &&) = delete;

  template<typename T>
  const logger &operator<<(const T &val) const {
    std::cerr << val;
    return *this;
  }

  const logger &operator<<(const banner_t &) const {
    std::cerr << banner();
    return *this;
  }

  const logger &operator<<(const endl_t &) const {
    std::cerr << std::endl;
    return *this;
  }

  template<typename T2, uint W>
  const logger &operator<<(const with_width<T2, W> &v) const {
    std::cerr << std::setw(W) << v.value;
    return *this;
  }

  template<color_e C>
  const logger &operator<<(const color<C> &) const {
    std::cerr << color<C>::val;
    return *this;
  }

  template<modifier_e M>
  const logger &operator<<(const mod<M> &) const {
    std::cerr << mod<M>::val;
    return *this;
  }

  template<color_e C>
  const logger &operator<<(const log_source<C> &source) const {
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();
    return (*this) << color<color_e::RESET>{} << "[" << mod<modifier_e::BOLD>{} << source.col
                   << with_width<std::string, 10>(source.name) << color<color_e::RESET>{} << " at "
                   << with_width<decltype(diff), 10>(diff) << "ms]: ";
  }

  ~logger() = default;

private:
  constexpr logger() noexcept = default;

  static time_t start;
};

}

namespace dotchat::values {
const static inline server::logger &log = server::logger::get();
const static inline server::logger::endl_t endl = {};
const static inline server::logger::color<server::logger::color_e::BLUE> blue = {};
const static inline server::logger::color<server::logger::color_e::GREEN> green = {};
const static inline server::logger::color<server::logger::color_e::RED> red = {};
const static inline server::logger::color<server::logger::color_e::WHITE> white = {};
const static inline server::logger::color<server::logger::color_e::CYAN> cyan = {};
const static inline server::logger::color<server::logger::color_e::YELLOW> yellow = {};
const static inline server::logger::color<server::logger::color_e::MAGENTA> magenta = {};
const static inline server::logger::color<server::logger::color_e::GREY> grey = {};
const static inline server::logger::color<server::logger::color_e::RESET> reset = {};

const static inline server::logger::mod<server::logger::modifier_e::BOLD> bold = {};
const static inline server::logger::mod<server::logger::modifier_e::ITALIC> italic = {};
const static inline server::logger::mod<server::logger::modifier_e::UNDERLINE> underline = {};
const static inline server::logger::mod<server::logger::modifier_e::RESET> reset_mod = {};
}

#endif //DOTCHAT_SERVER_LOGGER_HPP
