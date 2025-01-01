//
// Created by lenovo on 2025/1/1.
//

#ifndef USER_H
#define USER_H
#include "Memoryriver.h"
#include "cstr.h"
#include "c.h"
#include "database.h"
#include "book.h"

#include <string>
#include <cstring>
#include <stack>

struct user {
    int privilege;
    cstr<10> identity;
    cstr<30> userid, username, password;

    [[nodiscard]] bool validate_throw() const {
        if (!(privilege == 0 || privilege == 1 || privilege == 3 || privilege == 7)) throw Error("");;
        if (!(cstr_end(userid) && cstr_end(identity) && cstr_end(username) && cstr_end(password))) throw Error("");;
        if (!(identity == "admin" || identity == "clerk" || identity == "customer" || identity == "guest")) throw Error("");;
        if (!(valid_password(password))) throw Error("");;
        if (!(valid_username(username))) throw Error("");;
        if (!(valid_userid(userid))) throw Error("");;
        return true;
    }

    [[nodiscard]] bool validate() const {
        try {
            return validate_throw();
        } catch (...) {
            return false;
        }
    }
};

class userCenter {
private:
    Memoryriver bf;

public:
    static userCenter &getInstance() {
        static userCenter me;
        return me;
    }
    void login(userid_t userid, password_t password);
    void logout();
    void regis(userid_t userid, password_t password, username_t username);
    void useradd(userid_t userid, password_t password, privilege_t privilege, username_t username);
    void changePassword(userid_t userid, password_t cur_pass, password_t new_pass);
    void erase(userid_t userid);
    void select(ISBN_t ISBN);

    std::vector<user> login_stack;
    std::vector<bookid_t> select_stack;
    Database<cstr<30>, user, std::less<cstr<30>>{}, std::equal_to<cstr<30>>{}> db;

private:
    void sync(user &ac);
    userCenter();
};

static userCenter &acci = userCenter::getInstance();
// #define acci AccountCenter::getInstance()
#endif //USER_H
