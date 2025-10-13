#pragma once

#include <array>
#include <iostream>
#include <vector>
#include <cmath>

#include "TraceObj3D.hpp"

struct LayerObj3D : public TraceObj3D {
    double LENGTH_PER_TIME = 1;
    LayerObj3D(double lpt) : LENGTH_PER_TIME(lpt) {};
    LayerObj3D() {};
    bool make_points() override;
};

bool LayerObj3D::make_points() {
    points.clear();
    points.reserve(slices.size());
    double z = 0;
    for (PointSize i = 0; i < slices.size(); i++) {
        for (const auto& p : slices[i].points) {
            points.emplace_back(Geom::Point3{
                p.x, p.y, z
            });
        }
        z += LENGTH_PER_TIME;
    }
    return true;
}

std::istream& operator>>(std::istream& ist, LayerObj3D& obj) {
    Slice slice;
    while (ist >> slice) {
        // slice.sort_by_direction();
        obj.slices.push_back(slice);
    }

    if (ist.fail() && !ist.eof()) {
        return ist;
    }
    if (!obj.check_slice_point_size() || !obj.from_slices()) {
        // スライスからの生成に失敗
        ist.setstate(std::ios_base::failbit);
    };
    return ist;
}
