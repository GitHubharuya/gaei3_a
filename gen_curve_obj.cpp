#include "TraceCurveObj3D.hpp"

#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    std::vector<double>xs, ys;
    double x, y;
    while (std::cin >> x >> y) {
        xs.push_back(x);
        ys.push_back(y);
    }

    double total_length = 110;
    if (argc >= 2) {
        total_length = std::stof(argv[1]);
    }

    TraceCurveObj3D obj{xs, ys, total_length};
    std::cout << obj;
}
