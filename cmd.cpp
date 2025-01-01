#include "cmd.h"
#include "c.h"
#include "user.h"
#include "log.h"
#include "book.h"

#include <string>

using namespace ci;

#define _GOODTK(tk) ((tk).cmdB4param and not (tk).fail_param)
#define GOODTK Massert(_GOODTK(tk), "bad command")


Tokenized ci::tokenize(std::string s) {
  Tokenized res;
  res.raw = s;
  res.cmdB4param = true;

  int len = (int)s.size();
  bool cmd = true;
  for (int i = 0, i_; i < len; i = i_) {
    while (i < len && s[i] == ' ')
      ++i;
    bool inside_quote = false;
    bool backslash = false;
    if (s[i] != '-') {
      std::string cur;
      if (not cmd)
        throw Error("unexpected token");
      for (i_ = i; i_ < len; ++i_) {
        if (backslash) {
          cur += s[i_];
          backslash = false;
        } else if (s[i_] == '"') {
          if (inside_quote) {
            inside_quote = false;
          } else {
            inside_quote = true;
          }
        } else if (s[i_] == ' ') {
          if (inside_quote)
            cur += s[i_];
          else
            break;
        } else
          cur += s[i_];
      }
      if (backslash)
        throw Error("tokenize: backslash expected one charactor, read eol");
      if (cur != "")
        res.command.push_back(cur);
    } else {
      std::string key, val;
      cmd = false;
      for (i_ = i + 1; i_ < len; ++i_) {
        if (backslash) {
          key += s[i_];
          backslash = false;
        } else if (s[i_] == '"') {
          if (inside_quote) {
            inside_quote = false;
          } else {
            inside_quote = true;
          }
        } else if (s[i_] == ' ') {
          if (inside_quote)
            key += s[i_];
          else {
            ++i_;
            break;
          }
        } else if (s[i_] == '=') {
          if (not inside_quote) {
            ++i_;
            break;
          }
        } else
          key += s[i_];
      }
      if (backslash)
        throw Error("tokenize: backslash expected one charactor, read eol");
      for (; i_ < len; ++i_) {
        if (backslash) {
          val += s[i_];
          backslash = false;
        } else if (s[i_] == '"') {
          if (inside_quote) {
            inside_quote = false;
          } else {
            inside_quote = true;
          }
        } else if (s[i_] == ' ') {
          if (inside_quote)
            val += s[i_];
          else
            break;
        } else if (s[i_] == '=') {
          if (not inside_quote)
            break;
        } else
          val += s[i_];
      }
      if (backslash)
        throw Error("tokenize: backslash expected one charactor, read eol");
      if (res.param.count(key))
        throw Error("tokenize: duplicated parameter");
      res.param[key] = val;
    }
  }
  return res;
}



void Ci::process_one() {
  std::string s;
  std::getline(is, s);
  // Massert(not is.eof(), "input end");
  if (is.eof())
    exit(0);
  Tokenized tk = tokenize(s);
  if (tk.command.empty())
    return;
  if (tk.command[0] == "exit" or tk.command[0] == "quit") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    if (!(tk.param.empty() && tk.command.size() == 1)) throw Error("");;
    exit(0);
  } else if (tk.command.at(0) == "su") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    if (!(tk.param.empty())) throw Error("");;
    if (!(tk.command.size() >= 2 && tk.command.size() <= 3)) throw Error("");;
    userCenter::getInstance().login(string2userid(tk.command.at(1)), string2password(tk.command.size() >= 3 ? tk.command.at(2) : ""));
  } else if (tk.command.at(0) == "register") {
    if (!(tk.splited.size() == 4)) throw Error("");;
    userCenter::getInstance().regis(string2userid(tk.command.at(1)), string2password(tk.command.at(2)), string2username(tk.splited.at(3)));
  } else if (tk.command.at(0) == "passwd") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    if (!(tk.param.empty())) throw Error("");;
    if (!(tk.command.size() == 3 || tk.command.size() == 4)) throw Error("");;
    if (!(acci.login_stack.size() > 1)) throw Error("");;
    if (tk.command.size() == 3)
      userCenter::getInstance().changePassword(string2userid(tk.command.at(1)), string2password(""), string2password(tk.command.at(2)));
    else
      userCenter::getInstance().changePassword(string2userid(tk.command.at(1)), string2password(tk.command.at(2)), string2password(tk.command.at(3)));
  } else if (tk.command.at(0) == "logout") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    if (!(tk.param.empty() && tk.command.size() == 1)) throw Error("");;
    userCenter::getInstance().logout();
  } else if (tk.command.at(0) == "useradd") {
    if (!(tk.splited.size() == 5)) throw Error("");;
    if (!(tk.command.at(3).size() == 1 && '0' <= tk.command.at(3)[0] && tk.command.at(3)[0] <= '9')) throw Error("");;
    userCenter::getInstance().useradd(string2userid(tk.command.at(1)), string2password(tk.command.at(2)), (privilege_t)(tk.command.at(3)[0] - '0'), string2username(tk.splited.at(4)));
  } else if (tk.command.at(0) == "delete") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    if (!(tk.param.empty())) throw Error("");;
    if (!(tk.command.size() == 2)) throw Error("");;
    if (!(! acci.login_stack.empty() && acci.login_stack.back().privilege >= 7)) throw Error("");;
   userCenter::getInstance().erase(string2userid(tk.command.at(1)));
  } else if (tk.command.at(0) == "select") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    if (!(tk.splited.size() == 2)) throw Error("");;
    userCenter::getInstance().select(string2ISBN(tk.splited.at(1)));
  } else if (tk.command.at(0) == "modify") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    static const std::vector<std::string> modify_allow_fields = { "ISBN", "name", "author", "keyword", "price" };
    static const std::vector<std::string> modify_unquote_fields = { "name", "author", "keyword" };
    if (!(param_inside(tk, modify_allow_fields))) throw Error("");;
    if (!(tk.command.size() == 1)) throw Error("");;
    if (!(userCenter::getInstance().login_stack.back().privilege >= 3)) throw Error("");;
    if (!(userCenter::getInstance().select_stack.size() > 1 && userCenter::getInstance().select_stack.back() != nullid)) throw Error("");;
    for (auto un : modify_unquote_fields) {
      if (tk.param.count(un)) {
        tk.param[un] = unquote(tk.param[un]);
      }
    }
    Bookstore::getInstance().modify(userCenter::getInstance().select_stack.back(), tk.param);
  } else if (tk.command.at(0) == "show" and tk.command.size() >= 2 and tk.command.at(1) == "finance") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    if (!(tk.command.size() <= 3)) throw Error("");;
    if (!(acci.login_stack.size() > 1 && acci.login_stack.back().privilege >= 7)) throw Error("");;
    if (!(tk.param.empty())) throw Error("");;
    if (tk.splited.size() == 2)
      Finance::getInstance().showAll();
    else
      Finance::getInstance().show(string2int(tk.splited.at(2)));
  } else if (tk.command.at(0) == "show") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    static const std::vector<std::string> show_allow_fields = { "ISBN", "name", "author", "keyword" };
    static const std::vector<std::string> show_unquote_fields = { "name", "author", "keyword" };
    if (!(tk.command.size() == 1)) throw Error("");;
    if (!(tk.param.empty() || (tk.param.size() == 1 && param_inside(tk, show_allow_fields)))) throw Error("");;
    if (!(acci.login_stack.size() > 1 && acci.login_stack.back().privilege >= 1)) throw Error("");;
    for (auto un : show_unquote_fields) {
      if (tk.param.count(un))
        tk.param[un] = unquote(tk.param[un]);
    }
    if (tk.param.count("ISBN")) {
      if (!(tk.param.at("ISBN") != "")) throw Error("");;
      Bookstore::getInstance().showByISBN(string2ISBN(tk.param["ISBN"]));
    } else if (tk.param.count("name")) {
      if (!(tk.param.at("name") != "")) throw Error("");;
      Bookstore::getInstance().showByName(string2bookname(tk.param["name"]));
    } else if (tk.param.count("author")) {
      if (!(tk.param.at("author") != "")) throw Error("");;
      Bookstore::getInstance().showByAuthor(string2author(tk.param["author"]));
    } else if (tk.param.count("keyword")) {
      if (!(tk.param.at("keyword") != "")) throw Error("");;
      auto &s = tk.param["keyword"];
      for (auto c : s) {
        if (!(c != '|')) throw Error("");;
      }
      Bookstore::getInstance().showByKeyword(string2keyword(tk.param["keyword"]));
    } else {
      assert(tk.param.empty());
      Bookstore::getInstance().showAll();
    }
  } else if (tk.command.at(0) == "import") {
    if (!(((tk).cmdB4param && ! (tk).fail_param))) throw Error("");;
    if (!(tk.splited.size() == 3)) throw Error("");;
    if (!(tk.param.empty())) throw Error("");;
    if (!(userCenter::getInstance().login_stack.back().privilege >= 3)) throw Error("");;
    if (!(userCenter::getInstance().select_stack.size() > 1 && userCenter::getInstance().select_stack.back() != nullid)) throw Error("");;
    if (!(valid_price(tk.splited.at(2)))) throw Error("");;
    Bookstore::getInstance().import_book(userCenter::getInstance().select_stack.back(), string2int(tk.splited.at(1)), string2double(tk.splited.at(2)));
  } else if (tk.command.at(0) == "buy") {
    if (!(acci.login_stack.size() > 1)) throw Error("");;
    if (!(tk.splited.size() == 3)) throw Error("");;
    bkst.buy(string2ISBN(tk.splited.at(1)), string2int(tk.splited.at(2)));
  } else if (tk.command.at(0) == ".print") {
    if (!(userCenter::getInstance().select_stack.size() > 1 && userCenter::getInstance().select_stack.back() != nullid)) throw Error("");;
    Book b = Bookstore::getInstance().askByBookid(userCenter::getInstance().select_stack.back());
    b.print();
  } else
    throw Error("unrecognized command");
}
