#include "LayerObj3D.hpp"
#include <iostream>

int main(int argc, char** argv) {
    double length_par_time = 1;
    int interpolate_size = 0;
    if (argc >= 2) {
        length_par_time = std::stof(argv[1]);
    }
    if (argc >= 3) {
        interpolate_size = std::stoi(argv[2]);
    }
    LayerObj3D obj(length_par_time, interpolate_size);
    std::cin >> obj;
    std::cout << obj;
}


