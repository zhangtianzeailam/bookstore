//
// Created by lenovo on 2025/1/1.
//

#ifndef C_H
#define C_H
#include <exception>
#include <string>
class Error : std::exception {
public:
    explicit Error(const std::string &msg_) : msg(msg_) {}
    std::string msg;
};

inline void Eassert(bool condition, std::string msg = "") {
    if (not condition)
        throw Error(msg);
}
#define Massert(condition, msg) \
if (!(condition)) throw Error("");
#endif //C_H
