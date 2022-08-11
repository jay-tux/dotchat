/////////////////////////////////////////////////////////////////////////////
// Name:        message.cpp
// Purpose:     Message parser and struct for the protocol (impl)
// Author:      jay-tux
// Created:     July 06, 2022 3:33 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <bit>
#include <array>
#include <arpa/inet.h>

#include "logger.hpp"
#include "tls/tls_bytestream.hpp"
#include "tls/tls_connection.hpp"
#include "dynsize_array.hpp"
#include "protocol/message.hpp"

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::proto;
using namespace dotchat::values;

#define TYPES X(INT8) X(INT16) X(INT32) X(UINT8) X(UINT16) X(UINT32)\
  X(CHAR) X(STRING) X(SUB_OBJECT) X(LIST)

//const static logger::log_source init{"MESSAGE", blue};

inline const char *type_to_str(message::arg_type a){
#define X(v) case message::arg_type::v: return #v;
  switch(a) {
    TYPES
    default: return "???";
  }
#undef X
}

std::string read_string(bytestream &stream) {
  uint8_t size;
  stream >> size;
  dyn_size_array<char> arr(size + 1);
  for(uint8_t i = 0; i < size; i++) {
    stream >> arr[i];
  }
  arr[size] = '\0';
  return { arr.buf };
}

uint32_t reorder(uint32_t tmp) { return ntohl(tmp); }
int32_t reorder(int32_t v) {
  return std::bit_cast<int32_t>(reorder(std::bit_cast<uint32_t>(v)));
}

uint16_t reorder(uint16_t tmp) { return ntohs(tmp); }
int16_t reorder(int16_t v) {
  return std::bit_cast<int16_t>(reorder(std::bit_cast<uint16_t>(v)));
}

template <typename T>
concept requires_reorder =
    std::same_as<T, int16_t> || std::same_as<T, uint16_t> ||
    std::same_as<T, int32_t> || std::same_as<T, uint32_t>;

template <dotchat::proto::_intl_::is_repr T>
T read_single(bytestream &stream) {
  std::array<bytestream::byte, sizeof(T)> arr = {};
  stream.read(arr);
  T val = std::bit_cast<T>(arr);

  if constexpr (requires_reorder<T>) {
    val = reorder(val);
  }

  return val;
}

message::arg_obj read_arg_obj(bytestream &stream);
message::arg read_value(message::arg_type type, bytestream &stream);

template <>
std::string read_single<std::string>(bytestream &stream) {
  return read_string(stream);
}

template<>
message::arg_obj read_single<message::arg_obj>(bytestream &stream) {
  return read_arg_obj(stream);
}

template <>
message::arg_list read_single<message::arg_list>(bytestream &stream) {
  uint8_t contained;
  stream >> contained;
  auto type = static_cast<message::arg_type>(contained);
  auto size = read_single<uint32_t>(stream);

  message::arg_list list;
  for(size_t i = 0; i < size; i++) {
    list.push_back(read_value(type, stream));
  }
  return list;
}


message::arg read_value(message::arg_type type, bytestream &stream) {
#define X(v) case message::arg_type::v: return message::arg{ read_single<typename dotchat::proto::_intl_::matching_type_t<message::arg_type::v>>(stream) };
  switch(type) {
    TYPES

    default:
      throw message_error("Invalid type to read.");
  }
#undef X
}

message::arg_obj read_arg_obj(bytestream &stream) {
  message::arg_obj res;
  uint8_t count;
  stream >> count;
  for(uint8_t i = 0; i < count; i++) {
    std::string key = read_string(stream);
    uint8_t type_i;
    stream >> type_i;
    auto type = static_cast<message::arg_type>(type_i);
    auto value = read_value(type, stream);
    res.set({ key, value });
  }
  return res;
}

message::message(bytestream &stream) {
  byte b1;
  byte b2;
  stream >> b1 >> b2;

  if(!magic_number_match(b1, b2))
    throw message_error("Can't parse message (missing magic number)");

  stream >> protocol_major >> protocol_minor;
  if(protocol_major > preferred_major_version())
    throw message_error("Can't parse message (incompatible major version)");
  if(protocol_major == preferred_major_version() && protocol_minor > preferred_minor_version())
    throw message_error("Can't parse message (incompatible minor version)");

  cmd = read_string(stream);
  args = read_arg_obj(stream);
}


uint16_t reorder_send(uint16_t v) { return htons(v); }
int16_t reorder_send(int16_t v) {
  return std::bit_cast<int16_t>(reorder_send(std::bit_cast<uint16_t>(v)));
}
uint32_t reorder_send(uint32_t v) { return htonl(v); }
int32_t reorder_send(int32_t v) {
  return std::bit_cast<int32_t>(reorder_send(std::bit_cast<uint32_t>(v)));
}

void send_one(const message::arg_obj &obj, bytestream &strm);
void send_list(const message::arg_list &l, bytestream &strm);

void send_val(int8_t v, bytestream &strm) { strm << v; }
void send_val(uint8_t v, bytestream &strm) { strm << v; }
void send_val(int16_t v, bytestream &strm) { strm << reorder_send(v); }
void send_val(uint16_t v, bytestream &strm) { strm << reorder_send(v); }
void send_val(int32_t v, bytestream &strm) { strm << reorder_send(v); }
void send_val(uint32_t v, bytestream &strm) { strm << reorder_send(v); }
void send_val(char v, bytestream &strm) { strm << v; }
void send_val(std::string_view v, bytestream &strm) {
  if(v.size() > 0xFF) throw message_error("String too long to send.");
  strm << (message::byte)v.size();
  for(const char &c: v) strm << c;
}

#define SUBSET X(INT8) X(INT16) X(INT32) X(UINT8) X(UINT16) X(UINT32) X(CHAR) X(STRING)

void send_arg(const message::arg &a, bytestream &strm, bool send_type = true) {
  if(send_type) strm << (int8_t)a.type();
#define X(v) case message::arg_type::v: \
  send_val(a.get<message::arg_type::v>(), strm); \
  break;

  switch(a.type()) {
    SUBSET

    case message::arg_type::LIST:
      send_list(static_cast<message::arg_list>(a), strm);
      break;
    case message::arg_type::SUB_OBJECT:
      send_one(static_cast<message::arg_obj>(a), strm);
      break;

    default:
      throw message_error("Can't send this object.");
  }
}

void send_one(const message::arg_obj &obj, bytestream &strm) {
  if(obj.size() > 0xFF) throw message_error("Too much arguments.");
  strm << (message::byte)obj.size();
  for(const auto &key: obj) {
    send_val(key, strm);
    send_arg(obj[key], strm);
  }
}

void send_list(const message::arg_list &l, bytestream &strm) {
  strm << (int8_t)l.type();
  send_val((uint32_t)l.size(), strm);
  for(const auto v: l) {
    send_arg(v, strm, false);
  }
}

void message::send_to(tls::bytestream &strm) const {
  strm << (byte)0x2E << (byte)0x43
       << preferred_major_version() << preferred_minor_version();

  send_val(cmd, strm);

  send_one(args, strm);
}