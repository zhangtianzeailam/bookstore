#include <bits/stdc++.h>
#include "Memoryriver.h"
#include "database.h"
#include "databaseplus.h"
#include "cmd.h"
int main() {
    Memoryriver m("test.db");
    DBMore<int, int, int> dbm{m};
    while (true) {
        Ci &ci = Ci::getInstance();
#if 0
        ci.process_one();
#else
        try {
            ci.process_one();
        } catch (const Error &e) {
            std::cout << "Invalid" << std::endl;
            // std::cerr << e.msg << std::endl;
        }
#endif
    }
    return 0;
}
