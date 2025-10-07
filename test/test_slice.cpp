#include "../Slice.hpp"

#include <string>
#include <sstream>

void slice_test() {
    std::string str = "0, 0 0 0 1\n1, 2 2 0 2\n";
    std::istringstream iss{str};
    std::vector<Slice> sls;

    Slice sl;
    while (iss >> sl) {
        sls.push_back(sl);
    }

    if (iss.fail()) {
        if (iss.eof()) {
            std::cout << "Success: input from stream" << std::endl;
        } else {
            std::cout << "failed" << std::endl;
        }
    } else {
        std::cout << "Success: input from stream" << std::endl;
    }

    std::vector<std::vector<Slice::Point2>> expected = {
        {{0, 0}, {0, 1}},
        {{2, 2}, {0, 2}}, 
    };
    bool ok = true;
    if (sls.size() != expected.size()) { ok = false; }
    for (int i = 0; i < sls.size(); i++) {
        if (sls[i].t != i) { ok = false; }
        if (sls[i].points.size() != expected[i].size()) { ok = false; }
        for (int j = 0; j < sls[i].points.size(); j++) {
            if (sls[i].points[j].x != expected[i][j].x
                || sls[i].points[j].y != expected[i][j].y) {
                ok = false;
            }
        }
    }

    if (ok) {
        std::cout << "Success: simple slice" << std::endl;
    } else {
        std::cout << "Failed: simple slice" << std::endl;
    }
}

int main() {
    slice_test();
}

