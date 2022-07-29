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

namespace dotchat {
namespace _intl_ {
template <typename T1, typename T2> concept cvref_same =
    std::same_as<typename std::remove_cvref_t<T1>, typename std::remove_cvref_t<T2>>;

template <typename T, typename ... Coll> struct _contains;

template <typename T1> struct _contains<T1> {
  const static bool value = false;
};

template <typename T1, typename T2, typename ... Coll> struct _contains<T1, T2, Coll...> {
  const static bool value = cvref_same<T1, T2> || _contains<T1, Coll...>::value;
};

template <typename T1, typename ... Coll> concept contains =
  _contains<T1, Coll...>::value;

template <typename T1, typename T2, typename ... Coll> concept cvref_eql_or_contains =
  cvref_same<T1, T2> || contains<T1, Coll...>;
}

struct mono {};

struct invalid_either : std::logic_error {
  invalid_either() : std::logic_error{str} {}

  [[nodiscard]] const char * what() const noexcept override {
    return str;
  }

  constexpr const static char * const str = "The type you requested was not set on this union.";
};

template <typename T1, typename ... TRest> class either {
public:
  enum class tristate { NONE, FST, OTHER };

  constexpr either() requires(std::default_initializable<T1>) : value{.val = new T1()}, holds_t1{tristate::NONE} {}

  constexpr either(T1 val1) : value{.val = new T1(val1)}, holds_t1{tristate::FST} {}

  template <typename T>
  constexpr either(T val) requires(_intl_::contains<T, TRest...>) : value{.other = new either<TRest...>(val)}, holds_t1{tristate::SND} {}

  either(const either<T1, TRest...> &other) { *this = other; }
  either(either<T1, TRest...> &&other) noexcept { *this = std::move(other); }

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

  either<T1, TRest...> &operator=(either<T1, TRest...> &&other) noexcept {
    std::swap(holds_t1, other.holds_t1);
    std::swap(value.val, other.value.val);
    return *this;
  }

  either<T1, TRest...> &operator=(const T1 &val) {
    *this = either<T1, TRest...>(val);
    return *this;
  }

  template <typename T> requires(_intl_::cvref_eql_or_contains<T, T1, TRest...>)
  either<T1, TRest...> &operator=(const T &val) {
    *this = either<T1, TRest...>(val);
    return *this;
  }

  template <typename T> requires(_intl_::cvref_eql_or_contains<T, T1, TRest...>)
  [[nodiscard]] bool holds() const {
    if constexpr(_intl_::cvref_same<T, T1>) return holds_t1 == tristate::FST;
    else {
      return holds_t1 == tristate::OTHER && value.other.template holds<T>();
    }
  }

  template <typename T> requires(_intl_::cvref_eql_or_contains<T, T1, TRest...>)
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

  template <typename T> requires(_intl_::cvref_eql_or_contains<T, T1, TRest...>)
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
  union { T1 *val; either<TRest...> *other; } value;
  tristate holds_t1{};
};

template <typename T1, typename T2>
class either<T1, T2> {
public:
  enum class tristate { NONE, FST, SND };
  constexpr either() requires(std::default_initializable<T1>) : value{.val = nullptr}, holds_t1{tristate::NONE} {}

  constexpr either(T1 val) : value{.val = new T1(val)}, holds_t1{tristate::FST} {}
  constexpr either(T2 val) : value{.other = new T2(val)}, holds_t1{tristate::SND} {}

  either(const either<T1, T2> &other) { *this = other; }
  either(either<T1, T2> &&other) noexcept { *this = std::move(other); }

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

  either<T1, T2> &operator=(either<T1, T2> &&other) noexcept {
    std::swap(value.val, other.value.val);
    std::swap(holds_t1, other.holds_t1);
    return *this;
  }

  either<T1, T2> &operator=(const T1 &val) {
    *this = either<T1, T2>(val);
    return *this;
  }

  either<T1, T2> &operator=(const T2 &val) {
    *this = either<T1, T2>(val);
    return *this;
  }

  template <typename T> requires(_intl_::contains<T, T1, T2>)
  [[nodiscard]] bool holds() const {
    if constexpr (_intl_::cvref_same<T, T1>) return holds_t1 == tristate::FST;
    else return holds_t1 == tristate::SND;
  }

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

  ~either() {
    if(value.val == nullptr) return;

    if(holds_t1 == tristate::FST) delete value.val;
    else if(holds_t1 == tristate::SND) delete value.other;
  }

private:
  union { T1 *val; T2 *other; } value;
  tristate holds_t1{};
};

template <typename T, typename ... Ts>
inline bool holds(const either<Ts...> &e) { return e.template holds<T>(); }

template <typename T, typename ... Ts>
inline T &get(either<Ts...> &e) { return e.template get<T>(); }

template <typename T, typename ... Ts>
inline const T &get(const either<Ts...> &e) { return e.template get<T>(); }
}

#endif //DOTCHAT_SERVER_EITHER_HPP
