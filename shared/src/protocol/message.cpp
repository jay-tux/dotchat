/////////////////////////////////////////////////////////////////////////////
// Name:        message.cpp
// Purpose:     Message parser and struct for the protocol (impl)
// Author:      jay-tux
// Created:     July 06, 2022 3:33 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include <bit>
#include <string>
#include <span>
#include <sstream>
#include <array>

#include "logger.hpp"
#include "protocol/message.hpp"
#include "protocol/message_error.hpp"

using namespace dotchat;
using namespace dotchat::tls;
using namespace dotchat::proto;
using namespace dotchat::values;

const logger::log_source init{"MESSAGE", blue};

#define CMDS X(AUTH) X(EXIT) X(LOAD) X(LOADO) X(LOADM) X(STORE) X(OK) X(ERR)
const char *cmd_repr(command_type c) {
#define X(cmd) case command_type::cmd: return #cmd;
  switch(c) {
    CMDS
    default: throw msg_error("Unparsable command");
  }
#undef X
}

command_type parse(const std::string_view &val) {
#define X(cmd) if(val == #cmd) return command_type::cmd;
  CMDS
  throw msg_error("Unparsable command");
#undef X
}

command_type read_cmd(std::span<bytestream::byte> v, int &idx) {
  std::string val;
  for(; idx < v.size() && v[idx] != ' '; idx++) {
    if(v[idx] < 'A' || v[idx] > 'Z') {
      throw msg_error("Unparsable command");
    }
    val += static_cast<char>(v[idx]);
  }
  idx++;
  return parse(val);
}

std::string read_string(std::span<bytestream::byte> v, int &idx) {
  std::string res;
  ++idx;
  bool prev_bs = false;
  while(idx < v.size()) {
    if(v[idx] == '"') {
      if(prev_bs) res += '"';
      else break;
    }
    else {
      if(prev_bs) res += '\\';

      prev_bs = false;
      if(v[idx] == '\\') prev_bs = true;
      else res += v[idx];
    }
    idx++;
  }
  ++idx;
  if(v[idx] != ' ')
    throw msg_error("Required space-separator missing between arguments");
  ++idx;
  return res;
}

std::string read_non_string(std::span<bytestream::byte> v, int &idx) {
  std::string res;
  while(idx < v.size() && v[idx] >= '0' && v[idx] <= '9') {
    res += v[idx];
    ++idx;
  }

  if(idx != v.size() && v[idx] != ' ')
    throw msg_error("Invalid character in numerical constant");
  return res;
}

void read_arg(std::span<bytestream::byte> v, int &idx, char &key, std::string &val) {
  key = v[idx];
  if(key < 'a' || key > 'z') {
    throw msg_error("Invalid key");
  }
  ++idx;

  if(v[idx] != ':') throw msg_error("Missing expected separator (:)");
  ++idx;

  val = (v[idx] == '"') ? read_string(v, idx) : read_non_string(v, idx);
}

std::variant<int, std::string> parse_val(const std::string &val) {
  if(val[0] >= '0' && val[0] <= '9') {
    int res = std::stoi(val);
    return { res };
  }
  else {
    return { val };
  }
}

template <size_t len>
void hex_dump(const std::array<char, len> &arr) requires (len >= 1) {
  log << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(arr[0]);
  for(size_t i = 1; i < len; i++) {
    log << " 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(arr[i]);
  }
  log << std::setfill(' ') << std::dec;
}

message::message(bytestream &source) {
  log << init << "Started parsing message..." << endl;
  std::array<char, sizeof(uint32_t)> vs = {};
  source >> vs[0] >> vs[1] >> vs[2] >> vs[3];
  auto len = std::bit_cast<uint32_t>(vs);
  log << init << "  -> Message length is " << len << " (hex dump: ";
  hex_dump(vs);
  log << ")" << endl;

  std::vector<bytestream::byte> _data_buf;
  _data_buf.reserve(len);
  std::span<bytestream::byte> data(_data_buf.begin(), len);
  log << init << "  -> Asking bytestream for " << len << " bytes of data." << endl;
  log << init << "  -> Bytestream remaining size: " << source.size() << endl;
  size_t curr = source.read(data);
  log << init << "  -> Actual message length is " << curr << endl;
  if(data[0] != '\n') throw msg_error("Required line-feed missing");

  int idx = 1;
  command = read_cmd(data, idx);
  log << init << "  -> Message command is " << cmd_repr(command) << endl;

  char key;
  std::string val;
  while(idx < data.size()) {
    read_arg(data, idx, key, val);
    args[key] = parse_val(val);
    log << init << "  -> Read argument: key = `" << key << "`; value = `";
    std::visit([](const auto &got){ log << got; }, args[key]);
    log << "`" << endl;
  }
}

void message::write_to(std::ostream &target) const {
  std::stringstream msg("");
  msg << cmd_repr(command);
  for(const auto &[k,v] : args) {
    msg << " " << k << ":";
    if(std::holds_alternative<int>(v)) msg << std::get<int>(v);
    else msg << '"' << std::get<std::string>(v) << '"';
  }
  std::string str = msg.str();
  auto len = static_cast<uint32_t>(str.size());
  auto ls = std::bit_cast<std::array<char, sizeof(uint32_t)>>(len);
  target << ls[0] << ls[1] << ls[2] << ls[3] << '\n' << str;
  log << init << "Message length should be " << str.size() << " (hex dump: ";
  hex_dump(ls);
  log << ")" << endl;
}

std::string message::as_string() const {
  std::stringstream ss;
  log << init << "Calling as_string..." << endl;
  write_to(ss);
  return ss.str();
}