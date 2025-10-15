#pragma once

#include <cmath>

namespace Geom {

struct Point2 {
    double x, y;
};

// 三角形 p0 p1 p2 の2倍の符号付き面積
double area2(const Point2& p0, const Point2& p1, const Point2& p2) {
    return (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
};

// p0 p1 p2 が反時計回り
bool is_ccw(const Point2& p0, const Point2& p1, const Point2& p2) {
    return area2(p0, p1, p2) > 0;
};

struct Point3 {
    double x, y, z;
    double sum_sq() const {
        return x * x + y * y + z * z;
    }
    void normalize() {
        double length = std::sqrt(sum_sq());
        if (length == 0) {
            return;
        }
        x /= length; y /= length; z /= length;
    }
};

double dot_prod(const Point3 a, const Point3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Point3 cross_prod(const Point3 a, const Point3 b) {
    double cx = a.y * b.z - a.z * b.y;
    double cy = -a.x * b.z + a.z * b.x;
    double cz = a.x * b.y - a.y * b.x;
    return { cx, cy, cz };
}

} // END namespace Geom
