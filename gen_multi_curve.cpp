#include "TraceCurveObj3D.hpp"
#include "MultiObj3D.hpp"

#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    double total_length = 110;
    if (argc >= 2) {
        total_length = std::stof(argv[1]);
    }

    int n, m;
    std::cin >> n >> m;
    std::vector<std::vector<double>> xs_list(n, std::vector<double>(m));
    std::vector<std::vector<double>> ys_list(n, std::vector<double>(m));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            std::cin >> xs_list[i][j] >> ys_list[i][j];
        }
    }

    std::vector<TraceCurveObj3D> obj_list;
    obj_list.reserve(n);
    std::vector<TraceObj3D*> obj_ptr_list(n);

    for (int i = 0; i < n; i++) {
        obj_list.emplace_back(TraceCurveObj3D{
            xs_list[i], ys_list[i], total_length
        });
        obj_ptr_list[i] = &obj_list[i];
        
    }

    MultiObj3D multi_obj{obj_ptr_list};

    std::cout << multi_obj;
}
