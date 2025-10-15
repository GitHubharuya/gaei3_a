#pragma once

#include "TraceCenterObj3D.hpp"

Slice get_circle(double r) {
    Slice circle;
    int pn = 50;
    double pi = std::acos(-1);
    circle.points.reserve(pn);
    for (int i = 0; i < pn; i++) {
        double th = (double)i / pn * 2 * pi;
        circle.points.emplace_back(Geom::Point2{
            r * std::cos(th), r * std::sin(th)
        });
    }
    return circle;
}

struct TraceCurveObj3D : public TraceCenterObj3D {
    TraceCurveObj3D(
        const std::vector<double>& xs,
        const std::vector<double>& ys,
        double _total_length = 100,
        double radius = 1
    ) : TraceCenterObj3D(xs, ys, {get_circle(radius)}, _total_length, true) {
    }
};

