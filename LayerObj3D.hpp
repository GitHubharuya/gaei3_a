#pragma once

#include <iostream>
#include <vector>
#include "TraceObj3D.hpp"

struct LayerObj3D : public TraceObj3D {
    double LENGTH_PER_TIME = 1;
    LayerObj3D(double lpt) : LENGTH_PER_TIME(lpt) {};
    LayerObj3D() {};
    std::vector<Slice> slices;
    bool make_points() override;
    bool check_slice_point_size() const;
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

bool LayerObj3D::check_slice_point_size() const {
    for (const auto& slice : slices) {
        if (slice.points.size() != point_size_per_step) return false;
    }
    return true;
}

std::istream& operator>>(std::istream& ist, LayerObj3D& obj) {
    Slice slice;
    while (ist >> slice) {
        obj.slices.push_back(slice);
    }

    if (ist.fail() && !ist.eof()) {
        return ist;
    }

    bool is_ok = true;
    obj.step_size = obj.slices.size();
    if (obj.step_size == 0) {
        is_ok = false;
        std::cerr << "slice size is 0\n";
    } else {
        obj.point_size_per_step = obj.slices[0].points.size();
        obj.first_slice = obj.slices[0];
        obj.last_slice = obj.slices.back();
    }

    is_ok &= obj.check_slice_point_size();
    is_ok &= obj.from_slices();
    
    if (!is_ok) {
        // スライスからの生成に失敗
        ist.setstate(std::ios_base::failbit);
    };
    return ist;
}
