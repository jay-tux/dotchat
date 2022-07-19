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

#define CMDS X(AUTH) X(EXIT) X(LOAD) X(LOADO) X(LOADM) X(LOADC) X(LEA) X(STORE) X(OK) X(ERR)
const char *cmd_repr(command_type c) {
#define X(cmd) case command_type::cmd: return #cmd;
  switch(c) {
    CMDS
    default:
      throw msg_error("Unparsable command");
  }
#undef X
}

command_type parse(const std::string_view &val) {
#define X(cmd) if(val == #cmd) return command_type::cmd;
  CMDS
  log << init << "Can't convert `" << val << "` to a valid command." << endl;
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
  if(idx < v.size() && v[idx] != ' ')
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

  if(idx < v.size() && v[idx] != ' ')
    throw msg_error("Invalid character in numerical constant");
  return res;
}

bool read_arg(std::span<bytestream::byte> v, int &idx, char &key, std::string &val) {
  key = v[idx];
  if(key < 'a' || key > 'z') {
    throw msg_error("Invalid key");
  }
  ++idx;

  if(v[idx] != ':') throw msg_error("Missing expected separator (:)");
  ++idx;

  bool to_parse = v[idx] != '"';
  val = (v[idx] == '"') ? read_string(v, idx) : read_non_string(v, idx);
  return to_parse;
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

message::message(bytestream &source) {
  std::array<char, sizeof(uint32_t)> vs = {};
  source >> vs[0] >> vs[1] >> vs[2] >> vs[3];
  auto len = std::bit_cast<uint32_t>(vs);

  std::vector<bytestream::byte> _data_buf;
  _data_buf.reserve(len);
  std::span<bytestream::byte> data(_data_buf.begin(), len);
  source.read(data);
  if(data[0] != '\n') throw msg_error("Required line-feed missing");

  int idx = 1;
  log << init << "Stream length is " << data.size() << "; current index is " << idx << endl;
  command = read_cmd(data, idx);

  char key;
  std::string val;
  while(idx < data.size()) {
    args[key] = read_arg(data, idx, key, val) ? parse_val(val) : val;
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
  if(args.empty()) msg << " ";
  std::string str = msg.str();
  auto len = static_cast<uint32_t>(str.size());
  auto ls = std::bit_cast<std::array<char, sizeof(uint32_t)>>(len);
  target << ls[0] << ls[1] << ls[2] << ls[3] << '\n' << str;
}

std::string message::as_string() const {
  std::stringstream ss;
  write_to(ss);
  return ss.str();
}