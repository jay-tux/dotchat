/////////////////////////////////////////////////////////////////////////////
// Name:        message_intl.cpp
// Purpose:     Message parser and struct internals (impl for _intl_ ns)
// Author:      jay-tux
// Created:     August 01, 2022 8:23 PM
// Copyright:   (c) 2022 jay-tux
// Licence:     MPL
/////////////////////////////////////////////////////////////////////////////

#include "protocol/message.hpp"

using namespace dotchat::proto::_intl_;

arg::arg(const arg_list &val)  : _type{matching_enum<arg_list>::val}, _content{val} {}
arg::arg(const arg_obj &val)  : _type{matching_enum<arg_obj>::val}, _content{val} {}

arg &arg::operator=(const arg_list &l) {
  _type = matching_enum<arg_list>::val;
  _content.reset();
  _content = l;
  return *this;
}

arg &arg::operator=(const arg_obj &o) {
  _type = matching_enum<arg_obj>::val;
  _content.reset();
  _content = o;
  return *this;
}

arg::operator arg_list() const {
  if(_type == matching_enum<arg_list>::val) return std::any_cast<arg_list>(_content);
  else throw std::bad_any_cast();
}

arg::operator arg_obj() const {
  if(_type == matching_enum<arg_obj>::val) return std::any_cast<arg_obj>(_content);
  else throw std::bad_any_cast();
}

arg_list &arg_list::get_list(size_t n) {
  if(_contained == matching_enum<arg_list>::val)
    return *std::any_cast<arg_list *>(&_content[n]);
  else
    throw std::bad_any_cast();
}

arg_obj &arg_list::get_obj(size_t n) {
  if(_contained == matching_enum<arg_obj>::val)
    return *std::any_cast<arg_obj *>(&_content[n]);
  else
    throw std::bad_any_cast();
}

const arg_list &arg_list::get_list(size_t n) const {
  if(_contained == matching_enum<arg_list>::val)
    return *std::any_cast<arg_list *>(&_content[n]);
  else
    throw std::bad_any_cast();
}

const arg_obj &arg_list::get_obj(size_t n) const {
  if(_contained == matching_enum<arg_obj>::val)
    return *std::any_cast<arg_obj *>(&_content[n]);
  else
    throw std::bad_any_cast();
}

void arg_list::push_back(const arg_list &val) {
  if(_content.empty())
    _contained = matching_enum<arg_list>::val;
  if(_contained == matching_enum<arg_list>::val)
    _content.emplace_back(val);
  else
    throw std::bad_any_cast();
}

void arg_list::push_back(const arg_obj &val) {
  if(_content.empty())
    _contained = matching_enum<arg_obj>::val;
  if(_contained == matching_enum<arg_obj>::val)
    _content.emplace_back(val);
  else
    throw std::bad_any_cast();
}

arg_list::iterable<arg_list> arg_list::iterable_for_sublist() {
  if(_contained == matching_enum<arg_list>::val)
    return arg_list::iterable<arg_list>{ _content };
  else
    throw std::bad_any_cast();
}

arg_list::iterable<arg_obj> arg_list::iterable_for_sub_obj() {
  if(_contained == matching_enum<arg_obj>::val)
    return arg_list::iterable<arg_obj>{ _content };
  else
    throw std::bad_any_cast();
}