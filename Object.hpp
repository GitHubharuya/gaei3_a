#pragma once

#include "Slice.hpp"

using PointIdx = unsigned long;
using PointSize = unsigned long;

struct Object {
    struct Point3 {
        double x, y, z;
    };
    std::vector<Object::Point3> points;
    std::vector<std::array<PointIdx, 3>> faces;
    std::vector<Slice> slices;
};

