#pragma once

#include <iostream>
#include <vector>
#include <limits>

struct Slice {
    int t;
    struct Point2 {
        double x, y;
    };
    std::vector<Slice::Point2> points;
    void sort_by_direction();
};

