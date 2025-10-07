#include "../Object.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <cmath>

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
    Object obj;
    std::istringstream iss;
    iss.str("0, 0 0 1 0 1 1 0 1\n"
            "1, 0 0 1 0 1 1 0 1\n");

    // 中点追加
    std::set<std::string> expected = {
        "v 0 0 0",
        "v 1 0 0",
        "v 1 1 0",
        "v 0 1 0",
        "v 0 0 1",
        "v 1 0 1",
        "v 1 1 1",
        "v 0 1 1",
        "v 0.5 0 0.5",
        "v 1 0.5 0.5",
        "v 0.5 1 0.5",
        "v 0 0.5 0.5",
        "f 6 5 9",
        "f 5 1 9",
        "f 1 2 9",
        "f 2 6 9",
        "f 7 6 10",
        "f 6 2 10",
        "f 2 3 10",
        "f 3 7 10",
        "f 8 7 11",
        "f 7 3 11",
        "f 3 4 11",
        "f 4 8 11",
        "f 5 8 12",
        "f 8 4 12",
        "f 4 1 12",
        "f 1 5 12",
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
    Object obj;
    std::istringstream iss;
    iss.str("0, 0 0 1 0 1 1 0 1\n"
            "1, 0 0 1 0 1 1 0 1\n"
            "2, 0 0 1 0 1 1 0 1\n");

    // 中点追加
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
        "v 0.5 0 0.5",
        "v 1 0.5 0.5",
        "v 0.5 1 0.5",
        "v 0 0.5 0.5",
        "v 0.5 0 1.5",
        "v 1 0.5 1.5",
        "v 0.5 1 1.5",
        "v 0 0.5 1.5",
        "f 6 5 13",
        "f 5 1 13",
        "f 1 2 13",
        "f 2 6 13",
        "f 7 6 14",
        "f 6 2 14",
        "f 2 3 14",
        "f 3 7 14",
        "f 8 7 15",
        "f 7 3 15",
        "f 3 4 15",
        "f 4 8 15",
        "f 5 8 16",
        "f 8 4 16",
        "f 4 1 16",
        "f 1 5 16",
        "f 10 9 17",
        "f 9 5 17",
        "f 5 6 17",
        "f 6 10 17",
        "f 11 10 18",
        "f 10 6 18",
        "f 6 7 18",
        "f 7 11 18",
        "f 12 11 19",
        "f 11 7 19",
        "f 7 8 19",
        "f 8 12 19",
        "f 9 12 20",
        "f 12 8 20",
        "f 8 5 20",
        "f 5 9 20",
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
