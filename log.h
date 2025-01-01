//
// Created by lenovo on 2025/1/1.
//

#ifndef LOG_H
#define LOG_H
#include "Memoryriver.h"
#include "database.h"

// TODO: optimize using DBMore::vector

class Finance {
    Memoryriver bf;
    Database<int, std::pair<bool, double>> db;
    int timestamp;
    Finance();

public:
    static Finance &getInstance() {
        static Finance r;
        return r;
    }

    void income(double c);
    void outcome(double c);
    void show(int c);
    void showAll();
};
#endif //LOG_H
