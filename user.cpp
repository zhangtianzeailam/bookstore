#include "user.h"
#include "cstr.h"
#include <cmath>

#include "c.h"

void userCenter::login(userid_t userid, password_t password) {
  if (not db.exist(userid))
    throw Error("login: account does not exist");
  auto ac = db.get(userid);
  if (password != "" and ac.password != password)
    throw Error("login: wrong password");
  if (password == "") {
    if (login_stack.size() == 1)
      throw Error("");
    auto &cur = login_stack.back();
    sync(cur);
    if (ac.privilege >= cur.privilege)
      throw Error("login: not enough privilege");
  }
  login_stack.push_back(ac);
  select_stack.push_back(nullid);
}

void userCenter::logout() {
  assert(not login_stack.empty());
  if (login_stack.size() == 1)
    throw Error("logout: no account logged in");
  login_stack.pop_back();
  select_stack.pop_back();
}

void userCenter::regis(userid_t userid, password_t password, username_t username) {
  if (db.exist(userid))
    throw Error("register: user exists");
  user ac{1, "customer", userid, username, password};
  db.insert(userid, ac);
}

void userCenter::useradd(userid_t userid, password_t password, privilege_t privilege, username_t username) {
  if (db.exist(userid))
    throw Error("useradd: user exists");
  if (!(login_stack.back().privilege >= 3)) throw Error("");;
  if (!(login_stack.back().privilege > privilege)) throw Error("");;
  if (!(valid_privilege(privilege))) throw Error("");;
  if (!(valid_userid(userid))) throw Error("");;
  if (!(valid_password(password))) throw Error("");;
  if (!(valid_username(username))) throw Error("");;
  user ac{
    privilege, (identity_t) string2cstr < 10 > (privilege == 1 ? "customer" : (privilege == 3 ? "clerk" : "admin")),
    userid, username, password
  };
  if (!(ac.validate() || privilege == 0)) throw Error("");;
  db.insert(userid, ac);
}

void userCenter::changePassword(userid_t userid, password_t cur_pass, password_t new_pass) {
  if (!(db.exist(userid))) throw Error("");;
  if (!(login_stack.back().privilege >= 1)) throw Error("");;
  auto ac = db.get(userid);
  if (cur_pass != "" and cur_pass != ac.password)
    throw Error("");
  if (cur_pass == "" and login_stack.back().privilege != 7)
    throw Error("");
  ac.password = new_pass;
  db.modify(userid, ac);
}

void userCenter::erase(userid_t userid) {
  if (!(db.exist(userid))) throw Error("");;
  if (!(login_stack.back().privilege >= 7)) throw Error("");;
  for (int i = 1; i < (int) login_stack.size(); ++i)
    if (!(login_stack[i].userid != userid)) throw Error("");;
  db.erase(userid);
}

void userCenter::select(ISBN_t ISBN) {
  if (!(login_stack.size() > 1)) throw Error("");;
  if (!(login_stack.back().privilege >= 3)) throw Error("");;
  select_stack.back() = Bookstore::getInstance().select(ISBN);
}

void userCenter::sync(user &ac) {
  try {
    ac = db.get(ac.userid);
  } catch (const Error &e) {
    if (e.msg != "get: not found")
      throw;
  }
}

userCenter::userCenter(): bf("account.db"), db(bf) {
  user guest{0, "guest", "guest#unspecified", "guest#unspecified", "guest#unspecified"};
  login_stack.push_back(guest);
  select_stack.push_back(nullid);

  bool init = false;
  bf.getHeaderT(1, init);
  if (not init) {
    user root{7, "admin", "root", "root#unspecified", "sjtu"};
    db.insert(root.userid, root);

    init = true;
    bf.putT(bf.getHeaderPos(1), init);
  }
}


//
