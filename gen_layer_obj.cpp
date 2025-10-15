#include "LayerObj3D.hpp"
#include <iostream>

int main(int argc, char** argv) {
    double length_par_time = 1;
    if (argc >= 2) {
        length_par_time = std::stof(argv[1]);
    }
    LayerObj3D obj(length_par_time);
    std::cin >> obj;
    std::cout << obj;
}


