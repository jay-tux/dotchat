/////////////////////////////////////////////////////////////////////////////
// Name:        dyn_size_array.hpp
// Purpose:     Dyanimic-sized array which can't be resized
// Author:      jay-tux
// Created:     July 30, 2022 11:44 AM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_DYNSIZE_ARRAY_HPP
#define DOTCHAT_DYNSIZE_ARRAY_HPP

#include <type_traits>

namespace dotchat {
namespace _intl_ {
template <typename T>
concept copy_assignable = std::is_copy_assignable_v<T>;

template <typename T>
concept move_assignable = std::is_move_assignable_v<T>;
}

template <typename T>
struct dyn_size_array {
  using size_t = std::size_t;
  explicit dyn_size_array(size_t size): buf{new T[size]}, size{size} {}
  
  dyn_size_array(const dyn_size_array &other) 
    requires(_intl_::copy_assignable<T>): size{other.size}, buf{new T[other.size]} {
    for(size_t i = 0; i < other.size; i++) buf[i] = other[i];
  }
  
  dyn_size_array(dyn_size_array &&other) noexcept 
    requires(_intl_::move_assignable<T>): size{other.size}, buf{other.buf} {
    other.buf = nullptr;
    other.size = 0;
  }
  
  dyn_size_array &operator=(const dyn_size_array &) = delete;
  dyn_size_array &operator=(dyn_size_array &&) = delete;
  
  T &operator[](size_t s) { return buf[s]; }
  const T &operator[](size_t s) const { return buf[s]; }

  struct iterator {
    size_t idx;
    dyn_size_array<T> &ref;
    T &operator *() { return ref[idx]; }
    iterator operator++() { return {idx + 1, ref}; }
  };

  iterator begin() { return { 0, *this }; }
  iterator end() { return { size, *this }; }
  
  ~dyn_size_array() {
    delete [] buf;
    size = 0;
    buf = nullptr;
  }
  
  T *buf;
  size_t size;
};
}

#endif //DOTCHAT_DYNSIZE_ARRAY_HPP
