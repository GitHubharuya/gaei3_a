#pragma once

#include <cmath>

namespace Geom {

struct Point3 {
    double x, y, z;
    double sum_sq() const {
        return x * x + y * y + z * z;
    }
};

double dot_prod(const Point3 a, const Point3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Point3 cross_prod(const Point3 a, const Point3 b) {
    double cx = a.y * b.z - a.z * b.y;
    double cy = -a.x * b.z + a.z * b.y;
    double cz = a.x * b.y - a.y * b.x;
    return { cx, cy, cz };
}

} // END namespace Geom
