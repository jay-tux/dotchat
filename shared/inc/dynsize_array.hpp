/////////////////////////////////////////////////////////////////////////////
// Name:        dyn_size_array.hpp
// Purpose:     Dynamic-sized array which can't be resized
// Author:      jay-tux
// Created:     July 30, 2022 11:44 AM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short Dynamic-sized array which can't be resized.
 */

#ifndef DOTCHAT_DYNSIZE_ARRAY_HPP
#define DOTCHAT_DYNSIZE_ARRAY_HPP

#include <type_traits>

/**
 * \short Namespace containing general code for dotchat. Specific code is organized in nested namespaces.
 */
namespace dotchat {
/**
 * \short Namespace for internal helper code.
 */
namespace _intl_ {
/**
 * \short Concept version of the `std::is_copy_assignable` type trait.
 * \tparam T The type to check.
 */
template <typename T>
concept copy_assignable = std::is_copy_assignable_v<T>;

/**
 * \short Concept version of the `std::is_move_assignable` type trait.
 * \tparam T The type to check.
 */
template <typename T>
concept move_assignable = std::is_move_assignable_v<T>;
}

/**
 * \short Structure representing a dynamic-sized, constant-size, type safe array (constant-size vector).
 * \tparam T The type contained.
 */
template <std::default_initializable T>
struct dyn_size_array {
  /**
   * \short Type alias for the size type (`std::size_t`).
   */
  using size_t = std::size_t;

  /**
   * \short Creates a new dynamically-sized array of a certain size.
   * \param size The size of the array (in amount of elements).
   *
   * This constructor requires `std::default_initializable<T>`.
   */
  explicit dyn_size_array(size_t size) requires(std::default_initializable<T>): buf{new T[size]}, size{size} {}

  /**
   * \short Copies another dynamically sized array.
   * \param other The array to copy.
   *
   * This constructor requires `dotchat::_intl_::copy_assignable<T>`.
   */
  dyn_size_array(const dyn_size_array &other) 
    requires(_intl_::copy_assignable<T>): size{other.size}, buf{new T[other.size]} {
    for(size_t i = 0; i < other.size; i++) buf[i] = other[i];
  }

  /**
   * \short Steals the data from the other dynamically sized array.
   * \param other The array whose data to steal.
   */
  dyn_size_array(dyn_size_array &&other) noexcept: size{other.size}, buf{other.buf} {
    other.buf = nullptr;
    other.size = 0;
  }

  /**
   * \short Dynamically-sized arrays can't be copy-assigned.
   */
  dyn_size_array &operator=(const dyn_size_array &) = delete;
  /**
   * \short Dynamically-sized arrays can't be move-assigned.
   */
  dyn_size_array &operator=(dyn_size_array &&) = delete;

  /**
   * \short Accesses an element by index (without bounds checking).
   * \param s The index of the element to access.
   * \returns A reference to the requested element.
   */
  T &operator[](size_t s) { return buf[s]; }
  /**
   * \short Accesses an element by index (without bounds checking).
   * \param s The index of the element to access.
   * \returns A constant reference to the requested element.
   */
  const T &operator[](size_t s) const { return buf[s]; }

  /**
   * \short Structure representing an iterator over a dynamically-sized array.
   */
  struct iterator {
    /**
     * \short The index in the array.
     */
    size_t idx;
    /**
     * \short A reference to the array.
     */
    dyn_size_array<T> &ref;
    /**
     * \short Accesses the element pointed to by this iterator.
     * \returns A reference to the element this iterator points to.
     */
    T &operator *() { return ref[idx]; }
    /**
     * \short Advances the iterator by one position.
     * \returns An iterator to the next element in the sequence.
     */
    iterator operator++() { return {idx + 1, ref}; }
  };

  /**
   * \short Constructs an iterator to the beginning of the array.
   * \returns A new iterator to the beginning of the array.
   */
  iterator begin() { return { 0, *this }; }
  /**
   * \short Constructs an iterator past the end of the array.
   * \returns A new iterator past the end of the array.
   */
  iterator end() { return { size, *this }; }

  /**
   * \short Releases all memory occupied by the array.
   */
  ~dyn_size_array() {
    delete [] buf;
    size = 0;
    buf = nullptr;
  }

  /**
   * \short The actual internal buffer.
   */
  T *buf;
  /**
   * \short The size of the array.
   */
  size_t size;
};
}

#endif //DOTCHAT_DYNSIZE_ARRAY_HPP
