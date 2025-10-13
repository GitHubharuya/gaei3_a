#include "../LayerObj3D.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <set>

bool check_including_line(const std::set<std::string>& se, std::stringstream& ss) {
    std::string line;
    int linenum = 0;
    bool ok = true;
    while (getline(ss, line, '\n')) {
        if (!se.contains(line)) {
            ok = false;
        }
        linenum++;
    }
    return ok && linenum == se.size();
}

void test_obj_cube() {
    LayerObj3D obj;
    std::istringstream iss;
    iss.str("0, 0 0 1 0 1 1 0 1\n"
            "1, 0 0 1 0 1 1 0 1\n");

    std::set<std::string> expected = {
        "v 0 0 0",
        "v 1 0 0",
        "v 1 1 0",
        "v 0 1 0",
        "v 0 0 1",
        "v 1 0 1",
        "v 1 1 1",
        "v 0 1 1",
        "f 6 5 1",
        "f 1 2 6",
        "f 7 6 2",
        "f 2 3 7",
        "f 8 7 3",
        "f 3 4 8",
        "f 5 8 4",
        "f 4 1 5",
        "f 1 3 2",
        "f 1 4 3",
        "f 5 6 7",
        "f 5 7 8",
    };

    iss >> obj;
    std::stringstream ss;

    ss << obj;
    if (check_including_line(expected, ss)) {
        std::cout << "Success: cube obj" << std::endl;
    } else {
        std::cerr << "Failed: cube obj" << std::endl;
    }
}

// 立方体が二つつながった図形
void test_obj_double_cube() {
    LayerObj3D obj;
    std::istringstream iss;
    iss.str("0, 0 0 1 0 1 1 0 1\n"
            "1, 0 0 1 0 1 1 0 1\n"
            "2, 0 0 1 0 1 1 0 1\n");

    std::set<std::string> expected = {
        "v 0 0 0",
        "v 1 0 0",
        "v 1 1 0",
        "v 0 1 0",
        "v 0 0 1",
        "v 1 0 1",
        "v 1 1 1",
        "v 0 1 1",
        "v 0 0 2",
        "v 1 0 2",
        "v 1 1 2",
        "v 0 1 2",
        "f 6 5 1",
        "f 1 2 6",
        "f 7 6 2",
        "f 2 3 7",
        "f 8 7 3",
        "f 3 4 8",
        "f 5 8 4",
        "f 4 1 5",
        "f 10 9 5",
        "f 5 6 10",
        "f 11 10 6",
        "f 6 7 11",
        "f 12 11 7",
        "f 7 8 12",
        "f 9 12 8",
        "f 8 5 9",
        "f 1 3 2",
        "f 1 4 3",
        "f 9 10 11",
        "f 9 11 12",
    };

    iss >> obj;
    std::stringstream ss;
    ss << obj;
    if (check_including_line(expected, ss)) {
        std::cout << "Success: double cube obj" << std::endl;
    } else {
        std::cerr << "Failed: double cube obj" << std::endl;
    }
}

int main() {
    test_obj_cube();
    test_obj_double_cube();
}
