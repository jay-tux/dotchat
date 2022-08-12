/////////////////////////////////////////////////////////////////////////////
// Name:        hexgrid.hpp
// Purpose:     Tool to write a series of bytes as hex/chars
// Author:      jay-tux
// Created:     July 19, 2022 4:27 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_CLIENT_HEXGRID_HPP
#define DOTCHAT_CLIENT_HEXGRID_HPP

#include <span>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <termcolor/termcolor.hpp>

namespace dotchat {
namespace _intl_ {
template <typename T>
concept char_like = sizeof(T) == sizeof(char);

template <typename T = char>
struct hex_int {
  T c;

  explicit hex_int(T c) : c{c} {}

  friend std::ostream &operator<<(std::ostream &target, const hex_int<T> &val) {
    return target << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<uint8_t>(val.c));
  }
};

template <typename T = char>
struct printable {
  T c;
  char alt;
  bool colors;

  printable(T c, char alt, bool colors) : c{c}, alt{alt}, colors{colors} {}

  friend std::ostream &operator<<(std::ostream &target, const printable<T> &val) {
    if(std::isgraph(static_cast<int>(static_cast<unsigned char>(val.c)))) return target << static_cast<unsigned char>(val.c);
    else if(val.c == static_cast<T>(' ')) return target << ' ';
    else return target << termcolor::red << val.alt;
  }
};
}

template <_intl_::char_like T = char>
struct hexgrid {
  struct config_data {
    explicit config_data() = default;

    bool enable_header = true;
    bool enable_sidebar = true;
    bool use_color = true;
    char nonprint_chars = '.';
  };

  explicit hexgrid(const std::span<T> &data) : data{data}, conf{config_data()} {}
  hexgrid(const std::span<T> &data, const config_data &conf) : data{data}, conf{conf} {}

  config_data &config() { return conf; }
  const config_data &config() const { return conf; }

  void print_to(std::ostream &target) const {
    if(conf.enable_header) {
      // header color: grey
      if(conf.use_color) target << termcolor::bright_grey;

      if(conf.enable_sidebar) {
        target << "    " /* 4: sidebar width, 2: spacing */ "  ";
      }

      for(uint8_t i = 0; i < 16; i++) {
        target << _intl_::hex_int{i} << "  ";
      }

      target << std::endl;
    }

    for(size_t row = 0; row * 16 < data.size(); row++) {
      if(conf.enable_sidebar) {
        // sidebar color: grey
        if(conf.use_color) target << termcolor::bright_grey;
        target << _intl_::hex_int{static_cast<uint8_t>(row)} << "  ";
      }

      // data color: white
      if(conf.use_color) target << termcolor::white;
      for(size_t col = 0; col < 16; col++) {
        if(row * 16 + col < data.size())
          target << _intl_::hex_int{data[row * 16 + col]} << "  ";
        else
          target << "    " /* 4: empty placeholder, 2: spacing */ "  ";
      }

      // separator color: cyan
      if(conf.use_color) target << termcolor::cyan;
      target << "|  ";

      // char color: blue
      for(size_t col = 0; col < 16; col++) {
        if(row * 16 + col < data.size())
          target << termcolor::blue << _intl_::printable{data[row * 16 + col], conf.nonprint_chars, conf.use_color};
        else
          target << " ";
      }

      // print newline
      target << std::endl;
    }

    target << termcolor::reset << std::dec << std::setw(0) << std::setfill(' ');
  }

  friend std::ostream &operator<<(std::ostream &target, const hexgrid<T> &src) {
    src.print_to(target);
    return target;
  }

  const std::span<T> &data;
  config_data conf = config();
};
}

#endif //DOTCHAT_CLIENT_HEXGRID_HPP
