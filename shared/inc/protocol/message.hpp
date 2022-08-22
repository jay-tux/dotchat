/////////////////////////////////////////////////////////////////////////////
// Name:        message.hpp
// Purpose:     Message parser and struct for the protocol
// Author:      jay-tux
// Created:     July 06, 2022 3:14 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

/**
 * \file
 * \short Message parser and struct for the protocol.
 */

#ifndef DOTCHAT_CLIENT_MESSAGE_HPP
#define DOTCHAT_CLIENT_MESSAGE_HPP

#include <map>
#include <string>
#include <concepts>
#include <any>
#include <utility>
#include <stdexcept>
#include "../tls/tls_bytestream.hpp"

/**
 * \short Namespace containing all code related to the dotchat protocol.
 */
namespace dotchat::proto {
/**
 * \short Namespace for internal helper code.
 */
namespace _intl_ {
/**
 * \short Enumeration of all valid value types.
 */
enum class val_types: int8_t {
  INT8 = 0x01,        /*!< Represents an 8-bit signed integer. */
  INT16 = 0x02,       /*!< Represents a 16-bit signed integer. */
  INT32 = 0x03,       /*!< Represents a 32-bit signed integer. */
  UINT8 = 0x11,       /*!< Represents an 8-bit unsigned integer. */
  UINT16 = 0x12,      /*!< Represents a 16-bit unsigned integer. */
  UINT32 = 0x13,      /*!< Represents a 32-bit unsigned integer. */
  CHAR = 0x21,        /*!< Represents a single character. */
  STRING = 0x22,      /*!< Represents a character string (sent with length). */
  SUB_OBJECT = 0x31,  /*!< Represents a sub-object (recursively). */
  LIST = 0x41         /*!< Represents a list of a certain type (type of the list is sent as well). */
};

class arg;
class arg_list;
class arg_obj;

/**
 * \short If `V` is a valid member of `dotchat::proto::_intl_::val_types`, provides a member typedef `type` representing
 * the type `V` represents.
 * \tparam V The enumeration value to convert to a type.
 */
template <val_types V>
struct matching_type {};

/// \short Specialization for 8-bit signed integers.
template <> struct matching_type<val_types::INT8> { using type = int8_t; };
/// \short Specialization for 16-bit signed integers.
template <> struct matching_type<val_types::INT16> { using type = int16_t; };
/// \short Specialization for 32-bit signed integers.
template <> struct matching_type<val_types::INT32> { using type = int32_t; };
/// \short Specialization for 8-bit unsigned integers.
template <> struct matching_type<val_types::UINT8> { using type = uint8_t; };
/// \short Specialization for 16-bit unsigned integers.
template <> struct matching_type<val_types::UINT16> { using type = uint16_t; };
/// \short Specialization for 32-bit unsigned integers.
template <> struct matching_type<val_types::UINT32> { using type = uint32_t; };
/// \short Specialization for characters.
template <> struct matching_type<val_types::CHAR> { using type = char; };
/// \short Specialization for character strings.
template <> struct matching_type<val_types::STRING> { using type = std::string; };
/// \short Specialization for sub-objects.
template <> struct matching_type<val_types::SUB_OBJECT> { using type = arg_obj; };
/// \short Specialization for lists.
template <> struct matching_type<val_types::LIST> { using type = arg_list; };

/**
 * \short Shorthand for `dotchat::proto::_intl_::matching_type<V>::type`.
 * \tparam V The enumeration value to convert to a type.
 */
template <val_types V>
using matching_type_t = typename matching_type<V>::type;

/**
 * \short If `T` is a representable type, provides a const, static member `dotchat::proto::_intl_::val_types val`
 * representing the corresponding enumeration value.
 * \tparam T The type to convert to an enumeration value.
 */
template <typename T>
struct matching_enum {};
/// \short Specialization for 8-bit signed integers.
template <> struct matching_enum<int8_t> { const static val_types val = val_types::INT8; };
/// \short Specialization for 16-bit signed integers.
template <> struct matching_enum<int16_t> { const static val_types val = val_types::INT16; };
/// \short Specialization for 32-bit signed integers.
template <> struct matching_enum<int32_t> { const static val_types val = val_types::INT32; };
/// \short Specialization for 8-bit unsigned integers.
template <> struct matching_enum<uint8_t> { const static val_types val = val_types::UINT8; };
/// \short Specialization for 16-bit unsigned integers.
template <> struct matching_enum<uint16_t> { const static val_types val = val_types::UINT16; };
/// \short Specialization for 32-bit unsigned integers.
template <> struct matching_enum<uint32_t> { const static val_types val = val_types::UINT32; };
/// \short Specialization for characters.
template <> struct matching_enum<char> { const static val_types val = val_types::CHAR; };
/// \short Specialization for character strings.
template <> struct matching_enum<std::string> { const static val_types val = val_types::STRING; };
/// \short Specialization for sub-objects.
template <> struct matching_enum<arg_obj> { const static val_types val = val_types::SUB_OBJECT; };
/// \short Specialization for lists.
template <> struct matching_enum<arg_list> { const static val_types val = val_types::LIST; };

/**
 * \short Type trait to decide whether a type is in a given set of types.
 * \tparam T The type to check.
 * \tparam Ts The set of types.
 *
 * Provides a const, static member `bool val`; which is true if `std::same_as<T, T1>` is true for any `T1` in `Ts...`.
 */
template <typename T, typename ... Ts> struct one_of;
/// \short Recursive case.
template <typename T, typename T1, typename ... Ts>
struct one_of<T, T1, Ts...> {
  const static bool val = std::same_as<T, T1> || one_of<T, Ts...>::val;
};
/// \short Base case.
template <typename T, typename T1>
struct one_of<T, T1> {
  const static bool val = std::same_as<T, T1>;
};

/**
 * \short Concept relaying the meaning of a trivially representable value.
 * \tparam T The type to check.
 *
 * Trivially representable types are 8-, 16- and 32-bit signed and unsigned integers, characters and strings.
 */
template <typename T>
concept is_trivially_repr = one_of<
    T,
    matching_type_t<val_types::INT8>, matching_type_t<val_types::INT16>,
    matching_type_t<val_types::INT32>, matching_type_t<val_types::UINT8>,
    matching_type_t<val_types::UINT16>, matching_type_t<val_types::UINT32>,
    matching_type_t<val_types::CHAR>, matching_type_t<val_types::STRING>
>::val;

/**
 * \short Class representing a single argument value (a wrapper around `std::any`).
 */
class arg {
public:
  /**
   * \short Constructs a new, default argument value. It represents an 8-bit integer set to 0.
   */
  constexpr arg() = default;

  /**
   * \short Converts a `dotchat::proto::_intl_::is_trivially_repr` value to an argument value.
   * \tparam T The type of the value to convert.
   * \param val T The value to convert.
   */
  template <is_trivially_repr T>
  explicit inline arg(const T &val) : _type{matching_enum<T>::val}, _content{val} {}
  /**
   * \short Converts a `dotchat::proto::_intl_::arg_list` to an argument value.
   * \param The list to convert.
   */
  explicit arg(const arg_list &val);
  /**
   * \short Converts a `dotchat::proto::_intl_::arg_obj` to an argument value.
   * \param The object to convert.
   */
  explicit arg(const arg_obj &val);

  /**
   * \short Gets the type currently contained in the argument value.
   * \returns The type currently contained in the argument value.
   */
  [[nodiscard]] inline val_types type() const { return _type; }

  /**
   * \short Attempts to cast the contained value to the type matching `T`.
   * \tparam T The `dotchat::proto::_intl_::val_types` enumeration value representing the requested type.
   * \returns The casted value.
   * \throws `std::bad_any_cast` if the contained value is not of the requested type.
   */
  template <val_types T>
  inline matching_type_t<T> get() const { return std::any_cast<matching_type_t<T>>(_content); }

  /**
   * \short Assigns a new `dotchat::proto::_intl_::is_trivially_repr` value to this argument value, erasing all
   * information about the previous value.
   * \tparam T The type of value to assign.
   * \param val The value to assign.
   * \returns A reference to the updated argument value.
   */
  template <is_trivially_repr T>
  arg &operator=(const T &val) {
    _type = matching_enum<T>::val;
    _content.reset();
    _content = val;
    return *this;
  }

  /**
   * \short Assigns a new `dotchat::proto::_intl_::arg_list` to this argument value, erasing all information about the
   * previous value.
   * \param l The list to assign.
   * \returns A reference to the updated argument value.
   */
  arg &operator=(const arg_list &l);

  /**
   * \short Assigns a new `dotchat::proto::_intl_::arg_obj` to this argument value, erasing all information about the
   * previous value.
   * \param lo The object to assign.
   * \returns A reference to the updated argument value.
   */
  arg &operator=(const arg_obj &lo);

  /**
   * \short Attempts to cast the contained value to the type matching `T`.
   * \tparam T The `dotchat::proto::_intl_::val_types` enumeration value representing the requested type.
   * \returns The casted value.
   * \throws `std::bad_any_cast` if the contained value is not of the requested type.
   */
  template <is_trivially_repr T>
  explicit operator T() const {
    if(_type == matching_enum<T>::val) return std::any_cast<T>(_content);
    else throw std::bad_any_cast();
  }

  /**
   * \short Attempts to cast the contained value to a `dotchat::proto::_intl_::arg_list`.
   * \returns The casted value.
   * \throws `std::bad_any_cast` if the contained value is not a list.
   */
  explicit operator arg_list() const;

  /**
   * \short Attempts to cast the contained value to a `dotchat::proto::_intl_::arg_obj`.
   * \returns The casted value.
   * \throws `std::bad_any_cast` if the contained value is not a sub-object.
   */
  explicit operator arg_obj() const;

private:
  /**
   * \short The type contained in this argument value.
   */
  val_types _type = val_types::INT8;
  /**
   * \short The actual value contained in this argument value.
   */
  std::any _content = (int8_t)0;
};

/**
 * \short Class representing a list of argument values (wrapper around `std::vector<dotchat::proto::_intl_::arg>`).
 */
class arg_list {
public:
  /**
   * \short Gets the size of the list.
   * \returns The amount of elements contained in the list.
   */
  [[nodiscard]] inline size_t size() const { return _content.size(); }
  /**
   * \short Gets the type of the elements contained in the list.
   * \returns The type of the elements contained in the list.
   */
  [[nodiscard]] inline val_types type() const { return _contained; }
  /**
   * \short Accesses the `n`-th element from the list (as an argument value).
   * \param n The index of the element.
   * \returns A reference to the argument value at index `n`.
   */
  inline arg &operator[](size_t n) { return _content[n]; }
  /**
   * \short Accesses the `n`-th element from the list (as an argument value).
   * \param n The index of the element.
   * \returns A constant reference to the argument value at index `n`.
   */
  inline const arg &operator[](size_t n) const { return _content[n]; }

  /**
   * \short Accesses the `n`-th element from the list (as its actual value).
   * \tparam T The type of the value to request (as `dotchat::proto::_intl_::val_types` value).
   * \param n The index of the element.
   * \returns A reference to the argument value at index `n`, cast to the requested type.
   * \throws `std::bad_any_cast` if the contained values are not of the requested type.
   */
  template <typename T>
  inline T &get_as(size_t n) {
    if(_contained == matching_enum<T>::val)
      return *std::any_cast<T *>(&_content[n]);
    else
      throw std::bad_any_cast();
  }

  /**
   * \short Accesses the `n`-th element from the list (as a sub-list).
   * \param n The index of the element.
   * \returns A reference to the argument value at index `n`, cast to a `dotchat::proto::_intl_::arg_list`.
   * \throws `std::bad_any_cast` if the contained values are not sub-lists.
   */
  arg_list &get_list(size_t n);
  /**
   * \short Accesses the `n`-th element from the list (as an object).
   * \param n The index of the element.
   * \returns A reference to the argument value at index `n`, cast to a `dotchat::proto::_intl_::arg_obj`.
   * \throws `std::bad_any_cast` if the contained values are not objects.
   */
  arg_obj &get_obj(size_t n);

  /**
   * \short Access the `n`-th element from the list (as a `dotchat::proto::_intl_::is_trivially_repr` value, a sublist
   * or an object).
   * \tparam T The requested type; must match with either `dotchat::proto::_intl_::is_trivially_repr<T>`,
   * `std::same_as<T, dotchat::proto::_intl_::arg_list>` or `std::same_as<T, dotchat::proto::_intl_::arg_obj>`.
   * \param n The index of the element.
   * \returns A reference of the requested type to the element at index `n`.
   * \throws `std::bad_any_cast` if the contained values do not match `T`.
   */
  template <typename T>
  inline T &operator[](size_t n) {
    if constexpr(is_trivially_repr<T>)
      return get_as<T>(n);
    else if constexpr(std::same_as<T, arg_list>)
      return get_list(n);
    else if constexpr(std::same_as<T, arg_obj>)
      return get_obj(n);
    else
      throw std::bad_any_cast();
  }

  /**
   * \short Accesses the `n`-th element from the list (as its actual value).
   * \tparam T The type of the value to request (as `dotchat::proto::_intl_::val_types` value).
   * \param n The index of the element.
   * \returns A const reference to the argument value at index `n`, cast to the requested type.
   * \throws `std::bad_any_cast` if the contained values are not of the requested type.
   */
  template <typename T>
  inline const T &get_as(size_t n) const {
    if(_contained == matching_enum<T>::val)
      return std::any_cast<T>(_content[n]);
    else
      throw std::bad_any_cast();
  }

  /**
   * \short Accesses the `n`-th element from the list (as a sub-list).
   * \param n The index of the element.
   * \returns A const reference to the argument value at index `n`, cast to a `dotchat::proto::_intl_::arg_list`.
   * \throws `std::bad_any_cast` if the contained values are not sub-lists.
   */
  [[nodiscard]] const arg_list &get_list(size_t n) const;

  /**
   * \short Accesses the `n`-th element from the list (as an object).
   * \param n The index of the element.
   * \returns A const reference to the argument value at index `n`, cast to a `dotchat::proto::_intl_::arg_obj`.
   * \throws `std::bad_any_cast` if the contained values are not objects.
   */
  [[nodiscard]] const arg_obj &get_obj(size_t n) const;

  /**
   * \short Access the `n`-th element from the list (as a `dotchat::proto::_intl_::is_trivially_repr` value, a sublist
   * or an object).
   * \tparam T The requested type; must match with either `dotchat::proto::_intl_::is_trivially_repr<T>`,
   * `std::same_as<T, dotchat::proto::_intl_::arg_list>` or `std::same_as<T, dotchat::proto::_intl_::arg_obj>`.
   * \param n The index of the element.
   * \returns A reference of the requested type to the element at index `n`.
   * \throws `std::bad_any_cast` if the contained values do not match `T`.
   */
  template <typename T>
  inline const T &operator[](size_t n) const {
    if constexpr(is_trivially_repr<T>)
      return get_as<T>(n);
    else if constexpr(std::same_as<T, arg_list>)
      return get_list(n);
    else if constexpr(std::same_as<T, arg_obj>)
      return get_obj(n);
    else
      throw std::bad_any_cast();
  }

  /**
   * \short If this list holds elements of type `T`, adds the given element to the end.
   * \tparam T The type of the value to add; should satisfy `dotchat::proto::_intl_::is_trivially_repr<T>`.
   * \param val The value to add.
   * \throws `std::bad_any_cast` if the contained values do not match `T`.
   */
  template <is_trivially_repr T>
  inline void push_back(const T &val) {
    if(_content.empty())
      _contained = matching_enum<T>::val;
    if(_contained == matching_enum<T>::val)
      _content.emplace_back(val);
    else
      throw std::bad_any_cast();
  }

  /**
   * \short If this list holds sublists, adds the given sublist to the end.
   * \param val The sublist to add.
   * \throws `std::bad_any_cast` if the contained values are not sublists.
   */
  void push_back(const arg_list &val);

  /**
   * \short If this list holds sub-objects, adds the given sub-object to the end.
   * \param val The sub-object to add.
   * \throws `std::bad_any_cast` if the contained values are not sub-objects.
   */
  void push_back(const arg_obj &val);

  /**
   * \short If the type of the elements of this list matches the type of the argument value, adds it to the end.
   * \param val The argument value.
   * \throws `std::bad_any_cast` if the contained values do not match.
   */
  inline void push_back(const arg &arg) {
    if(_content.empty()) _contained = arg.type();
    if(arg.type() != _contained) throw std::bad_any_cast();
    _content.push_back(arg);
  }

  template <typename T> struct iterable;

  /**
   * \short Type-safe iterator over an argument value list.
   * \tparam T The type contained in the list on which this iterator operates.
   */
  template <typename T>
  struct iterator {
  private:
    /**
     * \short Constructs a new iterator from and index and a source vector.
     * \param idx The index to point at.
     * \param source The source vector.
     */
    iterator(size_t idx, std::vector<arg> &source) : idx{idx}, source{source} {}

    /**
     * \short The index this iterator points at.
     */
    size_t idx;
    /**
     * \short A reference to the source vector.
     */
    std::vector<arg> &source;
  public:
    /**
     * \short Dereferences the iterator, returning the value it points to.
     * \returns A reference to the value this iterator points to, cast to `T &`.
     */
    T &operator*() { return source[idx]; }
    /**
     * \short Creates a copy of this iterator, and moves it to the next value.
     * \returns A new iterator, pointing to the value with an index 1 higher than this iterator.
     */
    iterator<T> operator++() { return { idx + 1, source }; }
    /**
     * \short Compares this iterator against another for inequality.
     * \param other The iterator to compare to.
     * \returns True if both iterators point to a different element; otherwise false.
     */
    bool operator!=(const iterator &other) const {
      return &source != &(other.source) || idx != other.idx;
    }

    friend iterable<T>;
  };

  /**
   * \short A constant, non-modifiable iterator over an argument value list.
   *
   * This struct is a wrapper around `std::vector<arg>::const_iterator`.
   */
  struct const_iterator {
    /**
     * \short Dereferences this iterator, returning the argument value it points to.
     * \returns A copy of the argument value this iterator points to.
     */
    inline arg operator*() const { return *it; }
    /**
     * \short Increments this iterator, returning a new iterator pointing to the next element.
     * \returns A new iterator to the next element in the sequence.
     */
    inline const_iterator operator++() { return { ++it }; }
    /**
     * \short Compares two iterators for inequality.
     * \returns True if both iterators point to different elements, otherwise false.
     */
    inline bool operator!=(const const_iterator &other) const {
      return it != other.it;
    }

    /**
     * The wrapped iterator.
     */
    std::vector<arg>::const_iterator it;
  };

  /**
   * \short A type-safe, iterable representation of an argument value list.
   * \tparam T The type of the values contained.
   */
  template <typename T>
  struct iterable {
  private:
    /**
     * \short Wraps the data source of an argument value list in a `dotchat::proto::_intl_::arg_list::iterable<T>`.
     * \param source The data source to wrap.
     */
    explicit iterable(std::vector<arg> &source) : source{source} {}

    /**
     * A reference to the original data source.
     */
    std::vector<arg> &source;
  public:
    /**
     * \short Constructs a new type-safe iterator to the first element in the data source.
     * \returns A begin-iterator for the data source.
     */
    iterator<T> begin() { return { 0, source }; }
    /**
     * \short Constructs a new type-safe iterator past the last element in the data source.
     * \returns An end-iterator for the data source.
     */
    iterator<T> end() { return { source.size(), source }; }

  friend arg_list;
  };

  /**
   * \short Attempts to wrap this argument value list in a `dotchat::proto::_intl_::arg_list::iterable<T>`.
   * \tparam T The type for the iterator; should satisfy `dotchat::proto::_intl_::is_trivially_repr<T>`.
   * \returns A type-safe iterable object over this argument value list.
   * \throws `std::bad_any_cast` if `T` doesn't match the contained type.
   */
  template <is_trivially_repr T>
  iterable<T> iterable_for() {
    if(_contained == matching_enum<T>::val)
      return arg_list::iterable<arg_list>{ _content };
    else
      throw std::bad_any_cast();
  }
  /**
   * \short Attempts to wrap this argument value list in a type-safe iterable.
   * \returns A type-safe iterable object over this argument value list; of type `dotchat::proto::_intl_::arg_list`.
   * \throws `std::bad_any_cast` if the contained type are not sublists.
   */
  iterable<arg_list> iterable_for_sublist();
  /**
   * \short Attempts to wrap this argument value list in a type-safe iterable.
   * \returns A type-safe iterable object over this argument value list; of type `dotchat::proto::_intl_::arg_obj`.
   * \throws `std::bad_any_cast` if the contained type are not sub-objects.
   */
  iterable<arg_obj> iterable_for_sub_obj();

  /**
   * \short Attempts to wrap this argument value list in a type-safe iterable. Wrapper function for `iterable_for`,
   * `iterable_for_sublist` and `iterable_for_sub_obj`.
   * \tparam T The type for the iterator; should either satisfy `dotchat::proto::_intl_::is_trivially_repr<T>`,
   * `std::same_as<T, dotchat::proto::_intl_::arg_list>`, or `std::same_as<T, dotchat::proto::_intl_::arg_obj>`.
   * \returns A type-safe iterable object over the argument value list.
   * \throws `std::bad_any_cost` if the contained type does not match `T`.
   */
  template <typename T>
  iterable<T> as_iterable() {
    if constexpr(is_trivially_repr<T>)
      return iterable_for<T>();
    else if constexpr(std::same_as<T, arg_list>)
      return iterable_for_sublist();
    else if constexpr(std::same_as<T, arg_obj>)
      return iterable_for_sub_obj();
    else
      throw std::bad_any_cast();
  }

  /**
   * \short Constructs an iterator to the beginning of the vector.
   * \return A const_iterator to the beginning of the underlying vector.
   */
  [[nodiscard]] inline const_iterator begin() const { return { _content.cbegin() }; }
  /**
   * \short Constructs an iterator past the end of the vector.
   * \return A const_iterator past the end of the underlying vector.
   */
  [[nodiscard]] inline const_iterator end() const { return { _content.cend() }; }

private:
  /**
   * \short The type of values contained in this list.
   */
  val_types _contained = arg().type();
  /**
   * \short The content of the list, as argument values.
   */
  std::vector<arg> _content;
};

/**
 * \brief Class representing a set of key-value-pairs where each key is a `std::string` and each value is an argument
 * value.
 *
 * Wrapper around an `std::map<std::string, dotchat::proto::_intl_::arg>, std::less<>>`.
 */
class arg_obj {
public:
  /**
   * \brief Type alias for the map type.
   */
  using map_type = std::map<std::string, arg, std::less<>>;

  /**
   * \brief Returns true if the given key is present.
   * \param key The key to search for.
   * \returns True if the key is present, otherwise false.
   */
  [[nodiscard]] inline bool contains(const std::string &key) const { return values.contains(key); }
  /**
   * \brief Gets the actual type of the value corresponding to the given key.
   * \param key The key whose value-type to fetch.
   * \returns The type of the value corresponding to the key, as a `dotchat::proto::_intl_::value_types` value.
   * \throws `std::out_of_range` if the key is not present.
   */
  [[nodiscard]] inline val_types type(const std::string &key) const { return values.at(key).type(); }

  /**
   * \brief Returns a reference to the value corresponding to `key`, or a new one if the key wasn't present yet.
   * \param key The key to search for/add.
   * \returns A reference to the value for key.
   */
  inline arg &operator[](const std::string &key) { return values[key]; }
  /**
   * \brief Returns a reference to the value corresponding to `key`.
   * \param key The key to search for.
   * \returns A reference to the value for key.
   * \throws `std::out_of_range` if the key is not present.
   */
  inline const arg &operator[](const std::string &key) const { return values.at(key); }

  /**
   * \short Retrieves the value corresponding to the given key, and casts it to the requested type.
   * \tparam T The type to cast to. Should be representable (`dotchat::proto::_intl_::matching_enum<T>::val` must be
   * well-formed.
   * \param key The key to search for.
   * \returns The value corresponding to `key`, cast to `T`.
   * \throws `std::bad_any_cast` if the contained type of the value doesn't match `T`.
   * \throws `std::out_of_range` if the key isn't present.
   */
  template <typename T>
  inline T operator[](const std::string &key) const { return static_cast<T>(values.at(key)); }

  /**
   * \short Retrieves the value corresponding to the given key, and casts it to the requested type.
   * \tparam T The type to cast to. Should be representable (`dotchat::proto::_intl_::matching_enum<T>::val` must be
   * well-formed.
   * \param key The key to search for.
   * \returns The value corresponding to `key`, cast to `T`.
   * \throws `std::bad_any_cast` if the contained type of the value doesn't match `T`.
   * \throws `std::out_of_range` if the key isn't present.
   */
  template <typename T>
  inline T as(const std::string &key) const { return (*this).template operator[]<T>(key); }

  /**
   * \short Changes the value corresponding to the given key, or adds it to the set.
   * \tparam T The type of the value to set. Should be representable.
   * \param val The key-value pair to modify/add.
   */
  template <typename T>
  void set(std::pair<std::string, T> val) {
    arg a0;
    a0 = val.second;
    values[val.first] = a0;
  }

  /**
   * \short Gets the amount of key-value pairs in the set.
   * \returns The amount of key-value pairs in the set.
   */
  [[nodiscard]] inline size_t size() const { return values.size(); }

  /**
   * \short Changes the value corresponding to the given key, or adds it to the set.
   * \param val The key-value pair to modify/add.
   */
  inline void set(const std::pair<std::string, arg> &val) {
    values[val.first] = val.second;
  }

  /**
   * \short Iterator type which iterates over all keys in the set (wrapper around
   * `dotchat::proto::_intl_::arg_obj::map_type`).
   */
  struct iterator {
    /**
     * \short Dereferences the iterator, returning the current key.
     * \returns The key this iterator points to.
     */
    std::string operator*() const { return it->first; }
    /**
     * \short Increments the iterator, moving to the next key (in string-sort order).
     * \returns A new iterator pointing to the next key.
     */
    iterator operator++() { return {++it}; }
    /**
     * \short Compares two iterators for equivalence.
     * \param other The iterator to compare to.
     * \returns True if both iterators point to a different key, otherwise false.
     */
    bool operator!=(const iterator &other) const {
      return it != other.it;
    }

    /**
     * The internal map iterator.
     */
    map_type::const_iterator it;
  };

  /**
   * \short Returns a new iterator to the first key in the set.
   * \returns An iterator to the first key in the set.
   */
  [[nodiscard]] iterator begin() const { return {values.begin()}; }
  /**
   * \short Returns a new iterator past the last key in the set.
   * \returns An iterator past the last key in the set.
   */
  [[nodiscard]] iterator end() const { return {values.end()}; }

private:
  /**
   * The actual value collection.
   */
  map_type values;
};

/**
 * \short Concept relaying the meaning of a representable type.
 * \tparam T The type to check.
 *
 * A type is representable if one of the following holds:
 * - `dotchat::proto::_intl_::is_trivially_repr<T>` (the type is trivially representable); or
 * - `std::is_same<T, dotchat::proto::_intl_::matching_type_t<dotchat::proto::_intl_::val_types::SUB_OBJECT>>`; or
 * - `std::is_same<T, dotchat::proto::_intl_::matching_type_t<dotchat::proto::_intl_::val_types::LIST>>`.
 */
template <typename T>
concept is_repr = is_trivially_repr<T> || one_of<
    T,
    matching_type_t<val_types::SUB_OBJECT>, matching_type_t<val_types::LIST>
>::val;
}

/**
 * \short Structure representing an error while parsing a message.
 * \see `std::logic_error`
 */
struct message_error : public std::logic_error {
  using logic_error::logic_error;
};

/**
 * \short Class representing a message in the dotchat protocol.
 */
class message {
public:
  /**
   * \short Type alias for the byte type (inherited from the underlying TLS bytestream).
   */
  using byte = tls::bytestream::byte;
  /**
   * \short Type alias for the command type (`std::string`).
   */
  using command = std::string;
  /**
   * \short Type alias for the key type (`std::string`).
   */
  using key = std::string;
  /**
   * \short Type alias for the argument value type (`dotchat::proto::_intl_::arg`).
   */
  using arg = _intl_::arg;
  /**
   * \short Type alias for the argument value map type (`dotchat::proto::_intl_::arg_obj`).
   */
  using arg_obj = _intl_::arg_obj;
  /**
   * \short Type alias for the argument value list type (`dotchat::proto::_intl_::arg_list`).
   */
  using arg_list = _intl_::arg_list;
  /**
   * \short Type alias for the value types enumeration (`dotchat::proto::_intl_::val_types`).
   */
  using arg_type = _intl_::val_types;

  /**
   * \short Constructs a default message (no command or arguments, highest possible protocol version).
   */
  message() = default;

  /**
   * \short Reads the next message in the byte stream.
   * \param stream The stream to read from.
   */
  explicit message(tls::bytestream &stream);

  /**
   * \short Constructs a message from a command and a set of key-value pairs.
   * \tparam Ts The types of the values; each has to satisfy `dotchat::proto::_intl_::is_repr<T>`.
   * \param cmd The command of the message.
   * \param values The key-value pairs in the message.
   */
  template <_intl_::is_repr ... Ts>
  explicit message(command cmd, std::pair<key, Ts>... values): cmd{std::move(cmd)} {
    (args.set(values), ...);
  }

  /**
   * \short Copies another message, adding or changing one or more key-value pairs.
   * \tparam T1 The type of the first value; has to satisfy `dotchat::proto::_intl_::is_repr<T1>`.
   * \tparam Ts The types of the other values; each has to satisfy `dotchat::proto::_intl_::is_repr<T>`.
   * \param other The message to copy from.
   * \param mod1 The first modification (key-value pair to add or change).
   * \param mods The other modifications (key-value pairs to add or change).
   */
  template <_intl_::is_repr T1, _intl_::is_repr ... Ts>
  message(const message &other, std::pair<key, T1> mod1, std::pair<key, Ts>... mods) {
    *this = other;
    args.set(mod1);
    (args.set(mods),...);
  }

  /**
   * \short Accesses the object containing the arguments.
   * \returns A reference to the map object containing the arguments.
   */
  inline arg_obj &map() { return args; }
  /**
   * \short Accesses the command for this message.
   * \returns A reference to the command.
   */
  inline command &get_command() { return cmd; }

  /**
   * \short Accesses the object containing the arguments.
   * \returns A constant reference to the map object containing the arguments.
   */
  [[nodiscard]] inline const arg_obj &map() const { return args; }
  /**
   * \short Accesses the command for this message.
   * \returns A constant reference to the command.
   */
  [[nodiscard]] inline const command &get_command() const { return cmd; }

  /**
   * \short Returns the preferred major protocol version for this implementation.
   * \returns The preferred major version (0x00).
   */
  inline static byte preferred_major_version() { return 0x00; }
  /**
   * \short Returns the preferred minor protocol version for this implementation.
   * \returns The preferred minor version (0x01).
   */
  inline static byte preferred_minor_version() { return 0x01; }

  /**
   * \short Checks whether the two given bytes match the magic number (0x2E 0x43).
   * \param b1 The first byte (will be compared to 0x2E).
   * \param b2 The second byte (will be compared to 0x43).
   * \returns True if `b1 == 0x2E && b2 == 0x43`, otherwise false.
   */
  inline static bool magic_number_match(byte b1, byte b2) {
    return b1 == 0x2e && b2 == 0x43;
  }

  /**
   * \short Writes this message to the given TLS byte stream.
   * \param strm The stream to write to.
   * \throws `dotchat::proto::message_error` if the map of any of its sub-objects has more than 255 keys.
   * \throws `dotchat::proto::message_error` if the command or any string value (including keys) is longer than 255
   * characters.
   */
  void send_to(tls::bytestream &strm) const;

  /**
   * \short Inserts this message into the given TLS byte stream.
   * \param stream The stream to insert to.
   * \param m The message to insert.
   * \returns A reference to the modified stream.
   *
   * If `s` is a `dotchat::tls::bytestream`, and `m` is a `dotchat::proto::message`, then `s << m` is equivalent to
   * `m.send_to(s)` (except for return value).
   */
  inline friend tls::bytestream &operator<<(tls::bytestream &stream, const message &m) {
    m.send_to(stream);
    return stream;
  }

private:
  /**
   * The major version of the protocol used for this message.
   */
  byte protocol_major = preferred_major_version();
  /**
   * The minor version of the protocol used for this message.
   */
  byte protocol_minor = preferred_minor_version();
  /**
   * The command in this message.
   */
  command cmd;
  /**
   * The arguments to this message's receiver.
   */
  arg_obj args;
};
}

/*
 * --- MESSAGE FORMAT ---
 * .C hexadecimal (2 bytes; magic number: 0x2E 0x43)
 * protocol_version (2 bytes; major minor)
 * cmd_len (1 byte)
 * cmd (n bytes; indicated by cmd_len)
 * arg_count (1 byte)
 * args.
 */

/*
 *  --- ARGUMENT FORMAT ---
 *  key_len (1 byte)
 *  key (n bytes; indicated by key_len)
 *  val_type (1 byte)
 *  val (integral/float types: byte-per-byte; other types: see below).
 */

/*
 *  --- VALUE FORMAT (STRINGS) ---
 *  val_len (1 byte)
 *  val_cnt (n bytes; indicated by val_len)
 */

/*
 *  --- VALUE FORMAT (LISTS) ---
 *  cnt_type (1 byte)
 *  list_len (4 bytes)
 *  list_values (n bytes; each is same as val in ARGUMENT FORMAT)
 */

/*
 *  --- VALUE FORMAT (SUB-OBJECTS) ---
 *  sub_obj_count (1 byte)
 *  sub_objs (same as args).
 */

#endif //DOTCHAT_CLIENT_MESSAGE_HPP
