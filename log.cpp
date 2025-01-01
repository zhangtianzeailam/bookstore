#include "log.h"
#include <iomanip>




Finance::Finance() : bf("finance.db"), db(bf) {
    bf.getHeaderT(1, timestamp);
}

void Finance::income(double c) {
    db.insert(timestamp++, {false, c});
    bf.putT(bf.getHeaderPos(1), timestamp);
}

void Finance::outcome(double c) {
    db.insert(timestamp++, {true, c});
    bf.putT(bf.getHeaderPos(1), timestamp);
}

void Finance::show(int c) {
    if (!(0 <= c && c <= timestamp)) throw Error("");;
    if (c == 0)
        return puts(""), void();
    double in = 0, out = 0;
    for (int i = 1; i <= c; ++i) {
        auto t = db.get(timestamp - i);
        if (t.first)
            out += t.second;
        else
            in += t.second;
    }
    std::cout << "+ " << std::fixed << std::setprecision(2) << in << " - " << out << std::endl;
}

void Finance::showAll() {
    double in = 0, out = 0;
    for (int i = 0; i < timestamp; ++i) {
        auto t = db.get(i);
        if (t.first)
            out += t.second;
        else
            in += t.second;
    }
    std::cout << "+ " << std::fixed << std::setprecision(2) << in << " - " << out << std::endl;
}