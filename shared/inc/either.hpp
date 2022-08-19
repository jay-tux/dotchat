/////////////////////////////////////////////////////////////////////////////
// Name:        either.hpp
// Purpose:     Alternative variant implementation
// Author:      jay-tux
// Created:     July 21, 2022 3:54 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////


#ifndef DOTCHAT_SERVER_EITHER_HPP
#define DOTCHAT_SERVER_EITHER_HPP

#include <type_traits>
#include <concepts>
#include <utility>
#include <stdexcept>

/**
 * \short Namespace containing general code for dotchat. Specific code is organized in nested namespaces.
 */
namespace dotchat {
/**
 * \short Namespace for internal helper code.
 */
namespace _intl_ {
/**
 * \short Concept relaying the meaning of being cv-ref equal.
 * \tparam T1 The type to compare.
 * \tparam T2 The type to compare to.
 *
 * Types are cv-ref equal if they are the same type, ignoring const, volatile and references; the following are all
 * cv-ref equal:
 * - T, T&, T&&,
 * - const T, const T&, const T&&,
 * - volatile T, volatile T&, volatile T&&,
 * - const volatile T, const volatile T&, const volatile T&&.
 */
template <typename T1, typename T2> concept cvref_same =
    std::same_as<typename std::remove_cvref_t<T1>, typename std::remove_cvref_t<T2>>;

/**
 * \short Recursive type trait to check if a collection of types contains a specific type.
 * \tparam T The type to check.
 * \tparam Coll The collection of types.
 *
 * Equality is checked by using cv-ref equality.
 */
template <typename T, typename ... Coll> struct _contains;

/**
 * \short Base case.
 */
template <typename T1> struct _contains<T1> {
  const static bool value = false;
};

/**
 * \short Recursive case.
 */
template <typename T1, typename T2, typename ... Coll> struct _contains<T1, T2, Coll...> {
  const static bool value = cvref_same<T1, T2> || _contains<T1, Coll...>::value;
};

/**
 * \short Concept which checks if the given type is (ignoring const, volatile and references) equal to one of the other
 * types.
 * \tparam T1 The type to check.
 * \tparam Coll The set of types to check in.
 */
template <typename T1, typename ... Coll> concept contains =
  _contains<T1, Coll...>::value;
}

/**
 * \short Structure to represent an empty type; analogous to `std::monostate`.
 */
struct mono {};

/**
 * \short Structure to represent an invalid access; analogous to `std::bad_variant_access`.
 * \see `std::logic_error`
 */
struct invalid_either : std::logic_error {
  /**
   * \short Constructs a new error, with a set error message.
   */
  invalid_either() : std::logic_error{str} {}

  /**
   * \short The error message.
   */
  constexpr const static char * const str = "The type you requested was not set on this union.";
};

/**
 * \short Type-safe union based on dynamic memory; analogous to `std::variant`.
 * \tparam T1 The first type in the union.
 * \tparam TRest The other types in the union.
 *
 * This is the recursive case.
 */
template <typename T1, typename ... TRest> class either {
public:
  /**
   * \short Enumeration representing the three possible states for this union.
   */
  enum class tristate {
    NONE,     /*!< \short No member on this union is set. */
    FST,      /*!< \short The first member on this union is set. */
    OTHER     /*!< \short One of the other members on this union (not the first) is set. */
  };

  /**
   * \short Creates an empty union.
   */
  constexpr either() : value{.val = nullptr}, holds_t1{tristate::NONE} {}

  /**
   * \short Creates a new union, holding a value of the first type.
   * \param val1 The value to hold.
   */
  constexpr either(T1 val1) : value{.val = new T1(val1)}, holds_t1{tristate::FST} {}

  /**
   * \short Creates a new union, holding a value of one of the other types.
   * \tparam T The type of the value to store. It should satisfy `dotchat::_intl_::contains<T, TRest...>`.
   * \param val The value to store.
   */
  template <typename T>
  constexpr either(T val) requires(_intl_::contains<T, TRest...>) :
    value{.other = new either<TRest...>(val)}, holds_t1{tristate::SND} {}

  /**
   * \short Copy-initializes this union.
   * \param other The union to copy.
   */
  either(const either<T1, TRest...> &other) { *this = other; }
  /**
   * \short Move-initializes this union.
   * \param other The union to move from.
   */
  either(either<T1, TRest...> &&other) noexcept { *this = std::move(other); }

  /**
   * \short Copy-assigns this union, copying all data from the other one.
   * \param other The union to copy from.
   * \returns A reference to this union, after modification.
   *
   * If this union holds any value, it will be erased first.
   */
  either<T1, TRest...> &operator=(const either<T1, TRest...> &other) {
    if(this == &other) return *this;

    if(holds_t1 == tristate::FST) {
      if(value.val != nullptr) delete value.val;
    }
    else if(holds_t1 == tristate::OTHER) {
      if(value.other != nullptr) delete value.other;
    }

    holds_t1 = other.holds_t1;
    switch(other.holds_t1) {
      case tristate::FST:
        value.val = new T1(*(other.value.val));
        break;

      case tristate::OTHER:
        value.other = new either<TRest...>(*(other.value.other));
        break;

      default:
        value.val = nullptr;
        break;
    }

    return *this;
  }

  /**
   * \short Move-assigns this union, swapping all data with the other one.
   * \param other The union to swap with.
   * \returns A reference to this union, after modification.
   */
  either<T1, TRest...> &operator=(either<T1, TRest...> &&other) noexcept {
    std::swap(holds_t1, other.holds_t1);
    std::swap(value.val, other.value.val);
    return *this;
  }

  /**
   * \short Copies the given value into this union, overwriting any value that might already be in it.
   * \param val The value to copy.
   * \returns A reference to this union, after modification.
   */
  either<T1, TRest...> &operator=(const T1 &val) {
    *this = either<T1, TRest...>(val);
    return *this;
  }

  /**
   * \short Copies the given value into this union, overwriting any value that might already be in it.
   * \tparam T The type of the value to copy. It should satisfy `dotchat::_intl_::contains<T, T1, TRest...>`.
   * \param val The value to copy.
   * \returns A reference to this union, after modification.
   */
  template <typename T> requires(_intl_::contains<T, T1, TRest...>)
  either<T1, TRest...> &operator=(const T &val) {
    *this = either<T1, TRest...>(val);
    return *this;
  }

  /**
   * \short Checks whether this union holds a value of the given type.
   * \tparam T The type to check. It should satisfy `dotchat::_intl_::contains<T, T1, TRest...>`.
   * \returns True if this union holds a value of the requested type, otherwise false.
   */
  template <typename T> requires(_intl_::contains<T, T1, TRest...>)
  [[nodiscard]] bool holds() const {
    if constexpr(_intl_::cvref_same<T, T1>) return holds_t1 == tristate::FST;
    else {
      return holds_t1 == tristate::OTHER && value.other.template holds<T>();
    }
  }

  /**
   * \short Extracts the value of the requested type from this union.
   * \tparam T The type to extract. It should satisfy `dotchat::_intl_::contains<T, T1, TRest...>`.
   * \returns A reference to the value.
   * \throws `dotchat::invalid_either` if this union doesn't hold a value of that type.
   */
  template <typename T> requires(_intl_::contains<T, T1, TRest...>)
  [[nodiscard]] T &get() {
    if constexpr(_intl_::cvref_same<T, T1>) {
      if(holds_t1 != tristate::FST) throw invalid_either();
      return *value.val;
    }
    else {
      if(holds_t1 != tristate::OTHER) throw invalid_either();
      return value.other.template get<T>();
    }
  }

  /**
   * \short Extracts the value of the requested type from this union.
   * \tparam T The type to extract. It should satisfy `dotchat::_intl_::contains<T, T1, TRest...>`.
   * \returns A constant reference to the value.
   * \throws `dotchat::invalid_either` if this union doesn't hold a value of that type.
   */
  template <typename T> requires(_intl_::contains<T, T1, TRest...>)
  [[nodiscard]] const T &get() const {
    if constexpr(_intl_::cvref_same<T, T1>) {
      if(holds_t1 != tristate::FST) throw invalid_either();
      return *value.val;
    }
    else {
      if(holds_t1 != tristate::OTHER) throw invalid_either();
      return value.other.template get<T>();
    }
  }

  /**
   * \short Cleans up this union, releasing all memory used by it.
   */
  ~either() {
    if(value.val == nullptr) return;

    switch (holds_t1) {
      case tristate::FST:
        delete value.val;
        break;

      case tristate::SND:
        delete value.other;
        break;

      default:
        break;
    }
  }

private:
  /**
   * \short The internal union.
   */
  union { T1 *val; either<TRest...> *other; } value;
  /**
   * \short A value indicating which type is used on the union.
   */
  tristate holds_t1{};
};

/**
 * \short Type-safe union based on dynamic memory; analogous to `std::variant`.
 * \tparam T1 The first type in the union.
 * \tparam T2 The second type in the union.
 *
 * This is the base case (two types).
 */
template <typename T1, typename T2>
class either<T1, T2> {
public:
  /**
   * \short Enumeration representing the three possible states for this union.
   */
  enum class tristate {
    NONE,     /*!< \short No member on this union is set. */
    FST,      /*!< \short The first member on this union is set. */
    SND       /*!< \short The second member on this union is set. */
  };

  /**
   * \short Creates an empty union.
   */
  constexpr either() requires(std::default_initializable<T1>) : value{.val = nullptr}, holds_t1{tristate::NONE} {}

  /**
   * \short Creates a new union, holding a value of the first type.
   * \param val The value to hold.
   */
  constexpr either(T1 val) : value{.val = new T1(val)}, holds_t1{tristate::FST} {}
  /**
   * \short Creates a new union, holding the a value of the second type.
   * \param val The value to hold.
   */
  constexpr either(T2 val) : value{.other = new T2(val)}, holds_t1{tristate::SND} {}

  /**
   * \short Copy-initializes this union.
   * \param other The union to copy.
   */
  either(const either<T1, T2> &other) { *this = other; }
  /**
   * \short Move-initializes this union.
   * \param other The union to move from.
   */
  either(either<T1, T2> &&other) noexcept { *this = std::move(other); }

  /**
   * \short Copy-assigns this union, copying all data from the other one.
   * \param other The union to copy from.
   * \returns A reference to this union, after modification.
   *
   * If this union holds any value, it will be erased first.
   */
  either<T1, T2> &operator=(const either<T1, T2> &other) {
    if(this == &other) return *this;

    if(holds_t1 == tristate::FST) {
      if(value.val != nullptr) delete value.val;
    }
    else if(holds_t1 == tristate::SND) {
      if(value.other != nullptr) delete value.other;
    }

    if(other.holds_t1 == tristate::FST) value.val = new T1(*(other.value.val));
    else if(other.holds_t1 == tristate::SND) value.other = new T2(*(other.value.other));
    return *this;
  }

  /**
   * \short Move-assigns this union, swapping all data with the other one.
   * \param other The union to swap with.
   * \returns A reference to this union, after modification.
   */
  either<T1, T2> &operator=(either<T1, T2> &&other) noexcept {
    std::swap(value.val, other.value.val);
    std::swap(holds_t1, other.holds_t1);
    return *this;
  }

  /**
   * \short Copies the given value into this union, overwriting any value that might already be in it.
   * \param val The value to copy.
   * \returns A reference to this union, after modification.
   */
  either<T1, T2> &operator=(const T1 &val) {
    *this = either<T1, T2>(val);
    return *this;
  }

  /**
   * \short Copies the given value into this union, overwriting any value that might already be in it.
   * \param val The value to copy.
   * \returns A reference to this union, after modification.
   */
  either<T1, T2> &operator=(const T2 &val) {
    *this = either<T1, T2>(val);
    return *this;
  }

  /**
   * \short Checks whether this union holds a value of the given type.
   * \tparam T The type to check. It should satisfy `dotchat::_intl_::contains<T, T1, T2>`.
   * \returns True if this union holds a value of the requested type, otherwise false.
   */
  template <typename T> requires(_intl_::contains<T, T1, T2>)
  [[nodiscard]] bool holds() const {
    if constexpr (_intl_::cvref_same<T, T1>) return holds_t1 == tristate::FST;
    else return holds_t1 == tristate::SND;
  }

  /**
   * \short Extracts the value of the requested type from this union.
   * \tparam T The type to extract. It should satisfy `dotchat::_intl_::contains<T, T1, T2>`.
   * \returns A reference to the value.
   * \throws `dotchat::invalid_either` if this union doesn't hold a value of that type.
   */
  template <typename T> requires(_intl_::contains<T, T1, T2>)
  [[nodiscard]] T &get() {
    if constexpr (_intl_::cvref_same<T, T1>) {
      if(holds_t1 != tristate::FST) throw invalid_either();
      return *value.val;
    }
    else {
      if(holds_t1 != tristate::SND) throw invalid_either();
      return *value.other;
    }
  }

  /**
   * \short Extracts the value of the requested type from this union.
   * \tparam T The type to extract. It should satisfy `dotchat::_intl_::contains<T, T1, T2>`.
   * \returns A constant reference to the value.
   * \throws `dotchat::invalid_either` if this union doesn't hold a value of that type.
   */
  template <typename T> requires(_intl_::contains<T, T1, T2>)
  [[nodiscard]] T &get() const {
    if constexpr (_intl_::cvref_same<T, T1>) {
      if(holds_t1 != tristate::FST) throw invalid_either();
      return *value.val;
    }
    else {
      if(holds_t1 != tristate::SND) throw invalid_either();
      return *value.other;
    }
  }

  /**
   * \short Cleans up this union, releasing all memory used by it.
   */
  ~either() {
    if(value.val == nullptr) return;

    if(holds_t1 == tristate::FST) delete value.val;
    else if(holds_t1 == tristate::SND) delete value.other;
  }

private:
  /**
   * \short The internal union.
   */
  union { T1 *val; T2 *other; } value;
  /**
   * \short A value indicating which type is used on the union.
   */
  tristate holds_t1{};
};

/**
 * \short Checks whether or not the given union holds the given type.
 * \tparam T The type to search for; should satisfy `dotchat::_intl_::contains<T, Ts...>`.
 * \tparam Ts The types contained in the union.
 * \param e The union to check.
 * \returns True if the type is set, otherwise false.
 */
template <typename T, typename ... Ts>
inline bool holds(const either<Ts...> &e) requires(_intl_::contains<T, Ts...>) { return e.template holds<T>(); }

/**
 * \short Extracts the value with the given type from the given union.
 * \tparam T The type to search for; should satisfy `dotchat::_intl_::contains<T, Ts...>`.
 * \tparam Ts The types contained in the union.
 * \param e The union to search.
 * \returns A reference to the value.
 * \throws `dotchat::invalid_either` if this union doesn't hold a value of that type.
 */
template <typename T, typename ... Ts>
inline T &get(either<Ts...> &e) requires(_intl_::contains<T, Ts...>) { return e.template get<T>(); }

/**
 * \short Extracts the value with the given type from the given union.
 * \tparam T The type to search for; should satisfy `dotchat::_intl_::contains<T, Ts...>`.
 * \tparam Ts The types contained in the union.
 * \param e The union to search.
 * \returns A constant reference to the value.
 * \throws `dotchat::invalid_either` if this union doesn't hold a value of that type.
 */
template <typename T, typename ... Ts>
inline const T &get(const either<Ts...> &e) requires(_intl_::contains<T, Ts...>) { return e.template get<T>(); }
}

#endif //DOTCHAT_SERVER_EITHER_HPP
