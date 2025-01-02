//
// Created by lenovo on 2025/1/1.
//

#ifndef CSTR_H
#define CSTR_H

#include "c.h"
#include "cmd.h"

#include <array>
#include <cmath>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <vector>

template <size_t size>
class cstr : public std::array<char, size + 1> {
public:
  static constexpr int N = size;
  operator std::string() {
    return std::string(this->data());
  }
};

typedef int64_t pos_t;

// book types
#define DF(n, len) using n##_t = cstr<len>;
DF(ISBN, 20);
DF(bookname, 60);
DF(author, 60);
DF(keyword, 60);
using quantity_t = uint32_t;
using price_t = double;
using totalcost_t = double;
using bookid_t = pos_t;

// account types
using privilege_t = int;
using identity_t = cstr<10>;
using userid_t = cstr<30>;
using username_t = cstr<30>;
using password_t = cstr<30>;

static bool visibleChar(char ch) {
  return 32 <= ch && ch <= 126;
}

static bool validUserIDChar(char ch) {
  return ('a' <= ch and ch <= 'z') or ('A' <= ch and ch <= 'Z') or ('0' <= ch and ch <= '9') or ch == '_';
}

static bool inside(char c, const char *s) {
  for (; *s; ++s)
    if (c == *s)
      return true;
  return false;
}

template <size_t size>
bool operator==(const cstr<size> &u, const std::string &v) {
  return u.data() == v;
}

template <size_t size>
bool operator==(const std::string &v, const cstr<size> &u) {
  return u.data() == v;
}

template <size_t sizeu, size_t sizev>
bool operator==(const cstr<sizeu> &u, const cstr<sizev> &v) {
  return strcmp(u.data(), v.data()) == 0;
}

template <size_t sizeu, size_t sizev>
bool operator!=(const cstr<sizeu> &u, const cstr<sizev> &v) {
  return not (u == v);
}

template <size_t sizeu, size_t sizev>
bool operator<(const cstr<sizeu> &u, const cstr<sizev> &v) {
  return strcmp(u.data(), v.data()) < 0;
}

static std::vector<std::string> split_keyword(std::string keyword) {
  std::vector<std::string> ret;
  std::string now;
  for (int i = 0; i < (int)keyword.size(); ++i)
    if (keyword[i] == '|') {
      if (!(i > 0 && now != "" && i + 1 != (int)keyword.size() && keyword[i + 1] != '|')) throw Error("");;
      ret.push_back(now);
      now = "";
    } else
      now += keyword[i];
  if (now != "")
    ret.push_back(now);
  for (int i = 1; i < (int)ret.size(); ++i)
    for (int j = 0; j < i; ++j)
      if (!(ret[i] != ret[j])) throw Error("");;
  return ret;
}

template <size_t size>
bool cstr_end(const cstr<size> s) {
  return strnlen(s.data(), s.max_size()) < s.max_size();
}

template <size_t size>
bool cstr_null(const cstr<size> s) {
  return strnlen(s.data(), s.max_size()) == 0;
}

template <size_t size>
static cstr<size> string2cstr(std::string s) {
  if (!(s.size() <= size)) throw Error("");;
  cstr<size> ret;
  strncpy(ret.data(), s.c_str(), size + 1);
  return ret;
}

static double string2double(std::string s) {
  if (!(s.size() <= 13)) throw Error("");;
  std::stringstream ss{s};
  double ret{};
  ss >> ret;
  return std::round(ret * 100) / 100;
}

static std::string double2string(double r) {
  std::stringstream ss;
  ss << r;
  std::string ret;
  ss >> ret;
  return ret;
}

static bool param_inside(Tokenized &tk, const std::vector<std::string> &allow) {
  for (auto &[k, _] : tk.param) {
    bool find = false;
    for (auto &s : allow)
      if (s == k) {
        find = true;
        break;
      }
    if (not find)
      return false;
  }
  return true;
}

static int string2int(std::string s) {
  // return std::atoi(s.c_str());
  if (!(s.size() > 0)) throw Error("");;
  if (!(s.size() <= 10)) throw Error("");;
  for (int i = 0; i < (int)s.size(); ++i)
    if (!('0' <= s[i] && s[i] <= '9')) throw Error("");;
  std::stringstream ss;
  ss << s;
  long long ret;
  ss >> ret;
  if (!(ret <= 2147483647)) throw Error("");;
  return ret;
}

static bool valid_privilege(privilege_t privilege) {
  return privilege == 1 or privilege == 3 or privilege == 7;
}

static bool valid_price(std::string s) {
  if (s.size() > 13 or s.size() == 0)
    return false;
  int cnt = 0, last = 0;
  for (int i = 0; i < (int)s.size(); ++i)
    if (s[i] != '.' and (s[i] < '0' or s[i] > '9'))
      return false;
    else if (s[i] == '.')
      ++cnt, last = i;
  if (cnt > 1)
    return false;
  return true;
}

static bool valid_userid(cstr<30> s);

static bool valid_userid(const std::string &s) {
  return valid_userid(string2cstr<30>(s));
}

static bool valid_userid(cstr<30> s) {
  if (not cstr_end(s))
    return false;
  for (int i = 0; s[i]; ++i)
    if (not validUserIDChar(s[i]))
      return false;
  return true;
}

static bool valid_password(auto s) {
  return valid_userid(s);
}

static bool valid_username(cstr<30> s);

static bool valid_username(const std::string &s) {
  return valid_username(string2cstr<30>(s));
}

static bool valid_username(cstr<30> s) {
  if (not cstr_end(s))
    return false;
  for (int i = 0; s[i]; ++i)
    if (not visibleChar(s[i]))
      return false;
  return true;
}

static bool valid_bookname(const cstr<60> &s);

static bool valid_bookname(const std::string &s) {
  return valid_bookname(string2cstr<60>(s));
}

static bool valid_bookname(const cstr<60> &s) {
  // if (not std::is_convertible_v<decltype(s), cstr<60>> or not cstr_end(s))
  //   return false;
  if (not cstr_end(s))
    return false;
  for (int i = 0; s[i]; ++i)
    if (not visibleChar(s[i]) or s[i] == '"') {
      return false;
    }
  return true;
}

static bool valid_ISBN(cstr<20> s);
static bool valid_ISBN(const std::string &s) {
  return valid_ISBN(string2cstr<20>(s));
}

static bool valid_ISBN(cstr<20> s) {
  if (not cstr_end(s))
    return false;
  for (int i = 0; s[i]; ++i)
    if (not visibleChar(s[i]))
      return false;
  return true;
}

static bool valid_author(auto s) {
  return valid_bookname(s);
}

static bool valid_keyword(auto s) {
  return valid_bookname(s);
}

static bool valid_count(std::string s) {
  try {
    int c = string2int(s);
    return 0 <= c and c <= 2147483647;
  } catch (...) {
    return false;
  }
}

static bookname_t string2bookname(std::string s) {
  if (!(valid_bookname(s))) throw Error("");;
  return string2cstr<bookname_t::N>(s);
}

static author_t string2author(std::string s) {
  if (!(valid_author(s))) throw Error("");;
  return string2cstr<author_t::N>(s);
}

static ISBN_t string2ISBN(std::string s) {
  if (!(valid_ISBN(s))) throw Error("");;
  return string2cstr<ISBN_t::N>(s);
}

static userid_t string2userid(std::string s) {
  if (!(valid_userid(s))) throw Error("");;
  return string2cstr<userid_t::N>(s);
}

static password_t string2password(std::string s) {
  if (!(valid_password(s))) throw Error("");;
  return string2cstr<password_t::N>(s);
}

static username_t string2username(std::string s) {
  if (!(valid_username(s))) throw Error("");;
  return string2cstr<username_t::N>(s);
}

static keyword_t string2keyword(std::string s) {
  if (!(valid_keyword(s))) throw Error("");;
  return string2cstr<keyword_t::N>(s);
}

static std::string unquote(std::string s) {
  if (!(s.size() >= 2 && s[0] == '"' && s[s.size() - 1] == '"')) throw Error("");;
  return s.substr(1, (int)s.size() - 2);
}

#endif //CSTR_H
